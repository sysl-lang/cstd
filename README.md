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
| `inttypes.h` | every `PRI*`/`SCN*` macro for the target's `<stdint.h>` types, `imaxdiv_t`, `imaxabs` `imaxdiv` `strtoimax` `strtoumax` `wcstoimax` `wcstoumax` |
| `strings.h` | `strcasecmp` `strncasecmp` `strcasecmp_l` `strncasecmp_l` (ASCII folding, C locale), `ffs` `ffsl` `ffsll` |
| `assert.h` | `assert` (honours `NDEBUG` at each inclusion; failure writes `Assertion failed: expr (file: func: line)` to descriptor 2, then `abort`), `static_assert` |
| `endian.h` | POSIX.1-2024's `BYTE_ORDER` `LITTLE_ENDIAN` `BIG_ENDIAN` and `be16toh` `be32toh` `be64toh` `htobe16` `htobe32` `htobe64` `htole16` `htole32` `htole64` `le16toh` `le32toh` `le64toh` |
| `stdbit.h` | ISO C23's 14 families (`stdc_leading_zeros` `stdc_leading_ones` `stdc_trailing_zeros` `stdc_trailing_ones` `stdc_first_leading_zero` `stdc_first_leading_one` `stdc_first_trailing_zero` `stdc_first_trailing_one` `stdc_count_zeros` `stdc_count_ones` `stdc_has_single_bit` `stdc_bit_width` `stdc_bit_floor` `stdc_bit_ceil`), each as `_uc` `_us` `_ui` `_ul` `_ull` functions and as a type-generic macro, with `__STDC_ENDIAN_*__` and `__STDC_VERSION_STDBIT_H__` |
| `errno.h` | `errno` (one global, defined by the library), Linux's codes from `EPERM` to `ETIMEDOUT` |
| `stdio.h` | `printf` `dprintf` `sprintf` `snprintf` `vprintf` `vdprintf` `vsprintf` `vsnprintf` |
| `setjmp.h` | `setjmp` `longjmp` `sigsetjmp` `siglongjmp` |
| `math.h` | all 57 functions of POSIX.1-2024 in `double` and `float`, and in `long double` where it is a `double`; the classification and comparison macros, `HUGE_VAL`, `INFINITY`, `NAN`, the `FP_` and `MATH_` constants and `M_PI` and friends |

- **`setjmp` is assembly per processor** (`sh/sysl/cstd/setjmp.sysl`): aarch64 (x19–x30, sp,
  d8–d15; x18 left alone), thumb in Armv6-M's instructions so every Cortex-M runs it (r4–r11, sp,
  lr, and s16–s31 where the target has floating-point registers, `#if fpu`), and riscv32 (ra, sp,
  s0–s11). `<setjmp.h>` refuses to compile anywhere else. `longjmp` with 0 makes `setjmp` return 1.
- `sigjmp_buf` is `jmp_buf`, and `siglongjmp` is `longjmp`. `sigsetjmp` with a non-zero `savemask`
  reads the mask, and puts it back after a `siglongjmp`, through an external `sigprocmask` that the
  platform supplies, as it supplies `write`. `sigsetjmp` sits in a section of its own, so a board
  that links only `setjmp` with `--gc-sections` needs no `sigprocmask`.

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

### `<math.h>`

