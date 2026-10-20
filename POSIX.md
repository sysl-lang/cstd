# cstd against POSIX

This compares the functions cstd offers with the functions **POSIX.1-2024** (IEEE Std 1003.1-2024, The Open
Group Base Specifications Issue 8) requires of each header cstd ships in `include/`. It was checked on
2026-10-14 against the standard's pages at <https://pubs.opengroup.org/onlinepubs/9799919799/>: the
`basedefs/<header>.html` page of each offered header, plus the SYNOPSIS of a function's own page where
noted. cstd's side comes from the `@export("…")` attributes under `sh/` and `linux/` and the prototypes in
`include/`.

**The requirement**: every header cstd offers declares and implements every function POSIX requires of that
header.

**What counts as required.** A function with no margin code, and a function marked **[CX]**, are in
scope. [CX] marks an extension of ISO C that is part of the POSIX base, not an option. Everything else is
out of scope: **[OB]** (obsolescent) and every option-group code (**[XSI]**, **[ADV]**, **[SIO]**,
**[FSC]**, **[CPT]**, and any other the pages show). A function with [CX] together with an option code is
out. Out-of-scope functions are listed in a separate table per header so the decision is visible, but the
counts and totals cover in-scope functions only.

**Status** is one of `done (core)` or `done (linux)` (the half that exports it: `sh/sysl/cstd` or
`linux/sh/sysl/cstd_linux`), `missing`, `declared only` (the header declares it, nothing exports it), or
`exported, not declared`. No function is currently in either of the last two states.

**Needs** says what a missing function requires:

| category | meaning |
|---|---|
| `pure` | arithmetic over bytes the caller hands in (including fixed tables and private state) |
| `locale` | a locale-dependent function: the `_l` variants, `strcoll`/`strxfrm`, multibyte and wide conversion; C locale only |
| `heap` | allocates, so needs `malloc` |
| `os` | needs a system call or a hook: files, processes, signals, the environment, time zones, ids |
| `time` | needs a clock |
| `stdio streams` | needs the `FILE` layer |

**Half** is where the function would naturally sit: `core` for `pure` and `locale`; `linux` for `os` and
`time` (and the core only through a hook, see design question 1); `either` where the answer depends on a
design question below.

## `<assert.h>`

The page defines no function: it requires the `assert` macro (a macro, never a function), which is
redefined from the state of `NDEBUG` each time the header is included. cstd's header has no include
guard around it for that reason, supplies `static_assert` as `_Static_assert` before C23, and its
failure path is the exported helper `__cstd_assert_fail` (not a POSIX name), which writes
`Assertion failed: <expression> (<file>: <function>: <line>)` to descriptor 2 and calls `abort`.
`abort` is the platform's (`stdlib.h`'s, supplied by cstd's Linux half, the host libc, or on a board
cstd's weak default, which traps).

| macro | codes | status | needs | half |
|---|---|---|---|---|
| assert | | done (core) | | core |

## `<ctype.h>`

All 28 functions are done in the core. The page says they "shall be declared as functions and may also
be defined as macros"; cstd declares them as functions only, which satisfies it.

| function | codes | status | needs | half |
|---|---|---|---|---|
| isalnum | | done (core) | | core |
| isalpha | | done (core) | | core |
| isblank | | done (core) | | core |
| iscntrl | | done (core) | | core |
| isdigit | | done (core) | | core |
| isgraph | | done (core) | | core |
| islower | | done (core) | | core |
| isprint | | done (core) | | core |
| ispunct | | done (core) | | core |
| isspace | | done (core) | | core |
| isupper | | done (core) | | core |
| isxdigit | | done (core) | | core |
| tolower | | done (core) | | core |
| toupper | | done (core) | | core |
| isalnum_l | CX | done (core) | | core |
| isalpha_l | CX | done (core) | | core |
| isblank_l | CX | done (core) | | core |
| iscntrl_l | CX | done (core) | | core |
| isdigit_l | CX | done (core) | | core |
| isgraph_l | CX | done (core) | | core |
| islower_l | CX | done (core) | | core |
| isprint_l | CX | done (core) | | core |
| ispunct_l | CX | done (core) | | core |
| isspace_l | CX | done (core) | | core |
| isupper_l | CX | done (core) | | core |
| isxdigit_l | CX | done (core) | | core |
| tolower_l | CX | done (core) | | core |
| toupper_l | CX | done (core) | | core |

Out of scope: none. The header defines `locale_t` as `<locale.h>` does, and each `_l` variant answers
as its plain twin whatever locale it is given, since every locale is the C locale.

## `<locale.h>`

The C locale is the only locale. `setlocale` and `newlocale` accept "C", "POSIX" and "" (the native
locale, which is the C locale here) and refuse any other name; `setlocale` answers "C". `localeconv`
answers the C locale's conventions (`decimal_point` ".", every other string "", every `char` member
`CHAR_MAX`). Every locale object is one static object, so nothing allocates and `freelocale` releases
nothing; `LC_GLOBAL_LOCALE` is `(locale_t)-1`. The current locale is one per program, not per thread.
The header also defines `struct lconv`, `LC_ALL`, `LC_COLLATE`, `LC_CTYPE`, `LC_MONETARY`,
`LC_NUMERIC`, `LC_TIME`, `LC_MESSAGES`, the seven `LC_*_MASK` and `NULL`.

| function | codes | status | needs | half |
|---|---|---|---|---|
| duplocale | CX | done (core) | | core |
| freelocale | CX | done (core) | | core |
| getlocalename_l | CX | done (core) | | core |
| localeconv | | done (core) | | core |
| newlocale | CX | done (core) | | core |
| setlocale | | done (core) | | core |
| uselocale | CX | done (core) | | core |

