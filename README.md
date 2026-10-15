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
| `string.h` | `strlen` `strnlen` `strcmp` `strncmp` `strcpy` `strncpy` `strcat` `strncat` `strchr` `strrchr` `strstr` `strspn` `strcspn` `strpbrk` `strtok` `strtok_r` `strerror` `strerror_r` `strerror_l` `strsignal` `stpcpy` `stpncpy` `strlcpy` `strlcat` `strdup` `strndup` `strcoll` `strcoll_l` `strxfrm` `strxfrm_l` `memcpy` `memmove` `memset` `memcmp` `memchr` `memmem` — all of POSIX.1-2024's base `<string.h>` |
| `ctype.h` | `isalnum` `isalpha` `isblank` `iscntrl` `isdigit` `isgraph` `islower` `isprint` `ispunct` `isspace` `isupper` `isxdigit` `tolower` `toupper`, and the `_l` variant of each, for the C locale |
| `locale.h` | `setlocale` `localeconv` `newlocale` `duplocale` `freelocale` `uselocale` `getlocalename_l`, for the C locale |
| `stdlib.h` | `atoi` `atol` `atof` `strtol` `strtoul` `strtoll` `strtoull` `strtod` `strtof` `abs` `labs` `llabs` `div` `ldiv` `lldiv` `qsort` `bsearch` |
| `endian.h` | POSIX.1-2024's `BYTE_ORDER` `LITTLE_ENDIAN` `BIG_ENDIAN` and `be16toh` `be32toh` `be64toh` `htobe16` `htobe32` `htobe64` `htole16` `htole32` `htole64` `le16toh` `le32toh` `le64toh` |
| `stdbit.h` | ISO C23's 14 families (`stdc_leading_zeros` `stdc_leading_ones` `stdc_trailing_zeros` `stdc_trailing_ones` `stdc_first_leading_zero` `stdc_first_leading_one` `stdc_first_trailing_zero` `stdc_first_trailing_one` `stdc_count_zeros` `stdc_count_ones` `stdc_has_single_bit` `stdc_bit_width` `stdc_bit_floor` `stdc_bit_ceil`), each as `_uc` `_us` `_ui` `_ul` `_ull` functions and as a type-generic macro, with `__STDC_ENDIAN_*__` and `__STDC_VERSION_STDBIT_H__` |
| `errno.h` | `errno` (one global, defined by the library), Linux's codes from `EPERM` to `ETIMEDOUT` |
| `stdio.h` | `printf` `dprintf` `sprintf` `snprintf` `vprintf` `vdprintf` `vsprintf` `vsnprintf` |

- **`endian.h`'s byte order is the compiler's** (`__BYTE_ORDER__`), and the conversions do not assume
  it: the host's order is read off the bytes of a `u16` holding 1, and the rule (swap when the host's
  order is not the wanted one) is a function of that order, which `endian_tests.sysl` drives under
  both. Every target runs little-endian, so the big-endian half of the rule is tested at the sysl
  level and not by running on a big-endian machine. The twelve are functions only, which the page allows.
- **`stdbit.h` is cstd's alone: clang does not ship one.** Neither Apple clang 21's resource directory
  nor Homebrew's LLVM 23.1's carries it (both have `stdckdint.h` and no `stdbit.h`; the header is left
  to the C library), so cstd must supply it, and the 70 functions behind it exist in the archive. Were a
  clang to add one, `-I <cstd>/include` is searched before `-isystem <resource>/include`, so cstd's
  would still be the one included. `unsigned long` is measured, so it is 32 bits on a
  Cortex-M. `stdc_bit_ceil` of a value past the largest power of two the type holds is undefined in
  C; here it is 0. The generic macros accept the five unsigned types and refuse anything else at
  compile time.
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
- `strerror` knows `0` and every code `errno.h` defines, in glibc's words; any other code answers
  the fixed text `Unknown error`, because formatting the number in would need a formatter.
- `strtok` keeps one save pointer for the whole program, as C does; `strtok_r` keeps it in the caller's hands.
- `strdup` and `strndup` call an external `malloc` that the program supplies (the Linux half's, a
  board's, or the host's), and set `ENOMEM` themselves when it answers null. `strndup` reads at most
  `n` bytes and allocates only what it copies.
- Only the C locale exists: `locale_t` is an opaque pointer the `_l` functions ignore, `strcoll` is
  `strcmp`, and `strxfrm` copies its source when it fits (and touches nothing when it does not).
- `strerror_r` is POSIX's `int` form: a message cut to fit is still terminated and answers `ERANGE`,
  an unknown code answers `EINVAL` with `Unknown error` in the buffer. `strsignal` gives glibc's
  words for each signal `signal.h` defines and `Unknown signal` for any other number.