- **The algorithms are musl's** (1.2.6), ported into sysl with their structure, constants and tables
  kept so that each file can be read beside the C it came from: Arm's optimized-routines for `exp`,
  `exp2`, `log`, `log2` and `pow`; fdlibm (FreeBSD's msun) for the trigonometric, inverse
  trigonometric, error and gamma functions, `expm1`, `log10`, `log1p` and `cbrt`; musl's own for the
  rest. Their notices are in `LICENSE`.
- **Where musl rounds an intermediate and lands past 1 ulp, the intermediate is carried in
  double-double** (`math_dd.sysl`: two-sum, Dekker's product, and exp, expm1 and log built from the
  tables above without rounding their tails away), and only the answer is rounded. That is `sinh`,
  `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `atan2`, `erf`, `erfc`, `lgamma` and `tgamma`; musl's
  forms measure between 1.2 and 10 ulp, and its `tgamma` (Lanczos) and `acosh` for negative
  arguments are not used at all.
- **Nothing calls a C library.** `sqrt` is AArch64's `fsqrt` or x86-64's `sqrtsd` through
  `llvm.sqrt`, and elsewhere musl's integer square root; `fma` and `fmaf` are the machine's fused
  instruction through `llvm.fma` where it has one at that width (`#if fma_dp`, `#if fma`), and
  elsewhere musl's 128-bit integer `fma` and a `fmaf` rounded to odd; the roundings are integer
  operations on the bits. A test checks the archive's
  undefined symbols on the host, on 32-bit Arm and on AArch64.
- **`float` computes in `double` and rounds once**, except where a function of its own is needed to
  be exact: `fmaf` rounds the double sum to odd before narrowing, which is correct even into float's
  subnormals. `sqrtf`, `fmodf`, `remainderf`, `remquof`, the roundings and the scalings are exact
  through `double`; the others are correctly rounded on every input the tests sample (some 70 000
  each), except `atan2f`, which a tie between two floats can cost half an ulp and a hair (the test
  file says why).
- **`long double`** is `double` on arm64 macOS and 32-bit Arm, and there each `l` function is its
  double counterpart under the `l` name. Where it is binary128 (AArch64 Linux, RISC-V) they are not
  offered, and `math.h` does not declare them; `nexttoward` takes its direction at the type's width on
  both.
- **Errors go to `errno` only**: `math_errhandling` is `MATH_ERRNO`. A domain error sets `EDOM` and
  answers NaN; a pole error or an overflow sets `ERANGE` and answers ±infinity; an underflow answers
  the subnormal or zero and sets nothing, which POSIX permits. A NaN argument is never an error. The
  exception flags are whatever the arithmetic raises, and nothing promises which.
- C Annex F's special values hold throughout: `pow(-1, ±inf)` and `pow(x, ±0)` are 1, `atan2` of
  signed zeros answers ±0 or ±pi, `hypot(inf, NaN)` is inf, every zero keeps its sign.
- The rounding mode is assumed to be to nearest, as musl assumes it.

The largest error measured against MPFR, in units in the last place of the exact answer, over the
inputs the tests run (between 20 000 and 75 000 per function and form):

| function | `double` | `float` |
|---|---|---|
| `sqrt` `fma` `fmod` `remainder` `remquo` `floor` `ceil` `trunc` `round` `rint` `nearbyint` `lrint` `llrint` `lround` `llround` `frexp` `ldexp` `scalbn` `scalbln` `modf` `ilogb` `logb` `nextafter` `nexttoward` `fabs` `copysign` `fdim` `fmax` `fmin` | 0 (exact) | 0 (exact) |
| `exp` `exp2` | 0.503 | 0.500 |
| `expm1` | 0.737 | 0.500 |
| `log` | 0.503 | 0.500 |
| `log2` | 0.526 | 0.500 |
| `log10` | 0.612 | 0.500 |
| `log1p` | 0.787 | 0.500 |
| `pow` | 0.505 | 0.500 |
| `cbrt` | 0.665 | 0.500 |
| `hypot` | 0.939 | 0.500 |
| `sin` `cos` `tan` | 0.740 0.730 0.798 | 0.500 |
| `asin` `acos` `atan` | 0.799 0.815 0.766 | 0.500 |
| `atan2` | 0.803 | 0.500 |
| `sinh` `cosh` `tanh` | 0.647 0.580 0.639 | 0.500 |
| `asinh` `acosh` `atanh` | 0.500 0.501 0.500 | 0.500 |
| `erf` `erfc` | 0.869 0.746 | 0.500 |
| `tgamma` | 0.820 | 0.500 |
| `lgamma` | 0.614 for x > 0; 0.998 for x < 0, except next to its zeros | 0.500 |

The tests hold every elementary function to 1 ulp and the exact ones to the bit. **lgamma** below
zero is the one exception: lgamma(x) = log(pi / |x sin(pi x)|) - lgamma(-x) subtracts two nearly
equal numbers wherever lgamma(x) is close to one of its zeros (two in each unit interval from -2
down), and there the answer is held to an absolute error, measured at most 7.6 × 10^-18, rather than
to ulps.

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

`malloc` is `sh.sysl.cstd`'s `BreakHeap` (`malloc.sysl`), the same allocator a board's defaults
are, over this half's `sbrk`; its bookkeeping is `Heap` (`heap.sysl`, tested on the host with an
array standing in for the break). The break moves 64 KiB at a time where the kernel allows it, and
by exactly what is needed where it does not. First fit from an address-ordered free list, freed neighbours merged at once,
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