Out of scope: none.

## `<endian.h>`

New in Issue 8. The page has no margin codes. It requires the macros `BYTE_ORDER`, `LITTLE_ENDIAN` and
`BIG_ENDIAN` (distinct values, usable in `#if`; cstd takes them from the compiler's `__BYTE_ORDER__`, so
the header is right on a big-endian target too) and the twelve conversions, which "shall be declared as
functions, or defined as macros, or both"; cstd declares them as functions only.

| function | codes | status | needs | half |
|---|---|---|---|---|
| be16toh | | done (core) | | core |
| be32toh | | done (core) | | core |
| be64toh | | done (core) | | core |
| htobe16 | | done (core) | | core |
| htobe32 | | done (core) | | core |
| htobe64 | | done (core) | | core |
| htole16 | | done (core) | | core |
| htole32 | | done (core) | | core |
| htole64 | | done (core) | | core |
| le16toh | | done (core) | | core |
| le32toh | | done (core) | | core |
| le64toh | | done (core) | | core |

Out of scope: none.

## `<errno.h>`

Declares no functions. See "Declarations that disagree" for `errno` itself.

## `<fcntl.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| creat | | missing | os | linux |
| fcntl | | missing | os | linux |
| open | | done (linux) | | linux |
| openat | | done (linux) | | linux |

Out of scope:

| function | codes | status |
|---|---|---|
| posix_fadvise | ADV | missing |
| posix_fallocate | ADV | missing |

## `<inttypes.h>`

All 6 functions are done in the core. The page also requires `imaxdiv_t`, `wchar_t` (from
`<stddef.h>`), `<stdint.h>`, and a `PRI`/`SCN` format macro for each `<stdint.h>` type and each of
`d i o u x X` (`PRI`) and `d i o u x` (`SCN`) over `N` = 8, 16, 32, 64, `LEASTN`, `FASTN`, `MAX` and
`PTR`: 154 macros, all defined. Each expands to the compiler's own `__INT64_FMTd__`-family string for
the type the compiler's `<stdint.h>` gives that name, so `PRId64` is `"ld"` where `int64_t` is `long`
and `"lld"` where it is `long long`, and `PRIdPTR` follows the pointer width. `strtoimax` and
`strtoumax` are `strtoll` and `strtoull`; `wcstoimax` and `wcstoumax` run the same scan over a
`wchar_t` string (read as 32-bit; in the C locale only ASCII characters are white space or digits).

| function | codes | status | needs | half |
|---|---|---|---|---|
| imaxabs | | done (core) | | core |
| imaxdiv | | done (core) | | core |
| strtoimax | | done (core) | | core |
| strtoumax | | done (core) | | core |
| wcstoimax | | done (core) | | core |
| wcstoumax | | done (core) | | core |

## `<math.h>`

All 57 functions are done in the core in their `double` and `float` forms, 114 functions. The 57 `long
double` (`l`) forms are counted missing: where `long double` is binary128 (AArch64 Linux, which keel's
Linux half targets, and RISC-V) they need quad-precision algorithms that are not written. Where `long
double` is binary64 (arm64 macOS, 32-bit Arm) cstd does export and declare each, as its `double`
counterpart under the `l` name, exactly as accurate; `include/math.h` declares them only under
`__LDBL_MANT_DIG__ == 53`, so a binary128 program calling one fails to compile instead of linking.
`nexttoward` and `nexttowardf` take their long double direction at its own width on both kinds.

The page's macros are all defined: `fpclassify`, `isfinite`, `isinf`, `isnan`, `isnormal`, `signbit`
and the six quiet comparisons as the compiler's generic builtins; `HUGE_VAL`, `HUGE_VALF`,
`HUGE_VALL`, `INFINITY`, `NAN`; `FP_INFINITE`, `FP_NAN`, `FP_NORMAL`, `FP_SUBNORMAL`, `FP_ZERO`,
`FP_ILOGB0` and `FP_ILOGBNAN` (both `INT_MIN`); `MATH_ERRNO`, `MATH_ERREXCEPT`, and `math_errhandling`,
which is `MATH_ERRNO`. `FP_FAST_FMA` is not defined: `fma` is the machine's fused instruction only where the target has
one, and integer arithmetic elsewhere. The types
`float_t` and `double_t` are `float` and `double` (`FLT_EVAL_METHOD` 0 on every target). The [XSI]
constants `M_E` to `M_SQRT1_3` and their `l` forms are defined too, though out of scope.