- Text with no number in it, or a base C does not define, sets `EINVAL`, as the BSD libcs do. C
  leaves that case unspecified.

## The Linux half (`linux/`)

A C program for keel, or a static Linux aarch64 program, needs a runtime under it as well:
`linux/` is a second project (`cstd_linux`, module `sh.sysl.cstd_linux`) depending on this one by
path, so its archive holds both halves. It is a project of its own because everything in it is a
claim about the machine underneath — a board has no `svc` to make, and a hosted test links the host's
own `write` and `malloc`, which these would replace.

| header | functions |
|---|---|
| (crt0) | `_start`: reads Linux's initial stack, sets `environ`, calls `sysl_init_modules` and `main(argc, argv, envp)`, then `exit` |
| `unistd.h` | `read` `write` `close` `lseek` `_exit` `getpid` `getppid` `brk` `sbrk` `dup` `dup2` `pipe` `unlink` `rmdir` `chdir` `getcwd`, `environ` |
| `fcntl.h` | `open` (variadic, the mode read only under `O_CREAT`/`O_TMPFILE`) `openat`, arm64's flag values |
| `sys/stat.h` | `stat` `fstat` `mkdir`, the 128-byte asm-generic `struct stat` |
| `time.h` / `signal.h` | `nanosleep` `clock_gettime` / `kill` |
| `stdlib.h` | `malloc` `calloc` `realloc` `free` `exit` `_Exit` `atexit` (32 handlers, last first) `abort` (SIGABRT) |

Each call is one `svc #0` (x8 the number, x0–x5 the arguments); an answer from -4095 to -1 sets
`errno` and answers -1. `errno`'s numbers are Linux's.

`malloc` is `sh.sysl.cstd`'s `Heap` (`heap.sysl`, tested on the host with an array standing in for
the break) over `brk`: first fit from an address-ordered free list, freed neighbours merged at once,
16-byte aligned payloads, `realloc` in place where the block can be cut, can take its free
neighbour, or is the last before the break. **It is not thread-safe**: one heap, no lock, until
pthreads arrive. A program that moves the break itself after its first `malloc` gets `ENOMEM` from
a `malloc` that needs to grow.

```
sysl build-c linux --target aarch64-freestanding -o libcstd.a
clang --target=aarch64-linux-gnu -ffreestanding -nostdinc -isystem "$(clang -print-resource-dir)/include" \
      -I <cstd>/include -static -fno-pic -O2 -c prog.c
ld.lld -static -e _start -o prog prog.o libcstd.a
```

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
- `n = 0`, exact-fit and one-short buffers for `strlcpy`, `strlcat`, `stpncpy`, `strxfrm` and
  `strerror_r`, and `errno` left alone where the function promises it
- `strdup`/`strndup` against a `malloc` the case supplies in its prelude: the size asked for, and
  `ENOMEM` from a refusal
- `memmem` with an empty needle, a needle longer than the haystack, and a match at the very end
- every `<ctype.h>` function over EOF and 0–255, and each `_l` variant agreeing with its plain twin
  there, given a locale object and given `LC_GLOBAL_LOCALE`
- the names `setlocale` and `newlocale` accept and refuse, `newlocale`'s `ENOENT` and `EINVAL`,
  every `localeconv` field, and `uselocale` handing back the locale it replaced
- every `<stdbit.h>` function, through the function and the type-generic macro, over all 256 `unsigned
  char` values and a spread of patterns at each wider width, against a bit-by-bit reference written in
  the test, plus hand-worked tables of the edge values (0, all ones, the top bit alone, `bit_ceil`
  past the top)
- `<endian.h>` as the bytes memory holds, and its reversal rule under both host orders
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

And one ordinary case earns its keep here: `strnlen` and `strndup` over an unterminated array within
their `n` must not read past it, which only ASan can see.

The freestanding builds are checked with:

```
sysl build-lib . --target thumb-freestanding-softfp
sysl build-lib . --target aarch64-freestanding
```

## What is deliberately absent, and why

- **Allocation and `exit` in the portable half.** Where memory comes from and how a process ends are
  the platform's: `linux/` answers both for Linux's ABI; a board's arena is still to come.
- **`FILE` and the rest of stdio, `setjmp`.** Buffered streams are not written yet.
- **`long double`, `strtold` and `%Lf`.** cstd's floats are `double` and `float` only.
- **Locales other than C.** Only the C locale exists: every byte from 0x80 up is in no class,
  `setlocale` and `newlocale` accept only "C", "POSIX" and "" (which is the C locale here), and the
  `_l` functions answer as the C locale whatever `locale_t` they are given. A locale's data would
  have to come from somewhere, and nothing cstd runs on has it. The current locale `uselocale`
  installs is one per program, like `errno`.
- **A per-thread `errno`.** Nothing this library serves has threads yet.

## License

ISC. See `LICENSE`.