### On a board

On any other freestanding target the calls of the `unistd.h`, `fcntl.h`, `sys/stat.h`, `time.h` and
`signal.h` rows above but `_exit`, and `sigprocmask`, which `sigsetjmp` reaches, are weak defaults in
the core (`stubs.sysl`): each sets `errno` to `ENOSYS` and answers its failure value (-1, a null
`getcwd`, `(void *)-1` from `sbrk`). A board overrides any one by defining it strongly, in its sysl
(`@export("write")`) or its C (`ssize_t write(int, const void *, size_t)`), and the link keeps the
board's with no duplicate-symbol error. A static linker does not pull an archive member in to replace
a weak definition it already has, so a definition in a board's C is kept where something else in that
file is linked too — its startup code, or a function the board calls. A hosted target has none of
the defaults: its calls are the host libc's.

The `stdlib.h` row is defaulted too, so a board program links with nothing of its own — sysl's
runtime reaches `malloc`, `free` and `abort` for any sysl code that allocates or traps:

| function | the default |
|---|---|
| `malloc` `calloc` `realloc` `free` | the same `Heap` as the Linux half, growing through `sbrk` (`malloc.sysl`). With cstd's own `sbrk` every allocation is null with `ENOMEM`; a board that defines `sbrk` over its heap region has a working allocator with nothing else written. The heap grows 4 KiB at a time where the region has room, and by exactly what it needs where it does not |
| `atexit` `exit` | the Linux half's 32-slot table, which lives in the core and serves both (`exit.sysl`); `exit` runs the handlers last first and then calls `_Exit` |
| `_Exit` `_exit` `abort` | trap: the processor's undefined instruction (`udf` on Arm), so a debugger stops there and the board's fault handler sees it, rather than a spin nothing reports. A board that can end a run — semihosting, a reset — defines `_Exit` and `_exit`, and `exit` ends through them |

The four allocation functions are one allocator: a board with its own replaces all four, since cstd's
`realloc` and `free` only understand blocks cstd's `malloc` handed out.

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

**The mathematics is judged by MPFR**, which computes every function correctly rounded at any
precision (`sh/sysl/cstd/math_oracle.sysl`; the suite needs Homebrew's `mpfr` and `gmp` under
`/opt/homebrew/opt`). A case is C over `tests/math_gen.h`: it calls a cstd function on the special
values (zeros, infinities, NaNs, the ends of the subnormal and normal ranges, one and its
neighbours), on a fixed-seed spread of arbitrary bit patterns, and on the places its algorithm has
edges -- arguments next to multiples of pi/2 and past 2^20 pi/2 for the trigonometric functions, the
overflow and underflow thresholds, the neighbourhood of 1 for a logarithm, perfect squares and cubes,
halves for the roundings, cancellation for `fma`, each fit's interval boundaries -- and prints the
bits. A C program built once per run against MPFR reads those lines and checks each answer against
the correctly rounded one (to the bit, for the exact functions) or against the exact one to within the
function's bound in ulps, and checks `errno` on every line. It appends the largest error it saw to
`.sysl/c-tests/ulp-report.txt`, which is where the table above comes from. The `math_c_tests.sysl`
cases pin the header's macros and constants and the `errno` and Annex F behaviour by name.

The freestanding builds are checked with:

```
sysl build-lib . --target thumb-freestanding-softfp
sysl build-lib . --target thumb-freestanding
sysl build-lib . --target thumbv7em-freestanding
sysl build-lib . --target thumbv6m-freestanding
sysl build-lib . --target riscv32-freestanding
sysl build-lib . --target aarch64-freestanding
```

**cstd as a package's C library** is checked by `libc_probe_tests.sysl` over `tests/libc_probe`, a
package that vendors a C file including `<string.h>`, `<ctype.h>`, `<inttypes.h>` and `<math.h>` and
takes cstd with `if = "!libc"`. Built with `sysl build-c` for `thumbv7em-freestanding` and
`riscv32-freestanding`, its archive must define `strlen`, `toupper`, `strtoimax` and `sqrt`; built for
the host, where the real libc answers, it must define nothing but the probe's own symbols.