| function | codes | status | needs | half |
|---|---|---|---|---|
| acos | | done (core) | | core |
| acosf | | done (core) | | core |
| acosl | | missing | pure | core |
| acosh | | done (core) | | core |
| acoshf | | done (core) | | core |
| acoshl | | missing | pure | core |
| asin | | done (core) | | core |
| asinf | | done (core) | | core |
| asinl | | missing | pure | core |
| asinh | | done (core) | | core |
| asinhf | | done (core) | | core |
| asinhl | | missing | pure | core |
| atan | | done (core) | | core |
| atanf | | done (core) | | core |
| atanl | | missing | pure | core |
| atan2 | | done (core) | | core |
| atan2f | | done (core) | | core |
| atan2l | | missing | pure | core |
| atanh | | done (core) | | core |
| atanhf | | done (core) | | core |
| atanhl | | missing | pure | core |
| cbrt | | done (core) | | core |
| cbrtf | | done (core) | | core |
| cbrtl | | missing | pure | core |
| ceil | | done (core) | | core |
| ceilf | | done (core) | | core |
| ceill | | missing | pure | core |
| copysign | | done (core) | | core |
| copysignf | | done (core) | | core |
| copysignl | | missing | pure | core |
| cos | | done (core) | | core |
| cosf | | done (core) | | core |
| cosl | | missing | pure | core |
| cosh | | done (core) | | core |
| coshf | | done (core) | | core |
| coshl | | missing | pure | core |
| erf | | done (core) | | core |
| erff | | done (core) | | core |
| erfl | | missing | pure | core |
| erfc | | done (core) | | core |
| erfcf | | done (core) | | core |
| erfcl | | missing | pure | core |
| exp | | done (core) | | core |
| expf | | done (core) | | core |
| expl | | missing | pure | core |
| exp2 | | done (core) | | core |
| exp2f | | done (core) | | core |
| exp2l | | missing | pure | core |
| expm1 | | done (core) | | core |
| expm1f | | done (core) | | core |
| expm1l | | missing | pure | core |
| fabs | | done (core) | | core |
| fabsf | | done (core) | | core |
| fabsl | | missing | pure | core |
| fdim | | done (core) | | core |
| fdimf | | done (core) | | core |
| fdiml | | missing | pure | core |
| floor | | done (core) | | core |
| floorf | | done (core) | | core |
| floorl | | missing | pure | core |
| fma | | done (core) | | core |
| fmaf | | done (core) | | core |
| fmal | | missing | pure | core |
| fmax | | done (core) | | core |
| fmaxf | | done (core) | | core |
| fmaxl | | missing | pure | core |
| fmin | | done (core) | | core |
| fminf | | done (core) | | core |
| fminl | | missing | pure | core |
| fmod | | done (core) | | core |
| fmodf | | done (core) | | core |
| fmodl | | missing | pure | core |
| frexp | | done (core) | | core |
| frexpf | | done (core) | | core |
| frexpl | | missing | pure | core |
| hypot | | done (core) | | core |
| hypotf | | done (core) | | core |
| hypotl | | missing | pure | core |
| ilogb | | done (core) | | core |
| ilogbf | | done (core) | | core |
| ilogbl | | missing | pure | core |
| ldexp | | done (core) | | core |
| ldexpf | | done (core) | | core |
| ldexpl | | missing | pure | core |
| lgamma | | done (core) | | core |
| lgammaf | | done (core) | | core |
| lgammal | | missing | pure | core |
| llrint | | done (core) | | core |
| llrintf | | done (core) | | core |
| llrintl | | missing | pure | core |
| llround | | done (core) | | core |
| llroundf | | done (core) | | core |
| llroundl | | missing | pure | core |
| log | | done (core) | | core |
| logf | | done (core) | | core |
| logl | | missing | pure | core |
| log10 | | done (core) | | core |
| log10f | | done (core) | | core |
| log10l | | missing | pure | core |
| log1p | | done (core) | | core |
| log1pf | | done (core) | | core |
| log1pl | | missing | pure | core |
| log2 | | done (core) | | core |
| log2f | | done (core) | | core |
| log2l | | missing | pure | core |
| logb | | done (core) | | core |
| logbf | | done (core) | | core |
| logbl | | missing | pure | core |
| lrint | | done (core) | | core |
| lrintf | | done (core) | | core |
| lrintl | | missing | pure | core |
| lround | | done (core) | | core |
| lroundf | | done (core) | | core |
| lroundl | | missing | pure | core |
| modf | | done (core) | | core |
| modff | | done (core) | | core |
| modfl | | missing | pure | core |
| nan | | done (core) | | core |
| nanf | | done (core) | | core |
| nanl | | missing | pure | core |
| nearbyint | | done (core) | | core |
| nearbyintf | | done (core) | | core |
| nearbyintl | | missing | pure | core |
| nextafter | | done (core) | | core |
| nextafterf | | done (core) | | core |
| nextafterl | | missing | pure | core |
| nexttoward | | done (core) | | core |
| nexttowardf | | done (core) | | core |
| nexttowardl | | missing | pure | core |
| pow | | done (core) | | core |
| powf | | done (core) | | core |
| powl | | missing | pure | core |
| remainder | | done (core) | | core |
| remainderf | | done (core) | | core |
| remainderl | | missing | pure | core |
| remquo | | done (core) | | core |
| remquof | | done (core) | | core |
| remquol | | missing | pure | core |
| rint | | done (core) | | core |
| rintf | | done (core) | | core |
| rintl | | missing | pure | core |
| round | | done (core) | | core |
| roundf | | done (core) | | core |
| roundl | | missing | pure | core |
| scalbln | | done (core) | | core |
| scalblnf | | done (core) | | core |
| scalblnl | | missing | pure | core |
| scalbn | | done (core) | | core |
| scalbnf | | done (core) | | core |
| scalbnl | | missing | pure | core |
| sin | | done (core) | | core |
| sinf | | done (core) | | core |
| sinl | | missing | pure | core |
| sinh | | done (core) | | core |
| sinhf | | done (core) | | core |
| sinhl | | missing | pure | core |
| sqrt | | done (core) | | core |
| sqrtf | | done (core) | | core |
| sqrtl | | missing | pure | core |
| tan | | done (core) | | core |
| tanf | | done (core) | | core |
| tanl | | missing | pure | core |
| tanh | | done (core) | | core |
| tanhf | | done (core) | | core |
| tanhl | | missing | pure | core |
| tgamma | | done (core) | | core |
| tgammaf | | done (core) | | core |
| tgammal | | missing | pure | core |
| trunc | | done (core) | | core |
| truncf | | done (core) | | core |
| truncl | | missing | pure | core |

