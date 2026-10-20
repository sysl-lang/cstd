# Test coverage

Every `@export` under `sh/` and `linux/`, by the header that declares it, with the tests that call it. A
test counts only where it calls the function itself, or drives the header macro that is the only way
to reach it, and checks the answer; a function reached only from inside another tested one is
untested. Host cases are the `*_tests.sysl` files under `sh/sysl/cstd/`; a `.c` name is an image run
under QEMU from `tests/qemu/`.

An export is one of three kinds. **portable** is built for every target and called from the host
cases. **freestanding** is a weak default built only where the target has no libc, so only a QEMU
image reaches it, and only one that does not link a board definition replacing it. **linux** is the
Linux half (`linux/`), built for `aarch64-freestanding`: `linux_tests.sysl` links the C programs of
`tests/linux/` against it into static ELFs and runs them on Linux, in the `cstd-test` colima VM, and
`stubs_tests.sysl` and `board_defaults_tests.sysl` check that its archive defines each symbol
strongly.

## Summary

| header | exported | tested | untested |
|---|---:|---:|---:|
| `assert.h` | 1 | 1 | 0 |
| no header | 2 | 2 | 0 |
| `signal.h` | 9 | 9 | 0 |
| `fcntl.h` | 8 | 8 | 0 |
| `inttypes.h` | 6 | 6 | 0 |
| `sys/stat.h` | 6 | 6 | 0 |
| `locale.h` | 7 | 7 | 0 |
| `strings.h` | 7 | 7 | 0 |
| `stdio.h` | 10 | 10 | 0 |
| `endian.h` | 12 | 12 | 0 |
| `ctype.h` | 28 | 28 | 0 |
| `unistd.h` | 33 | 33 | 0 |
| `time.h` | 34 | 34 | 0 |
| `string.h` | 36 | 36 | 0 |
| `stdlib.h` | 43 | 43 | 0 |
| `stdbit.h` | 70 | 70 | 0 |
| `math.h` | 171 | 171 | 0 |
| **all** | **483** | **483** | **0** |

The Linux half's 46 are tested only where the `cstd-test` colima profile is running (README §
Testing); elsewhere `linux_tests.sysl` skips.

Findings:

- `cstd_start` is the Linux half's entry, reached from `_start`; `cstd_sigsetjmp_tail` and
  `__cstd_assert_fail` are internal names the `sigsetjmp` and `assert` macros reach, so no header
  declares them for a caller.

## `assert.h`

| function | kind | tests |
|---|---|---|
| `__cstd_assert_fail` | portable | assert_tests.sysl (through `assert`) |

## `signal.h`

| function | kind | tests |
|---|---|---|
| `kill` | freestanding | stubs.c |
| `kill` | linux | linux_tests.sysl |
| `sigprocmask` | freestanding | stubs.c |
| `sigprocmask` | linux | linux_tests.sysl |
| `sigemptyset` | portable | sigset_tests.sysl |
| `sigfillset` | portable | sigset_tests.sysl |
| `sigaddset` | portable | sigset_tests.sysl |
| `sigdelset` | portable | sigset_tests.sysl |
| `sigismember` | portable | sigset_tests.sysl |

## No header

| function | kind | tests |
|---|---|---|
| `cstd_sigsetjmp_tail` | portable | setjmp_tests.sysl (through `sigsetjmp`) |
| `cstd_start` | linux | linux_tests.sysl |

## `fcntl.h`

| function | kind | tests |
|---|---|---|
| `creat` | freestanding | stubs.c |
| `creat` | linux | linux_tests.sysl |
| `fcntl` | freestanding | stubs.c |
| `fcntl` | linux | linux_tests.sysl |
| `open` | freestanding | stubs.c |
| `open` | linux | linux_tests.sysl |
| `openat` | freestanding | stubs.c |
| `openat` | linux | linux_tests.sysl |

## `inttypes.h`

| function | kind | tests |
|---|---|---|
| `imaxabs` | portable | inttypes_tests.sysl, probe.c |
| `imaxdiv` | portable | inttypes_tests.sysl, probe.c |
| `strtoimax` | portable | inttypes_tests.sysl, probe.c |
| `strtoumax` | portable | inttypes_tests.sysl |
| `wcstoimax` | portable | inttypes_tests.sysl |
| `wcstoumax` | portable | inttypes_tests.sysl |

## `sys/stat.h`

