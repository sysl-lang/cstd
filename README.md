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
| `string.h` | `strlen` `strcmp` `strncmp` `strcpy` `strncpy` `strcat` `strncat` `strchr` `strrchr` `strstr` `strspn` `strcspn` `strpbrk` `strtok_r` `memcpy` `memmove` `memset` `memcmp` `memchr` |
| `ctype.h` | `isalnum` `isalpha` `isblank` `iscntrl` `isdigit` `isgraph` `islower` `isprint` `ispunct` `isspace` `isupper` `isxdigit` `tolower` `toupper`, for the C locale |
| `stdlib.h` | `atoi` `atol` `strtol` `strtoul` `strtoll` `strtoull` `abs` `labs` |
| `errno.h` | `errno` (one global, defined by the library), `EINVAL`, `ERANGE` |

- `long` is measured with a `c type`, so `strtol` answers 32 bits on a Cortex-M and 64 on an LP64
  machine.
- The `strto*` functions take base 0 or 2–36, skip leading white space, read one sign and an
  optional `0x`, set `endptr`, and saturate with `ERANGE`.
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

The tests are C programs. Each case is a C snippet with its expected output. It is compiled against
`include/`, linked with the archive `sysl build-c` writes, run, and its output compared. The cases
cover the empty string, overlapping `memmove`, bytes ≥ 0x80, every base, overflow in each `strto*`
with `errno`, and `endptr` positions.

**The suite is not in the tree yet.** On a hosted target, sysl 0.1.0-alpha.6 compiles the exported
`strlen` into a bare `ret` and `memset` into a call to itself (LLVM's loop-idiom recognition, which
the compiler suppresses only on freestanding targets). The suite lands once that compiler fix ships.
The freestanding builds are correct, and are checked with:

```
sysl build-lib . --target thumb-freestanding-softfp
sysl build-lib . --target aarch64-freestanding
```

## What is deliberately absent, and why

- **Anything that allocates** (`malloc` and friends, `strdup`). Where the memory comes from is the
  platform's decision: keel's heap, or a board's static arena. That half arrives with a design for it.
- **stdio, `printf`, `exit`, `setjmp`.** These need a platform underneath (file descriptors, a
  process, a crt0), so they belong with keel's port layer rather than this portable core.
- **`strtod`, `atof` and every float conversion.** sysl's standard library itself goes to C's
  `strtod`/`snprintf` for floats, so a correctly rounded implementation belongs in `sysl.fmt` first.
- **`strtok`.** It keeps hidden state between calls; `strtok_r` is the same thing with the state in
  the caller's hands.
- **Locales.** Only the C locale exists: every byte from 0x80 up is in no class.
- **A per-thread `errno`.** Nothing this library serves has threads yet.

## License

ISC. See `LICENSE`.