Out of scope:

| function | codes | status |
|---|---|---|
| j0 | XSI | missing |
| j1 | XSI | missing |
| jn | XSI | missing |
| y0 | XSI | missing |
| y1 | XSI | missing |
| yn | XSI | missing |

The [XSI] variable `signgam`, which `lgamma` sets on an XSI system, is not defined.

## `<setjmp.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| longjmp | | done (core) | | core |
| setjmp | | done (core) | | core |
| siglongjmp | CX | done (core) | | core |
| sigsetjmp | CX | done (core) | | core |

Written for aarch64, thumb (saving s16–s31 where the target has them), and riscv32 without F; the header refuses to
compile anywhere else (see the README). `sigsetjmp` with a non-zero `savemask` reads and restores the
mask through an external `sigprocmask`, which the platform supplies as it supplies `write`; cstd's own
`sigprocmask` comes with `<signal.h>`'s functions. `jmp_buf` and `sigjmp_buf` are one type.

`_setjmp` and `_longjmp` are not in POSIX.1-2024: Issue 7 marked them [OB XSI] and Issue 8 removed
them, so there is no out-of-scope table.

## `<signal.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| kill | CX | done (linux) | | linux |
| psiginfo | CX | missing | os | linux |
| psignal | CX | missing | os | linux |
| pthread_kill | CX | missing | os | linux |
| pthread_sigmask | CX | missing | os | linux |
| raise | | missing | os | linux |
| sig2str | CX | missing | pure | core |
| sigaction | CX | missing | os | linux |
| sigaddset | CX | missing | pure | core |
| sigdelset | CX | missing | pure | core |
| sigemptyset | CX | missing | pure | core |
| sigfillset | CX | missing | pure | core |
| sigismember | CX | missing | pure | core |
| signal | | missing | os | linux |
| sigpending | CX | missing | os | linux |
| sigprocmask | CX | missing | os | linux |
| sigqueue | CX | missing | os | linux |
| sigsuspend | CX | missing | os | linux |
| sigtimedwait | CX | missing | os | linux |
| sigwait | CX | missing | os | linux |
| sigwaitinfo | CX | missing | os | linux |
| str2sig | CX | missing | pure | core |

`psignal` and `psiginfo` write to standard error, hence `os`. The set operations are pure only once
`sigset_t` is defined; `signal.h` defines no types today.

Out of scope:

| function | codes | status |
|---|---|---|
| killpg | XSI | missing |
| sigaltstack | XSI | missing |

## `<stdio.h>`

cstd's `stdio.h` has no `FILE`. Its eight printf functions are in the core: `printf` and `vprintf` write to
descriptor 1 and `dprintf`/`vdprintf` to the descriptor given, all through an external `write` symbol that
the Linux half (or a board) supplies.

`asprintf` and `vasprintf` are not in the prototype list of `basedefs/stdio.h.html` as fetched, but
`functions/asprintf.html` and `functions/vfprintf.html` declare them under `#include <stdio.h>` with [CX],
so they are counted here. The page says every function listed "shall be declared as functions and may
also be defined as macros", which covers `getc`, `putc`, `getchar`, `putchar` and the `_unlocked` forms.

| function | codes | status | needs | half |
|---|---|---|---|---|
| asprintf | CX | missing | heap | either |
| clearerr | | missing | stdio streams | either |
| ctermid | CX | missing | pure | core |
| dprintf | CX | done (core) | | core |
| fclose | | missing | stdio streams | either |
| fdopen | CX | missing | stdio streams | either |
| feof | | missing | stdio streams | either |
| ferror | | missing | stdio streams | either |
| fflush | | missing | stdio streams | either |
| fgetc | | missing | stdio streams | either |
| fgetpos | | missing | stdio streams | either |
| fgets | | missing | stdio streams | either |
| fileno | CX | missing | stdio streams | either |
| flockfile | CX | missing | stdio streams | either |
| fmemopen | CX | missing | stdio streams | either |
| fopen | | missing | stdio streams | either |
| fprintf | | missing | stdio streams | either |
| fputc | | missing | stdio streams | either |
| fputs | | missing | stdio streams | either |
| fread | | missing | stdio streams | either |
| freopen | | missing | stdio streams | either |
| fscanf | | missing | stdio streams | either |
| fseek | | missing | stdio streams | either |
| fseeko | CX | missing | stdio streams | either |
| fsetpos | | missing | stdio streams | either |
| ftell | | missing | stdio streams | either |
| ftello | CX | missing | stdio streams | either |
| ftrylockfile | CX | missing | stdio streams | either |
| funlockfile | CX | missing | stdio streams | either |
| fwrite | | missing | stdio streams | either |
| getc | | missing | stdio streams | either |
| getc_unlocked | CX | missing | stdio streams | either |
| getchar | | missing | stdio streams | either |
| getchar_unlocked | CX | missing | stdio streams | either |
| getdelim | CX | missing | stdio streams | either |
| getline | CX | missing | stdio streams | either |
| open_memstream | CX | missing | stdio streams | either |
| pclose | CX | missing | stdio streams | either |
| perror | | missing | stdio streams | either |
| popen | CX | missing | stdio streams | either |
| printf | | done (core) | | core |
| putc | | missing | stdio streams | either |
| putc_unlocked | CX | missing | stdio streams | either |
| putchar | | missing | stdio streams | either |
| putchar_unlocked | CX | missing | stdio streams | either |
| puts | | missing | stdio streams | either |
| remove | | missing | os | linux |
| rename | | missing | os | linux |
| renameat | CX | missing | os | linux |
| rewind | | missing | stdio streams | either |
| scanf | | missing | stdio streams | either |
| setbuf | | missing | stdio streams | either |
| setvbuf | | missing | stdio streams | either |
| snprintf | | done (core) | | core |
| sprintf | | done (core) | | core |
| sscanf | | missing | pure | core |
| tmpfile | | missing | stdio streams | either |
| ungetc | | missing | stdio streams | either |
| vasprintf | CX | missing | heap | either |
| vdprintf | CX | done (core) | | core |
| vfprintf | | missing | stdio streams | either |
| vfscanf | | missing | stdio streams | either |
| vprintf | | done (core) | | core |
| vscanf | | missing | stdio streams | either |
| vsnprintf | | done (core) | | core |
| vsprintf | | done (core) | | core |
| vsscanf | | missing | pure | core |