| function | kind | tests |
|---|---|---|
| `fstat` | freestanding | stubs.c |
| `fstat` | linux | linux_tests.sysl |
| `mkdir` | freestanding | stubs.c |
| `mkdir` | linux | linux_tests.sysl |
| `stat` | freestanding | stubs.c |
| `stat` | linux | linux_tests.sysl |

## `locale.h`

| function | kind | tests |
|---|---|---|
| `duplocale` | portable | locale_tests.sysl |
| `freelocale` | portable | locale_tests.sysl |
| `getlocalename_l` | portable | locale_tests.sysl |
| `localeconv` | portable | locale_tests.sysl |
| `newlocale` | portable | locale_tests.sysl |
| `setlocale` | portable | locale_tests.sysl |
| `uselocale` | portable | locale_tests.sysl |

## `strings.h`

| function | kind | tests |
|---|---|---|
| `ffs` | portable | strings_tests.sysl |
| `ffsl` | portable | strings_tests.sysl |
| `ffsll` | portable | strings_tests.sysl |
| `strcasecmp` | portable | strings_tests.sysl |
| `strcasecmp_l` | portable | strings_tests.sysl |
| `strncasecmp` | portable | strings_tests.sysl |
| `strncasecmp_l` | portable | strings_tests.sysl |

## `stdio.h`

| function | kind | tests |
|---|---|---|
| `dprintf` | portable | printf_tests.sysl |
| `printf` | portable | printf_tests.sysl |
| `putchar` | portable | stdio_tests.sysl, defaults.c |
| `puts` | portable | stdio_tests.sysl, defaults.c |
| `snprintf` | portable | printf_float_tests.sysl, printf_tests.sysl |
| `sprintf` | portable | printf_float_tests.sysl, printf_tests.sysl |
| `vdprintf` | portable | printf_tests.sysl |
| `vprintf` | portable | printf_tests.sysl |
| `vsnprintf` | portable | locale_tests.sysl, printf_float_tests.sysl, printf_tests.sysl, string_posix_tests.sysl, string_tests.sysl, time_tests.sysl |
| `vsprintf` | portable | printf_tests.sysl |

## `endian.h`

| function | kind | tests |
|---|---|---|
| `be16toh` | portable | endian_tests.sysl |
| `be32toh` | portable | endian_tests.sysl |
| `be64toh` | portable | endian_tests.sysl |
| `htobe16` | portable | endian_tests.sysl |
| `htobe32` | portable | endian_tests.sysl |
| `htobe64` | portable | endian_tests.sysl |
| `htole16` | portable | endian_tests.sysl |
| `htole32` | portable | endian_tests.sysl |
| `htole64` | portable | endian_tests.sysl |
| `le16toh` | portable | endian_tests.sysl |
| `le32toh` | portable | endian_tests.sysl |
| `le64toh` | portable | endian_tests.sysl |

## `ctype.h`