The `setjmp` cases run on the host, which is aarch64: a jump across frames, 0 becoming 1, volatile
locals, x19–x28 and d8–d15 overwritten by the skipped frames and put back, nested buffers, a
hundred thousand jumps with the stack where it was, and `sigsetjmp`'s mask through the host's
`sigprocmask`. The thumb and riscv32 arms run on QEMU (`microbit`, a Cortex-M0; `mps2-an385`, a
Cortex-M3; `virt`, RV32; `mps2-an386`, a Cortex-M4F, where s16–s31 are overwritten by the skipped frames and
must come back): `tests/qemu/setjmp.c` is linked against cstd built for the target by
`tests/qemu/archive` and reports through semihosting. Those three cases need `qemu-system-arm`,
`qemu-system-riscv32`, `ld.lld` and Homebrew's LLVM clang, skip where any is missing, and skip
under ASan, whose flags would reach the bare-metal build. Under ASan a C caller of `longjmp` calls
`__asan_handle_no_return` itself, because the prototype is `noreturn`, so the skipped frames'
shadow is cleared without cstd doing anything.

**The operating-system defaults** are checked by `stubs_tests.sysl`: built for
`thumbv7em-freestanding`, cstd's archive lists every defaulted call as `W`; built for
`aarch64-freestanding`, the Linux half's lists each as `T` with no `W` beside it; and on QEMU's
`mps2-an386`, `tests/qemu/stubs.c` linked against `tests/board_probe` — a package whose C supplies a
strong `write` over semihosting — prints through the board's `write` and sees every other call answer
its failure value with `errno` ENOSYS. Making the default `write` strong turns that link into
`duplicate symbol: write`.

**The allocator and exit defaults** are checked by `board_defaults_tests.sysl`, the same way: `W` on
thumbv7em and `T` alone in the Linux half for all nine; on `mps2-an386`, `tests/qemu/heap.c` with the
board's `sbrk` over an 8 KiB array (`tests/board_probe/c/board_heap.c`) writes and reads back,
sees `calloc` zero a reused block, `realloc` keep the bytes as it moves, `free` then `malloc` reuse
the block, `ENOMEM` past the region and from an overflowing `calloc`, and 32 `atexit` handlers (a 33rd
refused) run last first by `exit` before the board's `_Exit`; `tests/qemu/nosbrk.c`, which leaves the
board's `sbrk` out of the link, sees `malloc`, `calloc` and `realloc` answer null with `ENOMEM`; and
`tests/qemu/sysl_alloc.c` links `tests/alloc_probe` — sysl code filling a growable buffer — into a
whole image with no `malloc`, `free` or `abort` of its own and gets the right sum.

## What is deliberately absent, and why

- **Allocation and `exit` in the portable half.** Where memory comes from and how a process ends are
  the platform's: `linux/` answers both for Linux's ABI; a board's arena is still to come.
- **`FILE` and the rest of stdio.** Buffered streams are not written yet.
- **`long double` beyond binary64: `strtold`, `%Lf`, and the `l` functions of `<math.h>` where
  `long double` is binary128.** cstd's floats are `double` and `float`; where `long double` is a
  `double` the `<math.h>` `l` functions are offered as their `double` counterparts, and where it is
  binary128 a quad-precision argument reduction, polynomials and tables are still to be written.
  A `double` answer widened would be wrong in its last 60 bits, so nothing is offered there.
- **Locales other than C.** Only the C locale exists: every byte from 0x80 up is in no class,
  `setlocale` and `newlocale` accept only "C", "POSIX" and "" (which is the C locale here), and the
  `_l` functions answer as the C locale whatever `locale_t` they are given. A locale's data would
  have to come from somewhere, and nothing cstd runs on has it. The current locale `uselocale`
  installs is one per program, like `errno`.
- **A per-thread `errno`.** Nothing this library serves has threads yet.

## License

ISC. See `LICENSE`, which also carries the notices of musl, Arm and Sun's fdlibm for the
mathematics.