`popen`, `pclose`, `tmpfile` and `fopen` also need `os`; they are counted under `stdio streams`.

Out of scope:

| function | codes | status |
|---|---|---|
| tmpnam | OB | missing |

## `<stdlib.h>`

`malloc`, `calloc`, `realloc`, `free`, `atexit`, `exit`, `_Exit` and `abort` are the Linux half's; on
any other bare-metal target the core gives each a weak default a board replaces (`malloc.sysl`,
`exit.sysl`): the allocator over `sbrk` (`BreakHeap`, the same code the Linux half exports strongly over its own `sbrk`), `exit` running the `atexit` table (one table, the core's, for
both halves) and then `_Exit`, and `_Exit` and `abort` trapping.

`strtold` is counted missing for the reason `<math.h>`'s `l` forms are: where `long double` is
binary64 (arm64 macOS, 32-bit Arm) cstd exports and declares it as `strtod` under its own name, and
where it is binary128 it is not offered. The multibyte functions are the C locale's, the only one
cstd has: one byte per character from 0x00 to 0x7F, `MB_CUR_MAX` 1, and `EILSEQ` from 0x80 up.

| function | codes | status | needs | half |
|---|---|---|---|---|
| _Exit | | done (linux) | | linux |
| abort | | done (linux) | | linux |
| abs | | done (core) | | core |
| aligned_alloc | | missing | heap | either |
| at_quick_exit | | missing | os | linux |
| atexit | | done (linux) | | linux |
| atof | | done (core) | | core |
| atoi | | done (core) | | core |
| atol | | done (core) | | core |
| atoll | | done (core) | | core |
| bsearch | | done (core) | | core |
| calloc | | done (linux) | | linux |
| div | | done (core) | | core |
| exit | | done (linux) | | linux |
| free | | done (linux) | | linux |
| getenv | | missing | os | linux |
| getsubopt | CX | missing | pure | core |
| labs | | done (core) | | core |
| ldiv | | done (core) | | core |
| llabs | | done (core) | | core |
| lldiv | | done (core) | | core |
| malloc | | done (linux) | | linux |
| mblen | | done (core) | | core |
| mbstowcs | | done (core) | | core |
| mbtowc | | done (core) | | core |
| mkdtemp | CX | missing | os | linux |
| mkostemp | CX | missing | os | linux |
| mkstemp | CX | missing | os | linux |
| qsort | | done (core) | | core |
| qsort_r | CX | missing | pure | core |
| quick_exit | | missing | os | linux |
| rand | | done (core) | | core |
| realloc | | done (linux) | | linux |
| reallocarray | CX | missing | heap | either |
| realpath | | missing | os | linux |
| secure_getenv | CX | missing | os | linux |
| setenv | CX | missing | os | linux |
| srand | | done (core) | | core |
| strtod | | done (core) | | core |
| strtof | | done (core) | | core |
| strtol | | done (core) | | core |
| strtold | | missing | pure | core |
| strtoll | | done (core) | | core |
| strtoul | | done (core) | | core |
| strtoull | | done (core) | | core |
| system | | missing | os | linux |
| unsetenv | CX | missing | os | linux |
| wcstombs | | done (core) | | core |
| wctomb | | done (core) | | core |

`getenv` and the other environment functions read `environ`, which the Linux half's `crt0` sets, hence
`os`; `setenv` also needs `heap`. `realpath` also allocates when handed a null buffer.

Out of scope:

| function | codes | status |
|---|---|---|
| a64l | XSI | missing |
| drand48 | XSI | missing |
| erand48 | XSI | missing |
| grantpt | XSI | missing |
| initstate | XSI | missing |
| jrand48 | XSI | missing |
| l64a | XSI | missing |
| lcong48 | XSI | missing |
| lrand48 | XSI | missing |
| mrand48 | XSI | missing |
| nrand48 | XSI | missing |
| posix_memalign | ADV | missing |
| posix_openpt | XSI | missing |
| ptsname | XSI | missing |
| ptsname_r | XSI | missing |
| putenv | XSI | missing |
| random | XSI | missing |
| seed48 | XSI | missing |
| setkey | OB XSI | missing |
| setstate | XSI | missing |
| srand48 | XSI | missing |
| srandom | XSI | missing |
| unlockpt | XSI | missing |

