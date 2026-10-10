# cstd

A C standard library written in sysl: the module `sh.sysl.cstd`, with C headers in `include/`.

It is not a general-purpose libc competing with musl. It exists for two consumers:

1. **keel**, the research kernel in sysl (`github.com/sysl-lang/keel`). C programs are ported to
   keel through it.
2. **Bare-metal boards.** sysl's freestanding targets have no libc, so a vendored C package that
   includes `<string.h>` or `<assert.h>` fails there with `'string.h' file not found`. cstd supplies
   the headers and the symbols behind them.

Every function is an ordinary sysl function over raw pointers, which is what the C signatures
require, exported under its C name with `@export`.

## What is in it

| header | functions |
|---|---|
| `string.h` | `strlen` `strnlen` `strcmp` `strncmp` `strcpy` `strncpy` `strcat` `strncat` `strchr` `strrchr` `strstr` `strspn` `strcspn` `strpbrk` `strtok` `strtok_r` `strerror` `memcpy` `memmove` `memset` `memcmp` `memchr` |
| `ctype.h` | `isalnum` `isalpha` `isblank` `iscntrl` `isdigit` `isgraph` `islower` `isprint` `ispunct` `isspace` `isupper` `isxdigit` `tolower` `toupper`, for the C locale |
| `stdlib.h` | `atoi` `atol` `atof` `strtol` `strtoul` `strtoll` `strtoull` `strtod` `strtof` `abs` `labs` `llabs` `div` `ldiv` `lldiv` `qsort` `bsearch` |
| `errno.h` | `errno` (one global, defined by the library), `EINVAL`, `ERANGE` |
| `stdio.h` | `printf` `dprintf` `sprintf` `snprintf` `vprintf` `vdprintf` `vsprintf` `vsnprintf` |

- The printf family takes C99's `%d %i %u %o %x %X %c %s %p %n %%` and `%f %F %e %E %g %G %a %A`,
  the flags `- + space # 0`, a width and a precision as a number or `*`, and the lengths
  `hh h l ll j z t`. There is no `long double`, so `L` is not taken and `%Lf` is copied out as
  written. There is no `FILE`:
  `printf` writes to descriptor 1 and `dprintf` to the one it is given, both through `write`, which
  the platform supplies. Output gathers in a 256-byte buffer on the stack and nothing allocates.
- Where C leaves a choice, musl's answer is taken, since keel targets musl compatibility: `%p` is
  `%#zx`, so a null pointer prints `0` (glibc prints `(nil)`); `%s` of a null pointer prints
  `(null)`; a conversion C does not define is copied out as written.

- **Floats print their exact value, correctly rounded.** A double is `m × 2^e`, so its decimal
  expansion is finite; it is built exactly in base-10^9 limbs (as musl's `fmt_fp` does) and rounded
  half to even where the precision says, so `%.0f` of 0.5 is `0`, `%.20f` of 0.1 is
  `0.10000000000000000555`, and `%f` of 1e300 prints all of its 301 digits. No float arithmetic is
  done, so a soft-float board prints exactly what a hosted machine does. `%a` is exact hex, a
  subnormal normalised to `0x1p-1074` as musl prints it. Infinity and NaN are `inf`/`nan` (upper
  case under `F E G A`), signed by the sign bit as musl does.
- **`strtod` and `strtof` are correctly rounded for any length of input**, through the standard
  library's `sysl.text.real_prefix`/`f32_prefix`: Eisel-Lemire on the first nineteen digits, and an
  exact big-integer comparison against the halfway point where more digits could decide it.
  Decimal and hex forms, `inf`, `infinity`, `nan` and `nan(...)` are read. Overflow answers
  ±infinity and an underflow a zero or subnormal, each with `ERANGE` (every subnormal result sets
  it; C leaves underflow's `errno` to the implementation). No number at all leaves `endptr` at the
  text and sets `EINVAL`, as the integer `strto*` do here.
- `long` is measured with a `c type`, so `strtol` answers 32 bits on a Cortex-M and 64 on an LP64
  machine.
- The `strto*` functions take base 0 or 2–36, skip leading white space, read one sign and an
  optional `0x`, set `endptr`, and saturate with `ERANGE`.
- `qsort` is a heapsort: O(n log n) on every input, constant stack, any element size, not stable.
- `strerror` knows `0`, `EINVAL` and `ERANGE`; any other code answers the fixed text
  `Unknown error`, because formatting the number in would need a formatter.
- `strtok` keeps one save pointer for the whole program, as C does; `strtok_r` keeps it in the caller's hands.
- Text with no number in it, or a base C does not define, sets `EINVAL`, as the BSD libcs do. C
  leaves that case unspecified.

## Using it from C

```
sysl build-c . --target aarch64-freestanding -o libcstd.a
clang --target=aarch64-none-elf -ffreestanding -nostdinc \
      -isystem "$(clang -print-resource-dir)/include" -I <cstd>/include -c prog.c
```

`-nostdinc` drops every system header. `-isystem` puts back clang's own freestanding ones
(`stddef.h`, `stdint.h`, `limits.h`, `stdbool.h`), and `include/` supplies the rest.

## Testing

```
sysl test .
```

The tests are C programs. Each case is a C snippet passed to `c_case(snippet, expected_stdout)`
(`sh/sysl/cstd/c_case_tests.sysl`). The snippet becomes the body of `main`, compiled
`-nostdinc -fno-builtin -O0` against `include/` and linked with the archive `sysl build-c` writes
(built once per run, in `.sysl/c-tests/`). The program is run and its standard output compared
exactly. A case prints through the `say_*` helpers in `tests/say.h`, which use nothing from the
host's libc except `write`. Expected values are written in the test, never taken from the host's
libc.

A function that writes memory is checked on the bytes it wrote. The whole buffer is printed, with a
guard byte on each side, NULs included. The cases cover:

- the empty string, and bytes ≥ 0x80 (compared as `unsigned char`)
- not-found answers
- overlapping `memmove` in both directions
- `strncpy`'s padding and its missing terminator
- every `<ctype.h>` function over EOF and 0–255
- every base from 2 to 36
- `EINVAL` for a bad base or no digits
- overflow in each `strto*` with `ERANGE`
- `endptr` positions

Under AddressSanitizer:

```
SYSL_EXTRA_CFLAGS="-fsanitize=address -g" sysl test .
```

The variable reaches the `sysl build-c` the suite runs, and `c_case` hands the same flags to each
case's clang, so the archive and every program are instrumented together. Two tests run only in
this mode:
- `nm -u` of the archive must show `__asan_report_load1` and `__asan_report_store1`.
- `strlen` over a buffer with no NUL must be reported as a `stack-buffer-overflow` in `strlen`.

The freestanding builds are checked with:

```
sysl build-lib . --target thumb-freestanding-softfp
sysl build-lib . --target aarch64-freestanding
```

## What is deliberately absent, and why

- **Anything that allocates** (`malloc` and friends, `strdup`). Where the memory comes from is the
  platform's decision: keel's heap, or a board's static arena. That half arrives with a design for it.
- **`FILE` and the rest of stdio, `exit`, `setjmp`.** These need a platform underneath (a process, a
  crt0, buffered streams), so they belong with keel's port layer rather than this portable core.
- **`long double`, `strtold` and `%Lf`.** cstd's floats are `double` and `float` only.
- **Locales.** Only the C locale exists: every byte from 0x80 up is in no class.
- **A per-thread `errno`.** Nothing this library serves has threads yet.

## License

ISC. See `LICENSE`.