| function | kind | tests |
|---|---|---|
| `isalnum` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isalnum_l` | portable | locale_tests.sysl |
| `isalpha` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isalpha_l` | portable | locale_tests.sysl |
| `isblank` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isblank_l` | portable | locale_tests.sysl |
| `iscntrl` | portable | ctype_tests.sysl, locale_tests.sysl |
| `iscntrl_l` | portable | locale_tests.sysl |
| `isdigit` | portable | ctype_tests.sysl, locale_tests.sysl, probe.c |
| `isdigit_l` | portable | locale_tests.sysl |
| `isgraph` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isgraph_l` | portable | locale_tests.sysl |
| `islower` | portable | ctype_tests.sysl, locale_tests.sysl |
| `islower_l` | portable | locale_tests.sysl |
| `isprint` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isprint_l` | portable | locale_tests.sysl |
| `ispunct` | portable | ctype_tests.sysl, locale_tests.sysl |
| `ispunct_l` | portable | locale_tests.sysl |
| `isspace` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isspace_l` | portable | locale_tests.sysl |
| `isupper` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isupper_l` | portable | locale_tests.sysl |
| `isxdigit` | portable | ctype_tests.sysl, locale_tests.sysl |
| `isxdigit_l` | portable | locale_tests.sysl |
| `tolower` | portable | ctype_tests.sysl, locale_tests.sysl |
| `tolower_l` | portable | locale_tests.sysl |
| `toupper` | portable | ctype_tests.sysl, locale_tests.sysl, probe.c |
| `toupper_l` | portable | locale_tests.sysl |

## `stdlib.h`

| function | kind | tests |
|---|---|---|
| `_Exit` | freestanding | defaults.c |
| `_Exit` | linux | linux_tests.sysl |
| `abort` | freestanding | defaults.c |
| `abort` | linux | linux_tests.sysl |
| `abs` | portable | stdlib_tests.sysl |
| `atexit` | freestanding | heap.c |
| `atexit` | linux | linux_tests.sysl |
| `atof` | portable | strtod_tests.sysl |
| `atoi` | portable | stdlib_tests.sysl |
| `atol` | portable | stdlib_tests.sysl |
| `atoll` | portable | stdlib_tests.sysl |
| `bsearch` | portable | stdlib_tests.sysl |
| `calloc` | freestanding | heap.c, nosbrk.c |
| `calloc` | linux | linux_tests.sysl |
| `div` | portable | stdlib_tests.sysl |
| `exit` | freestanding | heap.c, sysl_alloc.c |
| `exit` | linux | linux_tests.sysl |
| `free` | freestanding | heap.c |
| `free` | linux | linux_tests.sysl |
| `getenv` | linux | linux_tests.sysl |
| `labs` | portable | stdlib_tests.sysl |
| `ldiv` | portable | stdlib_tests.sysl |
| `llabs` | portable | stdlib_tests.sysl |
| `lldiv` | portable | stdlib_tests.sysl |
| `malloc` | freestanding | heap.c, nosbrk.c |
| `malloc` | linux | linux_tests.sysl |
| `mblen` | portable | multibyte_tests.sysl |
| `mbstowcs` | portable | multibyte_tests.sysl |
| `mbtowc` | portable | multibyte_tests.sysl |
| `qsort` | portable | stdlib_tests.sysl |
| `rand` | portable | stdlib_tests.sysl |
| `realloc` | freestanding | heap.c, nosbrk.c |
| `realloc` | linux | linux_tests.sysl |
| `srand` | portable | stdlib_tests.sysl |
| `strtod` | portable | printf_float_tests.sysl, strtod_tests.sysl |
| `strtof` | portable | strtod_tests.sysl |
| `strtol` | portable | stdlib_tests.sysl, string_tests.sysl |
| `strtold` | portable (where `long double` is binary64) | strtod_tests.sysl |
| `strtoll` | portable | stdlib_tests.sysl |
| `strtoul` | portable | stdlib_tests.sysl |
| `strtoull` | portable | stdlib_tests.sysl |
| `wcstombs` | portable | multibyte_tests.sysl |
| `wctomb` | portable | multibyte_tests.sysl |

## `unistd.h`

| function | kind | tests |
|---|---|---|
| `_exit` | freestanding | defaults.c |
| `_exit` | linux | linux_tests.sysl |
| `brk` | freestanding | stubs.c |
| `brk` | linux | linux_tests.sysl |
| `chdir` | freestanding | stubs.c |
| `chdir` | linux | linux_tests.sysl |
| `close` | freestanding | stubs.c |
| `close` | linux | linux_tests.sysl |
| `dup` | freestanding | stubs.c |
| `dup` | linux | linux_tests.sysl |
| `dup2` | freestanding | stubs.c |
| `dup2` | linux | linux_tests.sysl |
| `environ` | linux | linux_tests.sysl |
| `getcwd` | freestanding | stubs.c |
| `getcwd` | linux | linux_tests.sysl |
| `getpid` | freestanding | stubs.c |
| `getpid` | linux | linux_tests.sysl |
| `getppid` | freestanding | stubs.c |
| `getppid` | linux | linux_tests.sysl |
| `lseek` | freestanding | stubs.c |
| `lseek` | linux | linux_tests.sysl |
| `pipe` | freestanding | stubs.c |
| `pipe` | linux | linux_tests.sysl |
| `read` | freestanding | stubs.c |
| `read` | linux | linux_tests.sysl |
| `rmdir` | freestanding | stubs.c |
| `rmdir` | linux | linux_tests.sysl |
| `sbrk` | freestanding | stubs.c |
| `sbrk` | linux | linux_tests.sysl |
| `unlink` | freestanding | stubs.c |
| `unlink` | linux | linux_tests.sysl |
| `write` | freestanding | defaults.c |
| `write` | linux | linux_tests.sysl |

## `time.h`

| function | kind | tests |
|---|---|---|
| `clock` | freestanding | clock.c, stubs.c |
| `clock_getres` | freestanding | stubs.c |
| `clock_getres` | linux | linux_tests.sysl |
| `clock_gettime` | freestanding | stubs.c |
| `clock_gettime` | linux | linux_tests.sysl |
| `clock_nanosleep` | freestanding | stubs.c |
| `clock_nanosleep` | linux | linux_tests.sysl |
| `clock_settime` | freestanding | stubs.c |
| `clock_settime` | linux | linux_tests.sysl |
| `difftime` | portable | clock.c, time_tests.sysl |
| `gmtime` | portable | clock.c, time_tests.sysl |
| `gmtime_r` | portable | clock.c, time_tests.sysl |
| `localtime` | portable | time_tests.sysl |
| `localtime_r` | portable | time_tests.sysl |
| `mktime` | portable | time_tests.sysl |
| `nanosleep` | freestanding | stubs.c |
| `nanosleep` | linux | linux_tests.sysl |
| `strftime` | portable | clock.c, time_tests.sysl |
| `strftime_l` | portable | time_tests.sysl |
| `time` | freestanding | clock.c, stubs.c |
| `timegm` | portable | clock.c, time_tests.sysl |
| `timer_create` | freestanding | stubs.c |
| `timer_create` | linux | linux_tests.sysl |
| `timer_delete` | freestanding | stubs.c |
| `timer_delete` | linux | linux_tests.sysl |
| `timer_getoverrun` | freestanding | stubs.c |
| `timer_getoverrun` | linux | linux_tests.sysl |
| `timer_gettime` | freestanding | stubs.c |
| `timer_gettime` | linux | linux_tests.sysl |
| `timer_settime` | freestanding | stubs.c |
| `timer_settime` | linux | linux_tests.sysl |
| `timespec_get` | freestanding | clock.c, stubs.c |
| `tzname` | portable | time_tests.sysl |
| `tzset` | portable | time_tests.sysl |

## `string.h`

| function | kind | tests |
|---|---|---|
| `memchr` | portable | string_tests.sysl |
| `memcmp` | portable | string_tests.sysl |
| `memcpy` | portable | endian_tests.sysl, math_c_tests.sysl, printf_float_tests.sysl, string_tests.sysl, strtod_tests.sysl |
| `memmem` | portable | string_posix_tests.sysl |
| `memmove` | portable | string_tests.sysl |
| `memset` | portable | printf_float_tests.sysl, printf_tests.sysl, string_posix_tests.sysl, string_tests.sysl, strtod_tests.sysl |
| `stpcpy` | portable | string_posix_tests.sysl |
| `stpncpy` | portable | string_posix_tests.sysl |
| `strcat` | portable | string_tests.sysl |
| `strchr` | portable | string_tests.sysl |
| `strcmp` | portable | inttypes_tests.sysl, string_tests.sysl |
| `strcoll` | portable | string_posix_tests.sysl |
| `strcoll_l` | portable | string_posix_tests.sysl |
| `strcpy` | portable | string_tests.sysl, strtod_tests.sysl |
| `strcspn` | portable | string_tests.sysl |
| `strdup` | portable | string_posix_tests.sysl |
| `strerror` | portable | string_posix_tests.sysl, string_tests.sysl |
| `strerror_l` | portable | string_posix_tests.sysl |
| `strerror_r` | portable | string_posix_tests.sysl |
| `strlcat` | portable | string_posix_tests.sysl |
| `strlcpy` | portable | string_posix_tests.sysl |
| `strlen` | portable | c_case_tests.sysl, locale_tests.sysl, probe.c, string_posix_tests.sysl, string_tests.sysl |
| `strncat` | portable | string_tests.sysl |
| `strncmp` | portable | string_tests.sysl |
| `strncpy` | portable | string_tests.sysl |
| `strndup` | portable | string_posix_tests.sysl |
| `strnlen` | portable | string_posix_tests.sysl, string_tests.sysl |
| `strpbrk` | portable | string_tests.sysl |
| `strrchr` | portable | string_tests.sysl |
| `strsignal` | portable | string_posix_tests.sysl |
| `strspn` | portable | string_tests.sysl |
| `strstr` | portable | string_tests.sysl |
| `strtok` | portable | string_tests.sysl |
| `strtok_r` | portable | string_tests.sysl |
| `strxfrm` | portable | string_posix_tests.sysl |
| `strxfrm_l` | portable | string_posix_tests.sysl |

## `stdbit.h`

Every function at every width, through the function and the type-generic macro, in
stdbit_tests.sysl: the 14 families `stdc_leading_zeros`, `stdc_leading_ones`, `stdc_trailing_zeros`,
`stdc_trailing_ones`, `stdc_first_leading_zero`, `stdc_first_leading_one`, `stdc_first_trailing_zero`,
`stdc_first_trailing_one`, `stdc_count_zeros`, `stdc_count_ones`, `stdc_has_single_bit`,
`stdc_bit_width`, `stdc_bit_floor` and `stdc_bit_ceil`, each with the suffixes `_uc`, `_us`, `_ui`,
`_ul` and `_ull` — 70 exports, all portable, none untested.

## `math.h`

The `l` forms are checked in math_c_tests.sysl as agreeing with the double form, which is what they
are where cstd defines them (long double is double there). The double and float forms are judged
against MPFR.

| function | kind | tests |
|---|---|---|
| `acos` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `acosf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `acosh` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `acoshf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `acoshl` | portable | math_c_tests.sysl |
| `acosl` | portable | math_c_tests.sysl |
| `asin` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `asinf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `asinh` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `asinhf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `asinhl` | portable | math_c_tests.sysl |
| `asinl` | portable | math_c_tests.sysl |
| `atan` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `atan2` | portable | math_c_tests.sysl, math_hyp_tests.sysl |
| `atan2f` | portable | math_hyp_tests.sysl |
| `atan2l` | portable | math_c_tests.sysl |
| `atanf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `atanh` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `atanhf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `atanhl` | portable | math_c_tests.sysl |
| `atanl` | portable | math_c_tests.sysl |
| `cbrt` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_root_tests.sysl |
| `cbrtf` | portable | math_float_cr_tests.sysl, math_root_tests.sysl |
| `cbrtl` | portable | math_c_tests.sysl |
| `ceil` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `ceilf` | portable | math_exact_tests.sysl |
| `ceill` | portable | math_c_tests.sysl |
| `copysign` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `copysignf` | portable | math_exact_tests.sysl |
| `copysignl` | portable | math_c_tests.sysl |
| `cos` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_trig_tests.sysl |
| `cosf` | portable | math_float_cr_tests.sysl, math_trig_tests.sysl |
| `cosh` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `coshf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `coshl` | portable | math_c_tests.sysl |
| `cosl` | portable | math_c_tests.sysl |
| `erf` | portable | math_c_tests.sysl, math_erf_tests.sysl, math_float_cr_tests.sysl |
| `erfc` | portable | math_c_tests.sysl, math_erf_tests.sysl, math_float_cr_tests.sysl |
| `erfcf` | portable | math_erf_tests.sysl, math_float_cr_tests.sysl |
| `erfcl` | portable | math_c_tests.sysl |
| `erff` | portable | math_erf_tests.sysl, math_float_cr_tests.sysl |
| `erfl` | portable | math_c_tests.sysl |
| `exp` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `exp2` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `exp2f` | portable | math_exp_tests.sysl, math_float_cr_tests.sysl |
| `exp2l` | portable | math_c_tests.sysl |
| `expf` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `expl` | portable | math_c_tests.sysl |
| `expm1` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `expm1f` | portable | math_exp_tests.sysl, math_float_cr_tests.sysl |
| `expm1l` | portable | math_c_tests.sysl |
| `fabs` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fabsf` | portable | math_exact_tests.sysl |
| `fabsl` | portable | math_c_tests.sysl |
| `fdim` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fdimf` | portable | math_exact_tests.sysl |
| `fdiml` | portable | math_c_tests.sysl |
| `floor` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `floorf` | portable | math_exact_tests.sysl |
| `floorl` | portable | math_c_tests.sysl |
| `fma` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fmaf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fmal` | portable | math_c_tests.sysl |
| `fmax` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fmaxf` | portable | math_exact_tests.sysl |
| `fmaxl` | portable | math_c_tests.sysl |
| `fmin` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fminf` | portable | math_exact_tests.sysl |
| `fminl` | portable | math_c_tests.sysl |
| `fmod` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `fmodf` | portable | math_exact_tests.sysl |
| `fmodl` | portable | math_c_tests.sysl |
| `frexp` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `frexpf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `frexpl` | portable | math_c_tests.sysl |
| `hypot` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_root_tests.sysl |
| `hypotf` | portable | math_float_cr_tests.sysl, math_root_tests.sysl |
| `hypotl` | portable | math_c_tests.sysl |
| `ilogb` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `ilogbf` | portable | math_exact_tests.sysl |
| `ilogbl` | portable | math_c_tests.sysl |
| `ldexp` | portable | math_c_tests.sysl, math_exact_tests.sysl, math_exp_tests.sysl, math_trig_tests.sysl |
| `ldexpf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `ldexpl` | portable | math_c_tests.sysl |
| `lgamma` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_gamma_tests.sysl |
| `lgammaf` | portable | math_float_cr_tests.sysl, math_gamma_tests.sysl |
| `lgammal` | portable | math_c_tests.sysl |
| `llrint` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `llrintf` | portable | math_exact_tests.sysl |
| `llrintl` | portable | math_c_tests.sysl |
| `llround` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `llroundf` | portable | math_exact_tests.sysl |
| `llroundl` | portable | math_c_tests.sysl |
| `log` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log10` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log10f` | portable | math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log10l` | portable | math_c_tests.sysl |
| `log1p` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log1pf` | portable | math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log1pl` | portable | math_c_tests.sysl |
| `log2` | portable | math_c_tests.sysl, math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log2f` | portable | math_exp_tests.sysl, math_float_cr_tests.sysl |
| `log2l` | portable | math_c_tests.sysl |
| `logb` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `logbf` | portable | math_exact_tests.sysl |
| `logbl` | portable | math_c_tests.sysl |
| `logf` | portable | math_exp_tests.sysl, math_float_cr_tests.sysl |
| `logl` | portable | math_c_tests.sysl |
| `lrint` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `lrintf` | portable | math_exact_tests.sysl |
| `lrintl` | portable | math_c_tests.sysl |
| `lround` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `lroundf` | portable | math_exact_tests.sysl |
| `lroundl` | portable | math_c_tests.sysl |
| `modf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `modff` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `modfl` | portable | math_c_tests.sysl |
| `nan` | portable | math_c_tests.sysl, printf_float_tests.sysl |
| `nanf` | portable | math_c_tests.sysl |
| `nanl` | portable | math_c_tests.sysl |
| `nearbyint` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `nearbyintf` | portable | math_exact_tests.sysl |
| `nearbyintl` | portable | math_c_tests.sysl |
| `nextafter` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `nextafterf` | portable | math_exact_tests.sysl |
| `nextafterl` | portable | math_c_tests.sysl |
| `nexttoward` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `nexttowardf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `nexttowardl` | portable | math_c_tests.sysl |
| `pow` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_pow_tests.sysl |
| `powf` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_pow_tests.sysl |
| `powl` | portable | math_c_tests.sysl |
| `remainder` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `remainderf` | portable | math_exact_tests.sysl |
| `remainderl` | portable | math_c_tests.sysl |
| `remquo` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `remquof` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `remquol` | portable | math_c_tests.sysl |
| `rint` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `rintf` | portable | math_exact_tests.sysl |
| `rintl` | portable | math_c_tests.sysl |
| `round` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `roundf` | portable | math_exact_tests.sysl |
| `roundl` | portable | math_c_tests.sysl |
| `scalbln` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `scalblnf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `scalblnl` | portable | math_c_tests.sysl |
| `scalbn` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `scalbnf` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `scalbnl` | portable | math_c_tests.sysl |
| `sin` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_trig_tests.sysl |
| `sinf` | portable | math_float_cr_tests.sysl, math_trig_tests.sysl |
| `sinh` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `sinhf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `sinhl` | portable | math_c_tests.sysl |
| `sinl` | portable | math_c_tests.sysl |
| `sqrt` | portable | math_c_tests.sysl, math_exact_tests.sysl, probe.c |
| `sqrtf` | portable | math_exact_tests.sysl |
| `sqrtl` | portable | math_c_tests.sysl |
| `tan` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_trig_tests.sysl |
| `tanf` | portable | math_float_cr_tests.sysl, math_trig_tests.sysl |
| `tanh` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `tanhf` | portable | math_float_cr_tests.sysl, math_hyp_tests.sysl |
| `tanhl` | portable | math_c_tests.sysl |
| `tanl` | portable | math_c_tests.sysl |
| `tgamma` | portable | math_c_tests.sysl, math_float_cr_tests.sysl, math_gamma_tests.sysl |
| `tgammaf` | portable | math_float_cr_tests.sysl, math_gamma_tests.sysl |
| `tgammal` | portable | math_c_tests.sysl |
| `trunc` | portable | math_c_tests.sysl, math_exact_tests.sysl |
| `truncf` | portable | math_exact_tests.sysl |
| `truncl` | portable | math_c_tests.sysl |