## `<string.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| memchr | | done (core) | | core |
| memcmp | | done (core) | | core |
| memcpy | | done (core) | | core |
| memmem | CX | done (core) | | core |
| memmove | | done (core) | | core |
| memset | | done (core) | | core |
| stpcpy | CX | done (core) | | core |
| stpncpy | CX | done (core) | | core |
| strcat | | done (core) | | core |
| strchr | | done (core) | | core |
| strcmp | | done (core) | | core |
| strcoll | | done (core) | | core |
| strcoll_l | CX | done (core) | | core |
| strcpy | | done (core) | | core |
| strcspn | | done (core) | | core |
| strdup | CX | done (core) | | core |
| strerror | | done (core) | | core |
| strerror_l | CX | done (core) | | core |
| strerror_r | CX | done (core) | | core |
| strlcat | CX | done (core) | | core |
| strlcpy | CX | done (core) | | core |
| strlen | | done (core) | | core |
| strncat | | done (core) | | core |
| strncmp | | done (core) | | core |
| strncpy | | done (core) | | core |
| strndup | CX | done (core) | | core |
| strnlen | CX | done (core) | | core |
| strpbrk | | done (core) | | core |
| strrchr | | done (core) | | core |
| strsignal | CX | done (core) | | core |
| strspn | | done (core) | | core |
| strstr | | done (core) | | core |
| strtok | | done (core) | | core |
| strtok_r | CX | done (core) | | core |
| strxfrm | | done (core) | | core |
| strxfrm_l | CX | done (core) | | core |

`strdup` and `strndup` call an external `malloc` the program supplies (the Linux half's, a board's,
or the host's). `strcoll`, `strxfrm` and the `_l` forms collate as the C locale does, by byte;
`locale_t` is an opaque pointer, defined under the guard `__CSTD_LOCALE_T` so `locale.h` agrees.

Out of scope:

| function | codes | status |
|---|---|---|
| memccpy | XSI | missing |

## `<strings.h>`

All 4 in-scope functions are done in the core. Only the C locale exists, so the `_l` forms ignore their
`locale_t` and case folding is ASCII. `locale_t` is declared in the header with the guard shared by the
other headers that need it.

| function | codes | status | needs | half |
|---|---|---|---|---|
| strcasecmp | | done (core) | | core |
| strcasecmp_l | | done (core) | | core |
| strncasecmp | | done (core) | | core |
| strncasecmp_l | | done (core) | | core |

Out of scope (offered anyway: the header declares them and the core implements them):

| function | codes | status |
|---|---|---|
| ffs | XSI | done (core) |
| ffsl | XSI | done (core) |
| ffsll | XSI | done (core) |

## `<sys/stat.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| chmod | | missing | os | linux |
| fchmod | | missing | os | linux |
| fchmodat | | missing | os | linux |
| fstat | | done (linux) | | linux |
| fstatat | | missing | os | linux |
| futimens | | missing | os | linux |
| lstat | | missing | os | linux |
| mkdir | | done (linux) | | linux |
| mkdirat | | missing | os | linux |
| mkfifo | | missing | os | linux |
| mkfifoat | | missing | os | linux |
| stat | | done (linux) | | linux |
| umask | | missing | os | linux |
| utimensat | | missing | os | linux |

Function-like macros the page requires: `S_ISBLK`, `S_ISCHR`, `S_ISDIR`, `S_ISFIFO`, `S_ISREG`, `S_ISLNK`,
`S_ISSOCK`, `S_TYPEISMQ`, `S_TYPEISSEM`, `S_TYPEISSHM` (and `S_TYPEISTMO`, [TYM], out of scope). cstd
defines `S_ISDIR`, `S_ISCHR`, `S_ISREG`, `S_ISFIFO`, `S_ISLNK`; `S_ISBLK`, `S_ISSOCK` and the four
`S_TYPEIS*` are missing.

Out of scope:

| function | codes | status |
|---|---|---|
| mknod | XSI | missing |
| mknodat | XSI | missing |

## `<sys/types.h>`

Declares no functions. Of the types the page requires, cstd defines `ssize_t`, `off_t`, `pid_t`, `mode_t`,
`uid_t`, `gid_t`, `dev_t`, `ino_t`, `nlink_t`, `blksize_t`, `blkcnt_t`, `time_t`, `clockid_t` (and
`size_t` through `stddef.h`). Missing: `clock_t`, `fsblkcnt_t`, `fsfilcnt_t`, `id_t`, `reclen_t`,
`timer_t`, and the `pthread_*` types (`key_t` and `suseconds_t` are [XSI], out of scope).

## `<time.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| clock | | done (core) | | core |
| clock_getres | CX | done (linux) | | linux |
| clock_gettime | CX | done (linux) | | linux |
| clock_nanosleep | CX | done (linux) | | linux |
| clock_settime | CX | done (linux) | | linux |
| difftime | | done (core) | | core |
| gmtime | | done (core) | | core |
| gmtime_r | CX | done (core) | | core |
| localtime | | done (core) | | core |
| localtime_r | CX | done (core) | | core |
| mktime | | done (core) | | core |
| nanosleep | CX | done (linux) | | linux |
| strftime | | done (core) | | core |
| strftime_l | CX | done (core) | | core |
| time | | done (core) | | core |
| timer_create | CX | done (linux) | | linux |
| timer_delete | CX | done (linux) | | linux |
| timer_getoverrun | CX | done (linux) | | linux |
| timer_gettime | CX | done (linux) | | linux |
| timer_settime | CX | done (linux) | | linux |
| timespec_get | | done (core) | | core |
| tzset | CX | done (core) | | core |
| tzname (variable) | CX | done (core) | | core |

There is no time-zone database, so local time is UTC: `localtime`, `localtime_r`, `mktime` and `tzset`
need no `TZ` and sit in the core (`time.sysl`). `time`, `clock` and `timespec_get` are in the core too,
built on `clock_gettime` and compiled only for a freestanding target (a hosted program's are its libc's);
on a board without a clock they fail as `clock_gettime`'s weak default does. Every `linux` row above has a
weak `ENOSYS` default on a board (`stubs.sysl`). The header defines `clock_t`, `struct tm` (with
POSIX.1-2024's `tm_gmtoff` and `tm_zone`), `struct timespec`, `struct itimerspec`, `timer_t` (the kernel's
`int`), `locale_t` under the shared guard, `CLOCKS_PER_SEC`, `TIME_UTC`, `TIMER_ABSTIME` and the `CLOCK_`
names, and declares `timegm`, which POSIX.1-2024 does not list (ISO C23 does; minmea calls it).

`tzname` names UTC twice. `struct sigevent` is declared incomplete, as `<signal.h>` does not define it yet;
`timer_create` hands it to the kernel unread. `daylight` and `timezone` are [XSI].

Out of scope:

| function | codes | status |
|---|---|---|
| asctime | OB | missing |
| clock_getcpuclockid | CPT | missing |
| ctime | OB | missing |
| getdate | XSI | missing |
| strptime | XSI | missing |

## `<unistd.h>`

| function | codes | status | needs | half |
|---|---|---|---|---|
| _exit | | done (linux) | | linux |
| _Fork | | missing | os | linux |
| access | | missing | os | linux |
| alarm | | missing | os | linux |
| chdir | | done (linux) | | linux |
| chown | | missing | os | linux |
| close | | done (linux) | | linux |
| confstr | | missing | pure | core |
| dup | | done (linux) | | linux |
| dup2 | | done (linux) | | linux |
| dup3 | | missing | os | linux |
| execl | | missing | os | linux |
| execle | | missing | os | linux |
| execlp | | missing | os | linux |
| execv | | missing | os | linux |
| execve | | missing | os | linux |
| execvp | | missing | os | linux |
| faccessat | | missing | os | linux |
| fchdir | | missing | os | linux |
| fchown | | missing | os | linux |
| fchownat | | missing | os | linux |
| fexecve | | missing | os | linux |
| fork | | missing | os | linux |
| fpathconf | | missing | os | linux |
| ftruncate | | missing | os | linux |
| getcwd | | done (linux) | | linux |
| getegid | | missing | os | linux |
| getentropy | | missing | os | linux |
| geteuid | | missing | os | linux |
| getgid | | missing | os | linux |
| getgroups | | missing | os | linux |
| gethostname | | missing | os | linux |
| getlogin | | missing | os | linux |
| getlogin_r | | missing | os | linux |
| getopt | | missing | pure | core |
| getpgid | | missing | os | linux |
| getpgrp | | missing | os | linux |
| getpid | | done (linux) | | linux |
| getppid | | done (linux) | | linux |
| getsid | | missing | os | linux |
| getuid | | missing | os | linux |
| isatty | | missing | os | linux |
| lchown | | missing | os | linux |
| link | | missing | os | linux |
| linkat | | missing | os | linux |
| lseek | | done (linux) | | linux |
| pathconf | | missing | os | linux |
| pause | | missing | os | linux |
| pipe | | done (linux) | | linux |
| pipe2 | | missing | os | linux |
| posix_close | | missing | os | linux |
| pread | | missing | os | linux |
| pwrite | | missing | os | linux |
| read | | done (linux) | | linux |
| readlink | | missing | os | linux |
| readlinkat | | missing | os | linux |
| rmdir | | done (linux) | | linux |
| setegid | | missing | os | linux |
| seteuid | | missing | os | linux |
| setgid | | missing | os | linux |
| setpgid | | missing | os | linux |
| setsid | | missing | os | linux |
| setuid | | missing | os | linux |
| sleep | | missing | time | linux |
| symlink | | missing | os | linux |
| symlinkat | | missing | os | linux |
| sysconf | | missing | os | linux |
| tcgetpgrp | | missing | os | linux |
| tcsetpgrp | | missing | os | linux |
| truncate | | missing | os | linux |
| ttyname | | missing | os | linux |
| ttyname_r | | missing | os | linux |
| unlink | | done (linux) | | linux |
| unlinkat | | missing | os | linux |
| write | | done (linux) | | linux |

The page also declares the variables `optarg`, `opterr`, `optind`, `optopt` (for `getopt`), which cstd
lacks, and the constants `F_OK`/`R_OK`/`W_OK`/`X_OK`, `SEEK_HOLE`/`SEEK_DATA` and `POSIX_CLOSE_RESTART`.

Out of scope:

| function | codes | status |
|---|---|---|
| crypt | XSI | missing |
| encrypt | OB XSI | missing |
| fdatasync | SIO | missing |
| fsync | FSC | missing |
| gethostid | XSI | missing |
| getresgid | XSI | missing |
| getresuid | XSI | missing |
| lockf | XSI | missing |
| nice | XSI | missing |
| setregid | XSI | missing |
| setresgid | XSI | missing |
| setresuid | XSI | missing |
| setreuid | XSI | missing |
| swab | XSI | missing |
| sync | XSI | missing |

## Summary

In-scope functions only.

| header | required | done | missing | pure | locale | heap | os | time | stdio streams |
|---|---|---|---|---|---|---|---|---|---|
| `assert.h` | 0 | 0 | 0 | | | | | | |
| `ctype.h` | 28 | 28 | 0 | | | | | | |
| `endian.h` | 12 | 12 | 0 | | | | | | |
| `errno.h` | 0 | 0 | 0 | | | | | | |
| `fcntl.h` | 4 | 2 | 2 | | | | 2 | | |
| `inttypes.h` | 6 | 6 | 0 | | | | | | |
| `locale.h` | 7 | 7 | 0 | | | | | | |
| `math.h` | 171 | 114 | 57 | 57 | | | | | |
| `setjmp.h` | 4 | 4 | 0 | | | | | | |
| `signal.h` | 22 | 1 | 21 | 7 | | | 14 | | |
| `stdio.h` | 67 | 8 | 59 | 3 | | 2 | 3 | | 51 |
| `stdlib.h` | 49 | 33 | 16 | 3 | | 2 | 11 | | |
| `string.h` | 36 | 36 | 0 | | | | | | |
| `strings.h` | 4 | 4 | 0 | | | | | | |
| `sys/stat.h` | 14 | 3 | 11 | | | | 11 | | |
| `sys/types.h` | 0 | 0 | 0 | | | | | | |
| `time.h` | 22 | 22 | 0 | | | | | | |
| `unistd.h` | 75 | 14 | 61 | 2 | | | 58 | 1 | |
| **total** | **521** | **294** | **227** | **72** | **0** | **4** | **99** | **1** | **51** |

Out of scope and missing: 57 functions (`fcntl.h` 2, `math.h` 6, `signal.h` 2, `stdio.h` 1,
`stdlib.h` 23, `string.h` 1, `sys/stat.h` 2, `time.h` 5, `unistd.h` 15). Out of scope but done: 3
(`strings.h` `ffs`, `ffsl`, `ffsll`).

## Other headers and functions

**Headers cstd's own headers include.** `<stddef.h>` (from `stdlib.h`, `unistd.h`, `stdio.h`,
`string.h`, `sys/types.h`) and `<stdarg.h>` (from `stdio.h`) are clang's freestanding headers, not
cstd's. `<sys/types.h>` is included by `time.h`, `fcntl.h`, `signal.h`, `unistd.h` and `sys/stat.h`;
`<time.h>` by `sys/stat.h`. Both are offered.

**Headers the standard's pages make the offered ones depend on.** `ctype.h`, `string.h`, `strings.h` and
`time.h` must define `locale_t` "as described in `<locale.h>`"; all five define it, each under the guard
`__CSTD_LOCALE_T`. `time.h` declares `struct sigevent` as defined in `<signal.h>` (cstd's declares it
incomplete), whose types (`sigset_t`, `siginfo_t`, `struct sigaction`,
`union sigval`, `stack_t`, `pthread_t`) cstd's `signal.h` does not define.

**Functions present under headers cstd does not offer.** None from POSIX. Three exported and declared
names are not POSIX functions at all: `brk` and `sbrk` (declared in `unistd.h`, exported by the Linux
half's `malloc.sysl`), and `timegm` (ISO C23's, declared in `time.h`, exported by the core's
`time.sysl`). `__cstd_assert_fail` is `assert`'s failure path, and `cstd_sigsetjmp_tail` `sigsetjmp`'s ending. `cstd_start` is the Linux half's internal entry; `errno` and `environ` are
variables.

**Headers in `include/` that POSIX does not define.** One: `stdbit.h`, which is ISO C23 (section 7.18),
not POSIX. POSIX.1-2024 is aligned with C17 and does not list it. It is held to the same rule as the
rest, that a header offered is complete: it declares and implements all 14 families in each of the five
unsigned types (70 functions), and the 14 type-generic macros, `__STDC_VERSION_STDBIT_H__`,
`__STDC_ENDIAN_LITTLE__`, `__STDC_ENDIAN_BIG__` and `__STDC_ENDIAN_NATIVE__`. These 70 are not counted in
the Summary, which covers POSIX alone. The other eighteen headers are POSIX headers.

## Declarations that disagree

- **`errno`** has thread-local storage duration only where the build has thread-local storage: on
  a board it is one object. It is POSIX's macro, `(*__errno_location())`, everywhere.
- **`brk`, `sbrk`** in `unistd.h` are not in POSIX.1-2024's `<unistd.h>`.
- Every other declared prototype matches the standard's (parameter names aside; cstd writes
  `__restrict` for `restrict` so the headers also read as C++, which is equivalent).

## Design questions

1. How the core half answers an `os` or `time` function on a bare board: a hook the board exports (as
   sysl's own library does, and as the core's `printf` already does by calling an external `write`), or
   leaving it out of the core's header so a board build fails at compile time rather than at link time.
2. Where `heap` functions (`strdup`, `strndup`, `asprintf`, `vasprintf`, `aligned_alloc`, `reallocarray`)
   live: in the core, calling an external `malloc` that the Linux half or a board supplies, or only in
   the Linux half.
3. Locale with only the C locale: settled. `locale_t` is an opaque pointer, `<locale.h>` is offered,
   and the `_l` variants answer as the C locale whatever they are given.
4. The `FILE` layer: its buffering, where `stdin`/`stdout`/`stderr` come from on a board, whether
   `printf`/`vprintf` become `fprintf(stdout, …)` while `dprintf` stays over `write`, and whether the
   memory-only parts (`sscanf`, `fmemopen`, `open_memstream`) sit in the core.
5. `errno` as POSIX's macro over a thread-local lvalue: settled. The macro is `(*__errno_location())`,
   as in glibc and musl, and the storage is `@thread_local` under sysl's `#if thread_local`, a plain
   object on a board.
6. Process, signal, timer, id and terminal functions: which of them keel's system calls can answer,
   and what the Linux half does for the ones it cannot (fail with `ENOSYS`, or leave them out).
7. `brk` and `sbrk`: whether `unistd.h` keeps declaring these non-POSIX names or they move to a header
   of cstd's own.
