# cstd against POSIX

This compares cstd with **POSIX.1-2024** (IEEE Std 1003.1-2024, The Open Group Base Specifications
Issue 8), every header of it, and marks what **ISO C17** requires among it, since a freestanding C
library owes C first. The standard's side is taken from the `basedefs/<header>.html` page of each of
the 86 headers its header index lists, at <https://pubs.opengroup.org/onlinepubs/9799919799/>, checked
on 2026-11-26: every prototype in each page's normative DESCRIPTION, with the margin code it sits
under, and each name checked against the standard's function index. cstd's side is v0.1.9 (`717d86b`),
read off what the archives define (`nm` of `sysl build-c .` for the host and `thumbv7em-freestanding`,
and of `sysl build-c linux --target aarch64-freestanding`) and what `include/` declares, not off the
README.

**What counts as required.** A function with no margin code, and a function marked **[CX]**, are
required: [CX] marks an extension of ISO C that is part of the POSIX base. Every option group
(**[XSI]**, **[ADV]**, **[TPS]**, **[SIO]**, **[FSC]**, **[CPT]** and the rest) and **[OB]** are
optional and are listed in each table's last row without being counted, except the three [OB]
functions C17 still requires (`asctime`, `ctime`, `tmpnam`), which are counted. "Functions" includes
the function-like macros a page lists among its prototypes (`fpclassify`, `FD_SET`, `CMPLX`).

**The rule** (ROADMAP.md): a header cstd offers carries every function POSIX requires of it. Five of the
shipped headers break it, `<signal.h>`, `<stdio.h>`, `<stdlib.h>`, `<sys/stat.h>` and `<unistd.h>`, and
`<errno.h>`, `<fcntl.h>` and `<sys/types.h>` lack macros or types their pages require.

**C-required.** A function a page lists with no margin code, in one of C17's 29 headers, is ISO C's.
Those names are **bold** in the tables. C17 makes three of its headers optional (`<complex.h>`,
`<threads.h>`, `<stdatomic.h>`, through `__STDC_NO_COMPLEX__`, `__STDC_NO_THREADS__` and
`__STDC_NO_ATOMICS__`); POSIX.1-2024 requires all three.

**Status** is one of:

| status | meaning |
|---|---|
| tested (`file`) | exported, declared, and called by the named test (a `*_tests.sysl` file, or the C program under `tests/` its test file runs) |
| present, untested | exported and declared, called by no test |
| present, disagrees | exported and declared, and different from the standard in signature, `errno` or answer (*Declarations that disagree*) |
| missing (a) | implementable in sysl now, in the core |
| missing (b) | needs an operating system or a board: a hook |
| missing (c) | blocked by a gap in sysl (Critical Rule 9) |
| absent by design | missing for a reason the README gives |
| the compiler's | supplied by clang's own freestanding header, not by cstd |

**The hook** for (b) is the shape cstd already uses for every operating-system call: a weak default in
the core (`stubs.sysl`) that sets `errno` to `ENOSYS` and answers the failure value, which the Linux
half replaces with a strong definition of one `svc #0`, and a board with a definition of its own, in
sysl (`@export("write")`) or in C. A size is small (under an hour), medium, or large.

**Scope.** cstd runs where there is no operating system (a board) or keel's (the Linux half). Each
header says which of its parts mean something on a board and which need an OS. The README rules out
locales other than C, and `long double` where it is binary128; it rules out no operating-system
function, so those are counted missing (b), not absent by design (design question 1).

**No gap in sysl blocks any missing function** on cstd's targets (aarch64, thumb, riscv32). Each shape
the missing functions need that a reader might doubt was probed with one program and one build: a
`long double` (`f128` through `c type`) taken, answered and read by `va_arg` in a variadic export, built
for `aarch64-freestanding` and `riscv32-freestanding`; a `double _Complex` and a `float _Complex` taken
and answered as a struct of two floats, called from C on the host with the right parts and built for
aarch64, riscv32 and `thumb-freestanding-softfp`; and `SIG_IGN`, the function pointer 1, told from
`SIG_DFL` by an export comparing a `*extern(i32)` with `ptr_cast(1usize)`. The FFI reference names the
fourth, `FILE *stdout = &files[1]`, as constant data an export may hold. The one refusal met is x86-64's
80-bit `long double` (*"the x87 extended format, which sysl has no type for"*), which the FFI reference
documents and which is no cstd target.

## `<aio.h>`

Asynchronous I/O. **Needs an OS**: a board has no request queue, and keel would answer it with a worker thread or a kernel queue. (b) through the hook, after `<pthread.h>`.

| status | functions |
|---|---|
| missing (b), medium | aio_cancel aio_error aio_read aio_return aio_suspend aio_write lio_listio |
| optional, not counted | missing: aio_fsync [FSC] |

## `<arpa/inet.h>`

Byte order and address text. **Meaningful on a board**: `htonl` and its three siblings are `<endian.h>`'s conversions under other names, and `inet_pton`/`inet_ntop` are text over bytes. lwIP carries its own (`lwip_htonl`, `ip4addr_aton`), so no board package waits on these. The header also owes `in_port_t`, `in_addr_t`, `struct in_addr` and `INET_ADDRSTRLEN`/`INET6_ADDRSTRLEN` from `<netinet/in.h>`. Not shipped.

| status | functions |
|---|---|
| missing (a), small | htonl htons inet_ntop inet_pton ntohl ntohs |
| optional, not counted | missing: inet_addr [OB] inet_ntoa [OB] |

## `<assert.h>`

Shipped, complete. The page defines no function: it requires the `assert` macro, redefined from the state of `NDEBUG` each time the header is included, which is why cstd's header has no include guard around it. `static_assert` is `_Static_assert` before C23. The failure path is the exported helper `__cstd_assert_fail` (not a POSIX name), which writes `Assertion failed: <expression> (<file>: <function>: <line>)` to descriptor 2 and calls `abort`; `assert_tests.sysl` drives both `NDEBUG` states and the message.

## `<complex.h>`

**Meaningful on a board**, and C17 makes it optional while POSIX requires all of it. Not shipped, and clang ships no `complex.h`. A `double _Complex` crosses the C boundary as a struct of two `double`s does (probed, above), so the 69 are plain sysl over the existing `<math.h>`. musl's `src/complex` is the port to follow, as `<math.h>` followed musl's `src/math`; `clog`, `cpow` and the inverse hyperbolics are where the care goes. (a), large.

| status | functions |
|---|---|
| missing (a), large | **CMPLX** **CMPLXF** **CMPLXL** **cabs** **cabsf** **cabsl** **cacos** **cacosf** **cacosh** **cacoshf** **cacoshl** **cacosl** **carg** **cargf** **cargl** **casin** **casinf** **casinh** **casinhf** **casinhl** **casinl** **catan** **catanf** **catanh** **catanhf** **catanhl** **catanl** **ccos** **ccosf** **ccosh** **ccoshf** **ccoshl** **ccosl** **cexp** **cexpf** **cexpl** **cimag** **cimagf** **cimagl** **clog** **clogf** **clogl** **conj** **conjf** **conjl** **cpow** **cpowf** **cpowl** **cproj** **cprojf** **cprojl** **creal** **crealf** **creall** **csin** **csinf** **csinh** **csinhf** **csinhl** **csinl** **csqrt** **csqrtf** **csqrtl** **ctan** **ctanf** **ctanh** **ctanhf** **ctanhl** **ctanl** |

## `<cpio.h>`

Constants only (`C_IRUSR`, `MAGIC` and the rest). **Meaningful anywhere**; not shipped. (a), small: one header, no code.

## `<ctype.h>`

Shipped, complete. All 28 functions are done in the core and declared as functions only, which the page allows (*shall be declared as functions and may also be defined as macros*). The header defines `locale_t` as `<locale.h>` does, and each `_l` variant answers as its plain twin whatever locale it is given, since every locale is the C locale.

| status | functions |
|---|---|
| tested (`ctype_tests.sysl`) | **isalnum** **isalpha** **isblank** **iscntrl** **isdigit** **isgraph** **islower** **isprint** **ispunct** **isspace** **isupper** **isxdigit** **tolower** **toupper** |
| tested (`locale_tests.sysl`) | isalnum_l isalpha_l isblank_l iscntrl_l isdigit_l isgraph_l islower_l isprint_l ispunct_l isspace_l isupper_l isxdigit_l tolower_l toupper_l |

## `<devctl.h>`

`posix_devctl`, new in Issue 8. **Needs an OS**: it is `ioctl` with a size. (b) through the hook, small, once `<termios.h>`'s `ioctl` path exists.

| status | functions |
|---|---|
| missing (b), small | posix_devctl |

## `<dirent.h>`

Directory streams. **Needs an OS or a file system**: on keel, `getdents64` in the Linux half; on a board, littlefs and FatFs each have their own directory API, and a board that wants `opendir` writes it over one of them. (b) through the hook (`posix_getdents` is the one hook; `opendir`, `readdir`, `scandir` and `alphasort` are sysl over it and `malloc`), medium.

| status | functions |
|---|---|
| missing (b), medium | alphasort closedir dirfd fdopendir opendir posix_getdents readdir rewinddir scandir |
| optional, not counted | missing: readdir_r [OB] seekdir [XSI] telldir [XSI] |

## `<dlfcn.h>`

**Needs a loader.** A board image is one static link; keel has no dynamic linker. Every function here is (b), large, and on a board the only honest answer is `dlopen` failing with `dlerror` saying why. See design question 1.

| status | functions |
|---|---|
| missing (b), large | dladdr dlclose dlerror dlopen dlsym |

## `<endian.h>`

Shipped, complete. New in Issue 8, no margin codes. The macros `BYTE_ORDER`, `LITTLE_ENDIAN` and `BIG_ENDIAN` come from the compiler's `__BYTE_ORDER__`, so the header is right on a big-endian target too, and the twelve conversions are declared as functions only, which the page allows.

| status | functions |
|---|---|
| tested (`endian_tests.sysl`) | be16toh be32toh be64toh htobe16 htobe32 htobe64 htole16 htole32 htole64 le16toh le32toh le64toh |

## `<errno.h>`

Shipped. The page declares no function. `errno` is POSIX's macro over `__errno_location()`, one per thread where the build has thread-local storage and one object on a board (`errno_tests.sysl`). The numbers are Linux's, since the Linux half's system calls answer them. **Thirty of the page's codes are not defined**: `EADDRINUSE` `EADDRNOTAVAIL` `EALREADY` `EBADMSG` `ECANCELED` `ECONNABORTED` `ECONNREFUSED` `ECONNRESET` `EDQUOT` `EHOSTUNREACH` `EIDRM` `EINPROGRESS` `EISCONN` `EMSGSIZE` `EMULTIHOP` `ENETDOWN` `ENETRESET` `ENETUNREACH` `ENOBUFS` `ENOLINK` `ENOMSG` `ENOPROTOOPT` `ENOTCONN` `ENOTRECOVERABLE` `EOWNERDEAD` `EPROTO` `EPROTONOSUPPORT` `EPROTOTYPE` `ESOCKTNOSUPPORT` `ESTALE`. Each is one `#define` at Linux's value and one line of `strerror`'s table in glibc's words: (a), small. The C-required three, `EDOM`, `EILSEQ` and `ERANGE`, are there.

## `<fcntl.h>`

Shipped. The four functions are the Linux half's, with a weak `ENOSYS` default on a board. **The page's macros are not all defined**: `AT_EACCESS`, `AT_SYMLINK_FOLLOW`, `AT_SYMLINK_NOFOLLOW`, `FD_CLOFORK`, `F_DUPFD_CLOFORK`, `F_GETOWN`, `F_SETOWN`, `F_GETOWN_EX`, `F_SETOWN_EX`, `F_OWNER_PID`, `F_OWNER_PGRP`, `F_OFD_GETLK`, `F_OFD_SETLK`, `F_OFD_SETLKW`, `O_CLOFORK`, `O_DSYNC`, `O_EXEC`, `O_RSYNC`, `O_SEARCH`, `O_SYNC` and `O_TTY_INIT` are absent, and so is `struct f_owner_ex`. Each that Linux has is a `#define` at arm64's value; `O_CLOFORK`, `FD_CLOFORK` and `F_DUPFD_CLOFORK` are new in Issue 8 and Linux has none, so the Linux half would refuse them with `EINVAL`. (a), small. `fcntl` reads its third argument as an `int` for every command; `F_GETOWN_EX` and `F_SETOWN_EX` would need it read as a pointer.

| status | functions |
|---|---|
| tested (`tests/linux/fcntl.c`) | creat fcntl open |
| tested (`tests/linux/files.c`) | openat |
| optional, not counted | missing: posix_fadvise [ADV] posix_fallocate [ADV] |

## `<fenv.h>`

The floating-point environment. **Meaningful on a board**: the rounding mode and the exception flags are the processor's (`FPCR`/`FPSR` on AArch64, `FPSCR` on a Cortex-M with an FPU, `fcsr` on RISC-V with F). Not shipped, and clang ships none. Each function is a few instructions per processor through module `asm` or the AArch64 system-register forms; a soft-float target has no flags and answers as a target without them may (`FE_TONEAREST` only, every `fe*except` succeeding for an empty mask). (a), medium. `<math.h>` assumes round-to-nearest, as musl does; see design question 7.

| status | functions |
|---|---|
| missing (a), medium | **feclearexcept** **fegetenv** **fegetexceptflag** **fegetround** **feholdexcept** **feraiseexcept** **fesetenv** **fesetexceptflag** **fesetround** **fetestexcept** **feupdateenv** |

## `<float.h>`

Constants only. clang's `float.h` is used, as `-nostdinc -isystem <resource>/include` puts it back: **the compiler's**. Where `long double` is binary128 the `LDBL_` constants describe binary128, which no cstd function computes in; that is the same line as `<math.h>`'s `l` forms.

## `<fmtmsg.h>`

`fmtmsg` writes a classified message to standard error (and to a console where there is one). **Meaningful on a board** over the existing `write`: (a), small.

| status | functions |
|---|---|
| missing (a), small | fmtmsg |

## `<fnmatch.h>`

Shell pattern matching over two strings. **Meaningful on a board**: (a), small. `glob` and `wordexp` build on it.

| status | functions |
|---|---|
| missing (a), small | fnmatch |

## `<ftw.h>`

File-tree walks. **Needs a file system**: sysl over `<dirent.h>` and `fstatat`. (b), medium, after `<dirent.h>`.

| status | functions |
|---|---|
| missing (b), medium | nftw |

## `<glob.h>`

Path-name expansion. **Needs a file system**: sysl over `<dirent.h>` and `fnmatch`. (b), medium, after both.

| status | functions |
|---|---|
| missing (b), medium | glob globfree |

## `<grp.h>`

The group database. **Needs an OS**: on keel a parse of `/etc/group` through the existing `open` and `read`; a board has no users and the defaults answer no entry. (b), medium.

| status | functions |
|---|---|
| missing (b), medium | getgrgid getgrgid_r getgrnam getgrnam_r |
| optional, not counted | missing: endgrent [XSI] getgrent [XSI] setgrent [XSI] |

## `<iconv.h>`

Character-set conversion. **Meaningful on a board** for the encodings that need no tables: UTF-8, UTF-16 and UTF-32 in either order, ASCII and Latin-1. Anything more needs tables nothing cstd runs on carries, as the README says of locales. (a), medium.

| status | functions |
|---|---|
| missing (a), medium | iconv iconv_close iconv_open |

## `<inttypes.h>`

Shipped, complete. The page also requires `imaxdiv_t`, `wchar_t`, `<stdint.h>`, and a `PRI`/`SCN` macro for each `<stdint.h>` type and each of `d i o u x X` (`PRI`) and `d i o u x` (`SCN`) over `N` = 8, 16, 32, 64, `LEASTN`, `FASTN`, `MAX` and `PTR`: 154 macros, all defined, each the compiler's own `__INT64_FMTd__`-family string for the type its `<stdint.h>` gives that name. `strtoimax` and `strtoumax` are `strtoll` and `strtoull`; `wcstoimax` and `wcstoumax` run the same scan over a `wchar_t` string. The `SCN` macros have no `scanf` to use them yet.

| status | functions |
|---|---|
| tested (`inttypes_tests.sysl`) | **imaxabs** **imaxdiv** **strtoimax** **strtoumax** **wcstoimax** **wcstoumax** |

## `<iso646.h>`

Macros only (`and`, `bitand`, ...): clang's, **the compiler's**.

## `<langinfo.h>`

`nl_langinfo` and `nl_langinfo_l`, the C locale's answers. **Meaningful on a board**: (a), small. Not shipped. `CODESET` is the one choice in it; see design question 3.

| status | functions |
|---|---|
| missing (a), small | nl_langinfo nl_langinfo_l |

## `<libgen.h>`

`basename` and `dirname`, text over a path the caller hands in. **Meaningful on a board**: (a), small.

| status | functions |
|---|---|
| missing (a), small | basename dirname |

## `<libintl.h>`

Message catalogues, new in Issue 8 (`gettext` and its fourteen siblings). **Meaningful on a board** as the C locale has them: with no catalogue every lookup answers its `msgid`, `textdomain` and `bindtextdomain` remember a name, and `bind_textdomain_codeset` answers null. (a), small, and complete as POSIX reads it, because a missing catalogue is a case the page defines.

| status | functions |
|---|---|
| missing (a), small | bind_textdomain_codeset bindtextdomain dcgettext dcgettext_l dcngettext dcngettext_l dgettext dgettext_l dngettext dngettext_l gettext gettext_l ngettext ngettext_l textdomain |

## `<limits.h>`

clang's `limits.h` is used. It has ISO C's constants and **none of POSIX's own**: `SSIZE_MAX`, `LONG_BIT`, `WORD_BIT`, `NL_ARGMAX`, `NL_TEXTMAX`, `CHARCLASS_NAME_MAX`, `IOV_MAX`, `NZERO`, the `_POSIX_` minima such as `_POSIX_PATH_MAX`, and the rest the page lists unconditionally are absent, and nothing in `include/` adds them. A `limits.h` of cstd's own that does `#include_next <limits.h>` and adds them is (a), small; see design question 8.

## `<locale.h>`

Shipped, complete. The C locale is the only locale. `setlocale` and `newlocale` accept `C`, `POSIX` and the empty name (the native locale, which is the C locale here) and refuse any other; `setlocale` answers `C`. `localeconv` answers the C locale's conventions (`decimal_point` `.`, every other string empty, every `char` member `CHAR_MAX`). Every locale object is one static object, so nothing allocates and `freelocale` releases nothing; `LC_GLOBAL_LOCALE` is `(locale_t)-1`. The current locale is one per program, not per thread. **Absent by design**, in the README's words: *Locales other than C. ... A locale's data would have to come from somewhere, and nothing cstd runs on has it.* No function is missing because of it.

| status | functions |
|---|---|
| tested (`locale_tests.sysl`) | duplocale freelocale getlocalename_l **localeconv** newlocale **setlocale** uselocale |

## `<math.h>`

Shipped, complete for `double` and `float`: all 57 functions in both forms, and the twelve classification and comparison macros as the compiler's generic builtins. Each function is judged by MPFR in the `math_*_tests.sysl` file of its family, and `math_c_tests.sysl` pins the macros, the constants, `errno` and Annex F by name; the README's table gives the largest error measured. The 57 `long double` forms exist where `long double` is binary64 (arm64 macOS, 32-bit Arm), each its `double` counterpart under the `l` name, and `include/math.h` declares them only under `__LDBL_MANT_DIG__ == 53`. **Where `long double` is binary128 (AArch64 Linux, which keel's Linux half targets, and RISC-V) they are absent by design**, in the README's words: *a quad-precision argument reduction, polynomials and tables are still to be written. A `double` answer widened would be wrong in its last 60 bits, so nothing is offered there.* That is work, not a language gap (probed, above). The page's macros are all defined (`HUGE_VAL`, `INFINITY`, `NAN`, the `FP_` classes, `FP_ILOGB0` and `FP_ILOGBNAN`, `MATH_ERRNO`, `MATH_ERREXCEPT`, `math_errhandling` as `MATH_ERRNO`); `FP_FAST_FMA` is left undefined, since `fma` is the fused instruction only where the target has one. The [XSI] Bessel functions and `signgam` are optional and missing; the [XSI] `M_` constants are defined.

| status | functions |
|---|---|
| tested (`math_c_tests.sysl` by name, and against MPFR in the `math_*_tests.sysl` file of its family) | **acos** **acosf** **acosh** **acoshf** **acoshl** **acosl** **asin** **asinf** **asinh** **asinhf** **asinhl** **asinl** **atan** **atan2** **atan2f** **atan2l** **atanf** **atanh** **atanhf** **atanhl** **atanl** **cbrt** **cbrtf** **cbrtl** **ceil** **ceilf** **ceill** **copysign** **copysignf** **copysignl** **cos** **cosf** **cosh** **coshf** **coshl** **cosl** **erf** **erfc** **erfcf** **erfcl** **erff** **erfl** **exp** **exp2** **exp2f** **exp2l** **expf** **expl** **expm1** **expm1f** **expm1l** **fabs** **fabsf** **fabsl** **fdim** **fdimf** **fdiml** **floor** **floorf** **floorl** **fma** **fmaf** **fmal** **fmax** **fmaxf** **fmaxl** **fmin** **fminf** **fminl** **fmod** **fmodf** **fmodl** **fpclassify** **frexp** **frexpf** **frexpl** **hypot** **hypotf** **hypotl** **ilogb** **ilogbf** **ilogbl** **isfinite** **isgreater** **isgreaterequal** **isinf** **isless** **islessequal** **islessgreater** **isnan** **isnormal** **isunordered** **ldexp** **ldexpf** **ldexpl** **lgamma** **lgammaf** **lgammal** **llrint** **llrintf** **llrintl** **llround** **llroundf** **llroundl** **log** **log10** **log10f** **log10l** **log1p** **log1pf** **log1pl** **log2** **log2f** **log2l** **logb** **logbf** **logbl** **logf** **logl** **lrint** **lrintf** **lrintl** **lround** **lroundf** **lroundl** **modf** **modff** **modfl** **nan** **nanf** **nanl** **nearbyint** **nearbyintf** **nearbyintl** **nextafter** **nextafterf** **nextafterl** **nexttoward** **nexttowardf** **nexttowardl** **pow** **powf** **powl** **remainder** **remainderf** **remainderl** **remquo** **remquof** **remquol** **rint** **rintf** **rintl** **round** **roundf** **roundl** **scalbln** **scalblnf** **scalblnl** **scalbn** **scalbnf** **scalbnl** **signbit** **sin** **sinf** **sinh** **sinhf** **sinhl** **sinl** **sqrt** **sqrtf** **sqrtl** **tan** **tanf** **tanh** **tanhf** **tanhl** **tanl** **tgamma** **tgammaf** **tgammal** **trunc** **truncf** **truncl** |
| optional, not counted | missing: j0 [XSI] j1 [XSI] jn [XSI] y0 [XSI] y1 [XSI] yn [XSI] |

## `<monetary.h>`

`strfmon` and `strfmon_l`, the C locale's: `localeconv`'s monetary members are all empty or `CHAR_MAX`, so the format is plain numbers. **Meaningful on a board**: (a), medium, since it formats doubles as `printf` does and should reuse its digit generator.

| status | functions |
|---|---|
| missing (a), medium | strfmon strfmon_l |

## `<mqueue.h>`

Message queues. **Needs an OS** (named kernel objects): (b), medium, through the hook on Linux's `mq_*` system calls, where keel has them.

| status | functions |
|---|---|
| missing (b), medium | mq_close mq_getattr mq_notify mq_open mq_receive mq_send mq_setattr mq_timedreceive mq_timedsend mq_unlink |

## `<ndbm.h>`

A key-value database in files. **Needs a file system**: (b), large; nothing in the org calls it.

| status | functions |
|---|---|
| missing (b), large | dbm_clearerr dbm_close dbm_delete dbm_error dbm_fetch dbm_firstkey dbm_nextkey dbm_open dbm_store |

## `<net/if.h>`

Interface names and indices. **Needs an OS network stack**: (b), small, through the hook. On a board lwIP's `netif_name_to_index` is the answer, and it is lwIP's to export.

| status | functions |
|---|---|
| missing (b), small | if_freenameindex if_indextoname if_nameindex if_nametoindex |

## `<netdb.h>`

Name and service lookup (`getaddrinfo`, `gethostent`, `getservbyname`, ...). **Needs an OS network stack and configuration files.** On a board lwIP carries `lwip_getaddrinfo` and its DNS client. (b), large; see design question 5.

| status | functions |
|---|---|
| missing (b), large | endhostent endnetent endprotoent endservent freeaddrinfo gai_strerror getaddrinfo gethostent getnameinfo getnetbyaddr getnetbyname getnetent getprotobyname getprotobynumber getprotoent getservbyname getservbyport getservent sethostent setnetent setprotoent setservent |

## `<netinet/in.h>`

Types and constants only (`struct sockaddr_in`, `struct sockaddr_in6`, `IPPROTO_*`, `INADDR_*`), and two constant objects, `in6addr_any` and `in6addr_loopback`. **Meaningful wherever sockets are**: (a), small. On a board lwIP's own headers define these; see design question 5.

## `<netinet/tcp.h>`

One macro, `TCP_NODELAY`. (a), small, with `<sys/socket.h>`.

## `<nl_types.h>`

Message catalogues by file (`catopen`, `catgets`, `catclose`). **Needs a file system**; without one, `catopen` failing and `catgets` answering its default string is conforming. (b), small.

| status | functions |
|---|---|
| missing (b), small | catclose catgets catopen |

## `<poll.h>`

`poll` and `ppoll`. **Needs an OS**: (b), small, through the hook (`ppoll` is Linux's system call; `poll` is `ppoll` with its timeout converted).

| status | functions |
|---|---|
| missing (b), small | poll ppoll |

## `<pthread.h>`

Threads. **Needs an OS or a scheduler.** On keel the Linux half answers with `clone`, `futex` and a thread-local block per thread, which sysl's `@thread_local` and `errno`'s storage already assume; on a board there is no scheduler in cstd, and a board that has one (FreeRTOS, Zephyr) has its own POSIX layer. The attribute functions are plain struct setters, but none is worth anything before `pthread_create` is. (b), large; see design question 4. The optional groups ([TPS], [TSS], [TSA], [TSH], [RPP], [MC1], [TCT], and the [OB] `pthread_atfork`) are listed and not counted.

| status | functions |
|---|---|
| missing (b), large | pthread_attr_destroy pthread_attr_getdetachstate pthread_attr_getguardsize pthread_attr_getschedparam pthread_attr_init pthread_attr_setdetachstate pthread_attr_setguardsize pthread_attr_setschedparam pthread_barrier_destroy pthread_barrier_init pthread_barrier_wait pthread_barrierattr_destroy pthread_barrierattr_init pthread_cancel pthread_cleanup_pop pthread_cleanup_push pthread_cond_broadcast pthread_cond_clockwait pthread_cond_destroy pthread_cond_init pthread_cond_signal pthread_cond_timedwait pthread_cond_wait pthread_condattr_destroy pthread_condattr_getclock pthread_condattr_init pthread_condattr_setclock pthread_create pthread_detach pthread_equal pthread_exit pthread_getspecific pthread_join pthread_key_create pthread_key_delete pthread_mutex_clocklock pthread_mutex_consistent pthread_mutex_destroy pthread_mutex_init pthread_mutex_lock pthread_mutex_timedlock pthread_mutex_trylock pthread_mutex_unlock pthread_mutexattr_destroy pthread_mutexattr_getrobust pthread_mutexattr_gettype pthread_mutexattr_init pthread_mutexattr_setrobust pthread_mutexattr_settype pthread_once pthread_rwlock_clockrdlock pthread_rwlock_clockwrlock pthread_rwlock_destroy pthread_rwlock_init pthread_rwlock_rdlock pthread_rwlock_timedrdlock pthread_rwlock_timedwrlock pthread_rwlock_tryrdlock pthread_rwlock_trywrlock pthread_rwlock_unlock pthread_rwlock_wrlock pthread_rwlockattr_destroy pthread_rwlockattr_init pthread_self pthread_setcancelstate pthread_setcanceltype pthread_setspecific pthread_spin_destroy pthread_spin_init pthread_spin_lock pthread_spin_trylock pthread_spin_unlock pthread_testcancel |
| optional, not counted | missing: pthread_atfork [OB] pthread_attr_getinheritsched [TPS] pthread_attr_getschedpolicy [TPS] pthread_attr_getscope [TPS] pthread_attr_getstack [TSA TSS] pthread_attr_getstacksize [TSS] pthread_attr_setinheritsched [TPS] pthread_attr_setschedpolicy [TPS] pthread_attr_setscope [TPS] pthread_attr_setstack [TSA TSS] pthread_attr_setstacksize [TSS] pthread_barrierattr_getpshared [TSH] pthread_barrierattr_setpshared [TSH] pthread_condattr_getpshared [TSH] pthread_condattr_setpshared [TSH] pthread_getcpuclockid [TCT] pthread_getschedparam [TPS] pthread_mutex_getprioceiling [RPP] pthread_mutex_setprioceiling [RPP] pthread_mutexattr_getprioceiling [RPP] pthread_mutexattr_getprotocol [MC1] pthread_mutexattr_getpshared [TSH] pthread_mutexattr_setprioceiling [RPP] pthread_mutexattr_setprotocol [MC1] pthread_mutexattr_setpshared [TSH] pthread_rwlockattr_getpshared [TSH] pthread_rwlockattr_setpshared [TSH] pthread_setschedparam [TPS] pthread_setschedprio [TPS] |

## `<pwd.h>`

The user database. **Needs an OS**: on keel a parse of `/etc/passwd`; a board has no users. (b), medium.

| status | functions |
|---|---|
| missing (b), medium | getpwnam getpwnam_r getpwuid getpwuid_r |
| optional, not counted | missing: endpwent [XSI] getpwent [XSI] setpwent [XSI] |

## `<regex.h>`

`regcomp`, `regexec`, `regerror`, `regfree`: POSIX basic and extended expressions. **Meaningful on a board** (it allocates through the existing `malloc`): (a), large. musl's TRE-derived engine is the port to follow.

| status | functions |
|---|---|
| missing (a), large | regcomp regerror regexec regfree |

## `<sched.h>`

`sched_yield` is the only base function. **Needs an OS or a scheduler**: (b), small, through the hook; with one thread it answers 0. The [PS] priority-scheduling functions are optional.

| status | functions |
|---|---|
| missing (b), small | sched_yield |
| optional, not counted | missing: sched_get_priority_max [PS] sched_get_priority_min [PS] sched_getparam [PS] sched_getscheduler [PS] sched_rr_get_interval [PS] sched_setparam [PS] sched_setscheduler [PS] |

## `<search.h>`

`hsearch`, `tsearch`, `lsearch`, `insque` and their siblings: tables over memory the caller hands in or `malloc` gives. **Meaningful on a board**: (a), medium. `tsearch` is a balanced tree (musl's is AVL).

| status | functions |
|---|---|
| missing (a), medium | hcreate hdestroy hsearch insque lfind lsearch remque tdelete tfind tsearch twalk |

## `<semaphore.h>`

Named and unnamed semaphores. **Needs threads**: (b), medium, after `<pthread.h>` (an unnamed semaphore is an atomic counter and a futex).

| status | functions |
|---|---|
| missing (b), medium | sem_clockwait sem_close sem_destroy sem_getvalue sem_init sem_open sem_post sem_timedwait sem_trywait sem_unlink sem_wait |

## `<setjmp.h>`

Shipped, complete. Written for aarch64, thumb (saving s16 to s31 where the target has them) and riscv32 without F; the header refuses to compile anywhere else, as the README says. `sigsetjmp` with a non-zero `savemask` reads and restores the mask through an external `sigprocmask`, which the platform supplies as it supplies `write`. `jmp_buf` and `sigjmp_buf` are one type. The QEMU cases (`tests/qemu/setjmp.c`) skip where `qemu-system-arm`, `qemu-system-riscv32`, `ld.lld` or Homebrew's clang is missing.

| status | functions |
|---|---|
| tested (`setjmp_tests.sysl`) | **longjmp** **setjmp** siglongjmp sigsetjmp |

## `<signal.h>`

Shipped, partly. Present: `kill` (the Linux half's; a weak `ENOSYS` default on a board), `sigprocmask` (the same), and the five set operations, bit arithmetic over a 64-bit `sigset_t`. **ISO C's half is missing**: `signal`, `raise`, `sig_atomic_t`, `SIG_DFL`, `SIG_IGN` and `SIG_ERR`, so a C17 program that says `signal(SIGINT, handler)` does not compile against cstd. On a board the C semantics need no OS: a table of 65 handlers, `signal` swapping an entry, `raise` calling the handler (or, for `SIG_DFL`, `abort` for `SIGABRT` and `_Exit` otherwise), as newlib does; the Linux half then defines both strongly over `rt_sigaction` and `tgkill`. The function-pointer constants are expressible in sysl (probed, above). `psignal`, `psiginfo`, `sig2str` and `str2sig` are text over `strsignal`'s table. `sigaction` and the waits need the kernel's signal frame and a restorer stub in module `asm`: (b), medium, through the hook. **Also missing**: the types `struct sigaction`, `siginfo_t`, `stack_t`, `ucontext_t`, `mcontext_t` and `union sigval` (and so a complete `struct sigevent`); the `SA_` and `SS_` flags, the `SIGEV_` constants, `SIGRTMIN` and `SIGRTMAX`, `SIG2STR_MAX`, `MINSIGSTKSZ` and `SIGSTKSZ`; and six of the base signal numbers, `SIGCONT`, `SIGSTOP`, `SIGTSTP`, `SIGTTIN`, `SIGTTOU` and `SIGWINCH` (with [XSI]'s `SIGURG`, `SIGXCPU` and `SIGXFSZ`).

| status | functions |
|---|---|
| tested (`sigset_tests.sysl`) | sigaddset sigdelset sigemptyset sigfillset sigismember |
| tested (`tests/linux/pty.c`) | sigprocmask |
| tested (`tests/linux/signals.c`) | kill |
| missing (a), small | psiginfo psignal **raise** sig2str **signal** str2sig |
| missing (b), large | pthread_kill pthread_sigmask |
| missing (b), medium | sigaction sigpending sigqueue sigsuspend sigtimedwait sigwait sigwaitinfo |
| optional, not counted | missing: killpg [XSI] sigaltstack [XSI] |

## `<spawn.h>`

`posix_spawn` and its file actions and attributes. **Needs processes**: on keel the Linux half's `clone(CLONE_VM|CLONE_VFORK)` and `execve`; nothing on a board. (b), large. The [PS] attribute pairs are optional.

| status | functions |
|---|---|
| missing (b), large | posix_spawn posix_spawn_file_actions_addchdir posix_spawn_file_actions_addclose posix_spawn_file_actions_adddup2 posix_spawn_file_actions_addfchdir posix_spawn_file_actions_addopen posix_spawn_file_actions_destroy posix_spawn_file_actions_init posix_spawnattr_destroy posix_spawnattr_getflags posix_spawnattr_getpgroup posix_spawnattr_getsigdefault posix_spawnattr_getsigmask posix_spawnattr_init posix_spawnattr_setflags posix_spawnattr_setpgroup posix_spawnattr_setsigdefault posix_spawnattr_setsigmask posix_spawnp |
| optional, not counted | missing: posix_spawnattr_getschedparam [PS] posix_spawnattr_getschedpolicy [PS] posix_spawnattr_setschedparam [PS] posix_spawnattr_setschedpolicy [PS] |

## `<stdalign.h>`

Macros only: clang's, **the compiler's**.

## `<stdarg.h>`

`va_list` and the four macros: clang's, **the compiler's**. cstd's own `stdio.h` includes it.

## `<stdatomic.h>`

clang's, which on a freestanding target is complete in itself: every generic function is a macro over a builtin, and `atomic_thread_fence`, `atomic_signal_fence` and the `atomic_flag` functions are declared and defined as macros over builtins too, so a call needs no symbol. Counted as **the compiler's**, not cstd's to test. What it can still need is outside POSIX: an atomic wider than the processor's lock-free width, or any atomic read-modify-write on Armv6-M (Cortex-M0), becomes a call to the `__atomic_*` or `__sync_*` helpers that compiler-rt supplies and cstd does not.

| status | functions |
|---|---|
| the compiler's (clang's `stdatomic.h`) | **atomic_compare_exchange_strong** **atomic_compare_exchange_strong_explicit** **atomic_compare_exchange_weak** **atomic_compare_exchange_weak_explicit** **atomic_exchange** **atomic_exchange_explicit** **atomic_fetch_add** **atomic_fetch_add_explicit** **atomic_fetch_and** **atomic_fetch_and_explicit** **atomic_fetch_or** **atomic_fetch_or_explicit** **atomic_fetch_sub** **atomic_fetch_sub_explicit** **atomic_fetch_xor** **atomic_fetch_xor_explicit** **atomic_flag_clear** **atomic_flag_clear_explicit** **atomic_flag_test_and_set** **atomic_flag_test_and_set_explicit** **atomic_init** **atomic_is_lock_free** **atomic_load** **atomic_load_explicit** **atomic_signal_fence** **atomic_store** **atomic_store_explicit** **atomic_thread_fence** |

## `<stdbool.h>`

clang's, **the compiler's**.

## `<stddef.h>`

clang's, **the compiler's**. cstd's headers include it for `size_t`, `NULL` and `wchar_t`.

## `<stdint.h>`

clang's, **the compiler's**. cstd's `<unistd.h>` does not make `intptr_t` visible, which the `<unistd.h>` page requires; including `<stdint.h>` there is (a), small.

## `<stdio.h>`

Shipped, a sixth of it. **There is no `FILE`**, and the README does not rule it out (*`FILE` and the rest of stdio. Buffered streams are not written yet.*), so the stream functions are counted missing, not absent by design. Present: the eight printf functions, `puts` and `putchar`, all writing through an external `write` that the Linux half or a board supplies. The eight printf functions disagree with the standard in four ways (*Declarations that disagree*). **Missing with `FILE`**: `FILE`, `fpos_t`, and this header's `off_t` and `ssize_t`; `BUFSIZ`, `FILENAME_MAX`, `FOPEN_MAX`, `TMP_MAX`, `L_tmpnam`, `L_ctermid`, `_IOFBF`, `_IOLBF` and `_IONBF`; `SEEK_SET`, `SEEK_CUR` and `SEEK_END` (they are in `<unistd.h>` only); and `stdin`, `stdout` and `stderr`. The `FILE` layer is sysl over the `read`, `write`, `lseek`, `open` and `close` hooks cstd already has, so it is (a): `stdin`, `stdout` and `stderr` are exported storage holding the addresses of three static `FILE` objects, which the FFI reference names as constant data, so they link on a board. `sscanf`, `vsscanf`, `fmemopen` and `open_memstream` need no descriptor, and `perror` needs only `write`. `rename` and `renameat` need a new hook (`renameat2`); `popen` and `pclose` need processes.

| status | functions |
|---|---|
| tested (`stdio_tests.sysl`) | **putchar** **puts** |
| present, disagrees (tested in `printf_float_tests.sysl`) | **snprintf** **sprintf** |
| present, disagrees (tested in `printf_tests.sysl`) | dprintf **printf** vdprintf **vprintf** **vsnprintf** **vsprintf** |
| missing (a), large | **clearerr** **fclose** fdopen **feof** **ferror** **fflush** **fgetc** **fgetpos** **fgets** fileno **fopen** **fprintf** **fputc** **fputs** **fread** **freopen** **fscanf** **fseek** fseeko **fsetpos** **ftell** ftello **fwrite** **getc** getc_unlocked **getchar** getchar_unlocked getdelim getline **putc** putc_unlocked putchar_unlocked **rewind** **scanf** **setbuf** **setvbuf** **ungetc** **vfprintf** **vfscanf** **vscanf** |
| missing (a), medium | fmemopen open_memstream **sscanf** **vsscanf** |
| missing (a), small | ctermid flockfile ftrylockfile funlockfile **perror** **remove** **tmpfile** **tmpnam** |
| missing (b), large | pclose popen |
| missing (b), small | **rename** renameat |

## `<stdlib.h>`

Shipped, most of it. `malloc`, `calloc`, `realloc`, `free`, `atexit`, `exit`, `_Exit` and `abort` are the Linux half's, and on any other bare-metal target the core's weak defaults (`malloc.sysl`, `exit.sysl`). `strtold` exists where `long double` is binary64 and is absent by design where it is binary128, as `<math.h>`'s `l` forms are. **Missing, C-required**: `aligned_alloc` (the core's `Heap` hands out 16-byte-aligned blocks; a larger alignment is an over-allocation and a header), `at_quick_exit` and `quick_exit` (a second table beside `atexit`'s), and `system`, which on a board may answer 0 for a null command, as C allows where there is no command processor, and on keel needs processes. **Missing, POSIX**: `qsort_r`, `reallocarray`, `getsubopt` and `secure_getenv` are small; `setenv` and `unsetenv` edit `environ` with `malloc`; `mkstemp`, `mkostemp` and `mkdtemp` are `open(O_CREAT|O_EXCL)` and `mkdir` over names drawn from `getentropy` or `rand`, over hooks cstd already has; `realpath` needs `readlink` and `fstatat`.

| status | functions |
|---|---|
| tested (`multibyte_tests.sysl`) | **mblen** **mbstowcs** **mbtowc** **wcstombs** **wctomb** |
| tested (`stdlib_tests.sysl`) | **abs** **atoi** **atol** **atoll** **bsearch** **div** **labs** **ldiv** **llabs** **lldiv** **qsort** **rand** **srand** **strtol** **strtoll** **strtoul** **strtoull** |
| tested (`strtod_tests.sysl`) | **atof** **strtod** **strtof** **strtold** |
| tested (`tests/linux/env.c`) | **getenv** |
| tested (`tests/linux/exit.c`) | **_Exit** **abort** **atexit** **exit** |
| tested (`tests/linux/heap.c`) | **calloc** **free** **malloc** **realloc** |
| missing (a), medium | setenv unsetenv |
| missing (a), small | **aligned_alloc** **at_quick_exit** getsubopt mkdtemp mkostemp mkstemp qsort_r **quick_exit** reallocarray secure_getenv |
| missing (b), large | **system** |
| missing (b), medium | realpath |
| optional, not counted | present: grantpt [XSI] posix_openpt [XSI] ptsname [XSI] ptsname_r [XSI] unlockpt [XSI]; missing: a64l [XSI] drand48 [XSI] erand48 [XSI] initstate [XSI] jrand48 [XSI] l64a [XSI] lcong48 [XSI] lrand48 [XSI] mrand48 [XSI] nrand48 [XSI] posix_memalign [ADV] putenv [XSI] random [XSI] seed48 [XSI] setkey [OB XSI] setstate [XSI] srand48 [XSI] srandom [XSI] |

## `<stdnoreturn.h>`

clang's, **the compiler's**.

## `<string.h>`

Shipped, complete: all 36 required functions, POSIX.1-2024's whole base `<string.h>`. `strdup` and `strndup` call an external `malloc`. `strcoll`, `strxfrm` and the `_l` forms collate as the C locale does, by byte. `memccpy` is [XSI] here and ISO C23 requires it; see *Beyond POSIX*.

| status | functions |
|---|---|
| tested (`string_posix_tests.sysl`) | memmem **memset** stpcpy stpncpy **strcoll** strcoll_l strdup **strerror** strerror_l strerror_r strlcat strlcpy **strlen** strndup strnlen strsignal **strxfrm** strxfrm_l |
| tested (`string_tests.sysl`) | **memchr** **memcmp** **memcpy** **memmove** **strcat** **strchr** **strcmp** **strcpy** **strcspn** **strncat** **strncmp** **strncpy** **strpbrk** **strrchr** **strspn** **strstr** **strtok** strtok_r |
| optional, not counted | missing: memccpy [XSI] |

## `<strings.h>`

Shipped, complete. The four required functions fold ASCII case, the C locale's; `ffs`, `ffsl` and `ffsll` are [XSI] and offered anyway.

| status | functions |
|---|---|
| tested (`strings_tests.sysl`) | strcasecmp strcasecmp_l strncasecmp strncasecmp_l |
| optional, not counted | present: ffs [XSI] ffsl [XSI] ffsll [XSI] |

## `<sys/ipc.h>`

`ftok` and the IPC permission structure, for System V IPC. **Needs an OS**: (b), small.

| status | functions |
|---|---|
| missing (b), small | ftok |

## `<sys/mman.h>`

`mmap`, `mprotect` and `munmap` are the base. **Needs an MMU and an OS**: on keel the Linux half's system calls; on a board an anonymous `mmap` could come from the heap, but a mapping with protections has nothing under it. (b), medium. The rest of the page is optional.

| status | functions |
|---|---|
| missing (b), medium | mmap mprotect munmap |
| optional, not counted | missing: mlock [MLR] mlockall [ML] msync [XSI] munlock [MLR] munlockall [ML] posix_madvise [ADV] posix_mem_offset [TYM] posix_typed_mem_get_info [TYM] posix_typed_mem_open [TYM] shm_open [SHM] shm_unlink [SHM] |

## `<sys/msg.h>`

System V message queues. **Needs an OS**: (b), medium.

| status | functions |
|---|---|
| missing (b), medium | msgctl msgget msgrcv msgsnd |

## `<sys/resource.h>`

`getrlimit` and `setrlimit`. **Needs an OS**: (b), small, through the hook (`prlimit64`).

| status | functions |
|---|---|
| missing (b), small | getrlimit setrlimit |
| optional, not counted | missing: getpriority [XSI] getrusage [XSI] setpriority [XSI] |

## `<sys/select.h>`

`select`, `pselect` and the four `FD_` operations. The `fd_set` operations are bit arithmetic, (a), small, as macros in the header; `select` and `pselect` need an OS (`pselect6`), (b), small.

| status | functions |
|---|---|
| missing (a), small | FD_CLR FD_ISSET FD_SET FD_ZERO |
| missing (b), small | pselect select |

## `<sys/sem.h>`

System V semaphores. **Needs an OS**: (b), medium.

| status | functions |
|---|---|
| missing (b), medium | semctl semget semop |

## `<sys/shm.h>`

System V shared memory. **Needs an OS and an MMU**: (b), medium.

| status | functions |
|---|---|
| missing (b), medium | shmat shmctl shmdt shmget |

## `<sys/socket.h>`

Sockets. **Needs a network stack.** On keel the Linux half's socket system calls, each small; on a board lwIP is the stack, and with `LWIP_COMPAT_SOCKETS` it already provides `socket`, `bind`, `connect` and the rest under these names (as macros or inline functions over `lwip_socket` and its siblings), so cstd defining them too would collide. (b), large as a whole; see design question 5.

| status | functions |
|---|---|
| missing (b), large | accept accept4 bind connect getpeername getsockname getsockopt listen recv recvfrom recvmsg send sendmsg sendto setsockopt shutdown sockatmark socket socketpair |

## `<sys/stat.h>`

Shipped, partly. Present: `stat`, `fstat` and `mkdir` (the Linux half's, weak `ENOSYS` defaults on a board), with the 128-byte asm-generic `struct stat`. **Missing**: eleven functions, each one Linux system call through the hook (`fchmodat`, `fstatat`, `utimensat`, `mkdirat`, `mknodat`, `umask`), (b), small. **Missing macros**: the permission bits `S_IRWXU` `S_IRUSR` `S_IWUSR` `S_IXUSR` `S_IRWXG` `S_IRGRP` `S_IWGRP` `S_IXGRP` `S_IRWXO` `S_IROTH` `S_IWOTH` `S_IXOTH` `S_ISUID` `S_ISGID` `S_ISVTX`, `UTIME_NOW` and `UTIME_OMIT`, the tests `S_ISBLK` and `S_ISSOCK`, the four `S_TYPEIS*`, and [XSI]'s `S_IFBLK`: (a), small. The header reads `restrict` through `<time.h>` and so does not parse as C++ (*Bugs*).

| status | functions |
|---|---|
| tested (`tests/linux/fcntl.c`) | fstat mkdir stat |
| missing (b), small | chmod fchmod fchmodat fstatat futimens lstat mkdirat mkfifo mkfifoat umask utimensat |
| optional, not counted | missing: mknod [XSI] mknodat [XSI] |

## `<sys/statvfs.h>`

`statvfs` and `fstatvfs`. **Needs a file system**: (b), small, through the hook (`statfs`).

| status | functions |
|---|---|
| missing (b), small | fstatvfs statvfs |

## `<sys/time.h>`

`select` (as `<sys/select.h>` declares it) and `utimes`. **Needs an OS**: (b), small.

| status | functions |
|---|---|
| missing (b), small | select utimes |

## `<sys/times.h>`

`times`. **Needs an OS** for its child times; on a board the process times are `clock`'s. (b), small.

| status | functions |
|---|---|
| missing (b), small | times |

## `<sys/types.h>`

Shipped, partly. The page declares no function. Defined: `ssize_t`, `off_t`, `pid_t`, `mode_t`, `uid_t`, `gid_t`, `dev_t`, `ino_t`, `nlink_t`, `blksize_t`, `blkcnt_t`, `time_t`, `clockid_t`, and `size_t` through `<stddef.h>`. **Missing**: `clock_t` and `timer_t` (in `<time.h>` only), `fsblkcnt_t`, `fsfilcnt_t`, `id_t`, `reclen_t`, and the twelve `pthread_*` types; `key_t` and `suseconds_t` are [XSI]. **`time_t` disagrees**: it is `long`, so 32 bits on a Cortex-M and on riscv32, and POSIX.1-2024 requires a width of at least 64 bits (Austin Group defect 1462, applied in Issue 8).

## `<sys/uio.h>`

`readv` and `writev`. **Needs an OS** as `read` and `write` do: (b), small, through the hook, whose core default may loop over the board's `read` and `write` rather than answer `ENOSYS`, the Linux half replacing it with the system calls.

| status | functions |
|---|---|
| missing (b), small | readv writev |

## `<sys/un.h>`

`struct sockaddr_un`. (a), small, with `<sys/socket.h>`.

## `<sys/utsname.h>`

`uname`. **Needs an OS** on keel (the system call); on a board fixed strings serve. (b), small.

| status | functions |
|---|---|
| missing (b), small | uname |

## `<sys/wait.h>`

`wait`, `waitid`, `waitpid` and the `W` macros. **Needs processes**: (b), medium, with `fork`.

| status | functions |
|---|---|
| missing (b), medium | wait waitid waitpid |

## `<syslog.h>`

`openlog`, `syslog`, `setlogmask`, `closelog`. **Needs an OS** for a log daemon; on a board, writing to descriptor 2 is a conforming sink. (b), small.

| status | functions |
|---|---|
| missing (b), small | closelog openlog setlogmask syslog |

## `<tar.h>`

Constants only. **Meaningful anywhere**: (a), small, one header.

## `<termios.h>`

Terminal control (`tcgetattr`, `tcsetattr`, `cfgetospeed`, ...). **Needs an OS**: each is an `ioctl` on keel through the hook. The Linux half's pseudo-terminal tests already set raw mode through the kernel, so the structure is known. (b), medium.

| status | functions |
|---|---|
| missing (b), medium | cfgetispeed cfgetospeed cfsetispeed cfsetospeed tcdrain tcflow tcflush tcgetattr tcgetsid tcgetwinsize tcsendbreak tcsetattr tcsetwinsize |

## `<tgmath.h>`

clang ships a `tgmath.h`, and it includes `<complex.h>`, which cstd does not have, so a program including it does not compile against cstd. It is present the day `<complex.h>` is: (a), small, after `<complex.h>`.

## `<threads.h>`

C11 threads, required by POSIX.1-2024 and optional in C17. **Needs a scheduler**: over `<pthread.h>` on keel, nothing on a board. (b), large, after `<pthread.h>`.

| status | functions |
|---|---|
| missing (b), large | **call_once** **cnd_broadcast** **cnd_destroy** **cnd_init** **cnd_signal** **cnd_timedwait** **cnd_wait** **mtx_destroy** **mtx_init** **mtx_lock** **mtx_timedlock** **mtx_trylock** **mtx_unlock** **thrd_create** **thrd_current** **thrd_detach** **thrd_equal** **thrd_exit** **thrd_join** **thrd_sleep** **thrd_yield** **tss_create** **tss_delete** **tss_get** **tss_set** |

## `<time.h>`

Shipped, nearly complete. There is no time-zone database, so local time is UTC: `localtime`, `localtime_r`, `mktime` and `tzset` need no `TZ` and sit in the core. `time`, `clock` and `timespec_get` are in the core too, built on `clock_gettime` and compiled only for a freestanding target. Every clock, sleep and timer call is the Linux half's, with a weak `ENOSYS` default on a board. The header defines `clock_t`, `struct tm` (with `tm_gmtoff` and `tm_zone`), `struct timespec`, `struct itimerspec`, `timer_t`, `locale_t`, `CLOCKS_PER_SEC`, `TIME_UTC`, `TIMER_ABSTIME`, the four `CLOCK_` names and `tzname`, and declares `timegm` (ISO C23's; minmea calls it). **Missing**: `asctime` and `ctime`, which POSIX marks [OB] and C17 still requires (each is `strftime` of `%a %b %e %H:%M:%S %Y` and a newline into a static buffer), (a), small. `struct sigevent` is declared incomplete. **`time_t` is 32 bits on a Cortex-M and riscv32**, which POSIX.1-2024 refuses (*`<sys/types.h>`*). `daylight` and `timezone` are [XSI].

| status | functions |
|---|---|
| tested (`tests/linux/time.c`) | **clock** clock_getres clock_nanosleep clock_settime nanosleep timer_create timer_delete timer_getoverrun timer_gettime timer_settime |
| tested (`tests/qemu/clock.c`) | **timespec_get** |
| tested (`time_tests.sysl`) | clock_gettime **difftime** **gmtime** gmtime_r **localtime** localtime_r **mktime** **strftime** strftime_l **time** tzset |
| missing (a), small | **asctime** **ctime** |
| optional, not counted | missing: clock_getcpuclockid [CPT] getdate [XSI] strptime [XSI] |

## `<uchar.h>`

`mbrtoc16`, `c16rtomb`, `mbrtoc32` and `c32rtomb` over the C locale's one-byte encoding (C23 adds `mbrtoc8` and `c8rtomb`). **Meaningful on a board**: (a), small. Not shipped, and clang ships none.

| status | functions |
|---|---|
| missing (a), small | **c16rtomb** **c32rtomb** **mbrtoc16** **mbrtoc32** |

## `<unistd.h>`

Shipped, a fifth of it. Present: fourteen of the sixteen calls of the README's table (`brk` and `sbrk` are not POSIX's), the Linux half's with weak `ENOSYS` defaults on a board (`stubs_tests.sysl` checks both), and `environ`. **Missing, (b), small**, one Linux system call each through the hook: the ids (`getuid` ... `setsid`), `access` and `faccessat`, `chown`, `fchown`, `fchownat` and `lchown`, `link`, `linkat`, `symlink`, `symlinkat`, `readlink`, `readlinkat` and `unlinkat`, `fchdir`, `ftruncate` and `truncate`, `pread` and `pwrite`, `dup3` and `pipe2`, `alarm` and `pause`, `getentropy` (`getrandom`; a board's is its random-number generator), `gethostname`, `getlogin` and `getlogin_r`, `isatty`, `ttyname` and `ttyname_r`, `tcgetpgrp` and `tcsetpgrp`. **(b), large**: `fork`, `_Fork`, the six `exec` functions and `fexecve`, which need keel's process model and which no board can answer. **(a), small**: `getopt` with `optarg`, `opterr`, `optind` and `optopt` (exported storage with constant initial values, `optind` 1), `sleep` (over the `nanosleep` hook), `posix_close` (over `close`), and `sysconf`, `pathconf`, `fpathconf` and `confstr`, which answer constants. **Missing macros**: `F_OK`, `R_OK`, `W_OK`, `X_OK`, `SEEK_DATA`, `SEEK_HOLE`, `POSIX_CLOSE_RESTART`, `_POSIX_VERSION` and the other option macros, every `_SC_`, `_PC_` and `_CS_` name; and `intptr_t`. `brk` and `sbrk` are declared here though POSIX.1-2024 has neither.

| status | functions |
|---|---|
| tested (`tests/linux/exit.c`) | _exit |
| tested (`tests/linux/fcntl.c`) | close read unlink write |
| tested (`tests/linux/fds.c`) | dup dup2 lseek pipe |
| tested (`tests/linux/files.c`) | chdir getcwd rmdir |
| tested (`tests/linux/signals.c`) | getpid getppid |
| missing (a), small | confstr fpathconf getopt pathconf posix_close sleep sysconf |
| missing (b), large | _Fork execl execle execlp execv execve execvp fexecve fork |
| missing (b), small | access alarm chown dup3 faccessat fchdir fchown fchownat ftruncate getegid getentropy geteuid getgid getgroups gethostname getlogin getlogin_r getpgid getpgrp getsid getuid isatty lchown link linkat pause pipe2 pread pwrite readlink readlinkat setegid seteuid setgid setpgid setsid setuid symlink symlinkat tcgetpgrp tcsetpgrp truncate ttyname ttyname_r unlinkat |
| optional, not counted | missing: crypt [XSI] encrypt [OB XSI] fdatasync [SIO] fsync [FSC] gethostid [XSI] getresgid [XSI] getresuid [XSI] lockf [XSI] nice [XSI] setregid [XSI] setresgid [XSI] setresuid [XSI] setreuid [XSI] swab [XSI] sync [XSI] |

## `<utmpx.h>`

The user-accounting database. **Needs an OS**: (b), small; nothing in the org calls it.

| status | functions |
|---|---|
| missing (b), small | endutxent getutxent getutxid getutxline pututxline setutxent |

## `<wchar.h>`

Wide characters. **Meaningful on a board** in the C locale, where a wide character is a byte value below 0x80 and the multibyte encoding is `<stdlib.h>`'s. Not shipped, and clang ships none. Two thirds of it is text over memory (`wcs*`, `wmem*`, `mbr*`, `wcsto*`, `wcsftime`, `swprintf`, `swscanf`): (a), medium, much of it `<string.h>`, `<stdlib.h>` and `strftime` over a wider element. The stream half (`fgetwc`, `fwprintf`, `getwchar`, ...) waits for `FILE`: (a), large with it. `wcstold` follows `strtold`. `wcwidth` and `wcswidth` are [XSI].

| status | functions |
|---|---|
| missing (a), large | **fgetwc** **fgetws** **fputwc** **fputws** **fwide** **fwprintf** **fwscanf** **getwc** **getwchar** open_wmemstream **putwc** **putwchar** **ungetwc** **vfwprintf** **vfwscanf** **vwprintf** **vwscanf** **wprintf** **wscanf** |
| missing (a), medium | **btowc** **mbrlen** **mbrtowc** **mbsinit** mbsnrtowcs **mbsrtowcs** **swprintf** **swscanf** **vswprintf** **vswscanf** wcpcpy wcpncpy **wcrtomb** wcscasecmp wcscasecmp_l **wcscat** **wcschr** **wcscmp** **wcscoll** wcscoll_l **wcscpy** **wcscspn** wcsdup **wcsftime** wcslcat wcslcpy **wcslen** wcsncasecmp wcsncasecmp_l **wcsncat** **wcsncmp** **wcsncpy** wcsnlen wcsnrtombs **wcspbrk** **wcsrchr** **wcsrtombs** **wcsspn** **wcsstr** **wcstod** **wcstof** **wcstok** **wcstol** **wcstold** **wcstoll** **wcstoul** **wcstoull** **wcsxfrm** wcsxfrm_l **wctob** **wmemchr** **wmemcmp** **wmemcpy** **wmemmove** **wmemset** |
| optional, not counted | missing: wcswidth [XSI] wcwidth [XSI] |

## `<wctype.h>`

Wide-character classes and mappings (`iswalpha` ... `towupper`, `wctype`, `wctrans`, and the `_l` forms), the C locale's: a wide character above 0x7F is in no class. **Meaningful on a board**: (a), small, the same table as `<ctype.h>`.

| status | functions |
|---|---|
| missing (a), small | **iswalnum** iswalnum_l **iswalpha** iswalpha_l **iswblank** iswblank_l **iswcntrl** iswcntrl_l **iswctype** iswctype_l **iswdigit** iswdigit_l **iswgraph** iswgraph_l **iswlower** iswlower_l **iswprint** iswprint_l **iswpunct** iswpunct_l **iswspace** iswspace_l **iswupper** iswupper_l **iswxdigit** iswxdigit_l **towctrans** towctrans_l **towlower** towlower_l **towupper** towupper_l **wctrans** wctrans_l **wctype** wctype_l |

## `<wordexp.h>`

Shell word expansion. **Needs a shell** (command substitution is part of it): (b), large; nothing in the org calls it.

| status | functions |
|---|---|
| missing (b), large | wordexp wordfree |

## Beyond POSIX

**`<stdbit.h>`** is ISO C23's (section 7.18), not POSIX's, and clang ships none. cstd offers it whole: the 14 families in each of the five unsigned types (70 functions, `stdbit_tests.sysl`), the 14 type-generic macros, `__STDC_VERSION_STDBIT_H__` and the three `__STDC_ENDIAN_` macros. It is not counted in the Summary.

**What C23 adds that POSIX.1-2024 does not require**, for a C library that owes C first: `memccpy` (POSIX's [XSI]), `memset_explicit`, `memalignment`, `strfromd`, `strfromf` and `strfroml`, `free_sized` and `free_aligned_sized`, `timespec_getres`, `mbrtoc8` and `c8rtomb`, `timegm`, and `<math.h>`'s new families (`exp10`, `rootn`, `pown`, `compoundn`, `roundeven`, `fmaximum` and its siblings, `nextup`/`nextdown`, the narrowing `fadd`/`fmul`/... and the rest). Of these cstd has `timegm` only; `memccpy`, `memset_explicit`, `strfromd`/`strfromf` and `free_sized` are (a), small, and C23's `<math.h>` is (a), large. clang ships `stdckdint.h`, so C23's checked arithmetic is the compiler's.

**Names cstd exports that no standard defines**: `brk` and `sbrk` (declared in `unistd.h`), `__errno_location`, `__cstd_assert_fail` (`assert`'s failure path), `cstd_sigsetjmp_tail` (`sigsetjmp`'s ending) and the Linux half's `cstd_start` and `_start`.

**What the board packages call.** Each of `bearssl`, `lwip`, `littlefs`, `fatfs`, `u8g2`, `minmea` and `heatshrink` was built with `sysl build-c . --target thumbv7em-freestanding` against cstd, and each archive's undefined symbols were read: every C-library call they make is satisfied today. They call `memcmp`, `memcpy`, `memmove`, `memset`, `strlen`, `strncmp`, `strchr`, `strcpy`, `strcspn`, `strspn`, `atoi`, `strtol`, `abs`, `isdigit`, `islower`, `isprint`, `isspace`, `isxdigit`, `printf` and `timegm`, and nothing else from a C library. `tinyusb` builds only for the RP2350 and was read instead of built: `memcpy`, `memset`, `memmove`, `strlen`, `printf` and `snprintf`. So no gap below is on a board package's path; their optional settings reach only `malloc` and `free` (littlefs's `LFS_MALLOC`, FatFs's long-file-name buffers), which the core's defaults supply.

## Summary

Required functions (and function-like macros), per header and status. Headers that require no function
(`assert.h`, `cpio.h`, `errno.h`, `float.h`, `iso646.h`, `limits.h`, `netinet/in.h`, `netinet/tcp.h`,
`stdalign.h`, `stdarg.h`, `stdbool.h`, `stddef.h`, `stdint.h`, `stdnoreturn.h`, `sys/types.h`,
`sys/un.h`, `tar.h`, `tgmath.h`) are left out; their macros and types are in their sections.

| header | required | tested | untested | disagrees | missing (a) | missing (b) | missing (c) | compiler's | optional |
|---|---|---|---|---|---|---|---|---|---|
| `aio.h` | 7 | | | | | 7 | | | 1 |
| `arpa/inet.h` | 6 | | | | 6 | | | | 2 |
| `complex.h` | 69 | | | | 69 | | | | |
| `ctype.h` | 28 | 28 | | | | | | | |
| `devctl.h` | 1 | | | | | 1 | | | |
| `dirent.h` | 9 | | | | | 9 | | | 3 |
| `dlfcn.h` | 5 | | | | | 5 | | | |
| `endian.h` | 12 | 12 | | | | | | | |
| `fcntl.h` | 4 | 4 | | | | | | | 2 |
| `fenv.h` | 11 | | | | 11 | | | | |
| `fmtmsg.h` | 1 | | | | 1 | | | | |
| `fnmatch.h` | 1 | | | | 1 | | | | |
| `ftw.h` | 1 | | | | | 1 | | | |
| `glob.h` | 2 | | | | | 2 | | | |
| `grp.h` | 4 | | | | | 4 | | | 3 |
| `iconv.h` | 3 | | | | 3 | | | | |
| `inttypes.h` | 6 | 6 | | | | | | | |
| `langinfo.h` | 2 | | | | 2 | | | | |
| `libgen.h` | 2 | | | | 2 | | | | |
| `libintl.h` | 15 | | | | 15 | | | | |
| `locale.h` | 7 | 7 | | | | | | | |
| `math.h` | 183 | 183 | | | | | | | 6 |
| `monetary.h` | 2 | | | | 2 | | | | |
| `mqueue.h` | 10 | | | | | 10 | | | |
| `ndbm.h` | 9 | | | | | 9 | | | |
| `net/if.h` | 4 | | | | | 4 | | | |
| `netdb.h` | 22 | | | | | 22 | | | |
| `nl_types.h` | 3 | | | | | 3 | | | |
| `poll.h` | 2 | | | | | 2 | | | |
| `pthread.h` | 73 | | | | | 73 | | | 29 |
| `pwd.h` | 4 | | | | | 4 | | | 3 |
| `regex.h` | 4 | | | | 4 | | | | |
| `sched.h` | 1 | | | | | 1 | | | 7 |
| `search.h` | 11 | | | | 11 | | | | |
| `semaphore.h` | 11 | | | | | 11 | | | |
| `setjmp.h` | 4 | 4 | | | | | | | |
| `signal.h` | 22 | 7 | | | 6 | 9 | | | 2 |
| `spawn.h` | 19 | | | | | 19 | | | 4 |
| `stdatomic.h` | 28 | | | | | | | 28 | |
| `stdio.h` | 66 | 2 | | 8 | 52 | 4 | | | |
| `stdlib.h` | 49 | 35 | | | 12 | 2 | | | 23 |
| `string.h` | 36 | 36 | | | | | | | 1 |
| `strings.h` | 4 | 4 | | | | | | | 3 |
| `sys/ipc.h` | 1 | | | | | 1 | | | |
| `sys/mman.h` | 3 | | | | | 3 | | | 11 |
| `sys/msg.h` | 4 | | | | | 4 | | | |
| `sys/resource.h` | 2 | | | | | 2 | | | 3 |
| `sys/select.h` | 6 | | | | 4 | 2 | | | |
| `sys/sem.h` | 3 | | | | | 3 | | | |
| `sys/shm.h` | 4 | | | | | 4 | | | |
| `sys/socket.h` | 19 | | | | | 19 | | | |
| `sys/stat.h` | 14 | 3 | | | | 11 | | | 2 |
| `sys/statvfs.h` | 2 | | | | | 2 | | | |
| `sys/time.h` | 2 | | | | | 2 | | | |
| `sys/times.h` | 1 | | | | | 1 | | | |
| `sys/uio.h` | 2 | | | | | 2 | | | |
| `sys/utsname.h` | 1 | | | | | 1 | | | |
| `sys/wait.h` | 3 | | | | | 3 | | | |
| `syslog.h` | 4 | | | | | 4 | | | |
| `termios.h` | 13 | | | | | 13 | | | |
| `threads.h` | 25 | | | | | 25 | | | |
| `time.h` | 24 | 22 | | | 2 | | | | 3 |
| `uchar.h` | 4 | | | | 4 | | | | |
| `unistd.h` | 75 | 14 | | | 7 | 54 | | | 15 |
| `utmpx.h` | 6 | | | | | 6 | | | |
| `wchar.h` | 74 | | | | 74 | | | | 2 |
| `wctype.h` | 36 | | | | 36 | | | | |
| `wordexp.h` | 2 | | | | | 2 | | | |
| **all of POSIX** | **1093** | **367** | **0** | **8** | **324** | **366** | **0** | **28** | **125** |
| **C17's headers** | **539** | **274** | **0** | **6** | **204** | **27** | **0** | **28** | |
| **C17, mandatory** | **417** | **274** | **0** | **6** | **135** | **2** | **0** | | |

*C17's headers* are the bold names: everything a page lists unmarked in one of C17's 29 headers, with
`asctime`, `ctime` and `tmpnam`. *C17, mandatory* leaves out the three headers C17 makes optional
(`<complex.h>`, `<threads.h>`, `<stdatomic.h>`). Every present function is called by a test, so none is
untested; the Linux half's run only where the `cstd-test` colima profile is up, and the QEMU cases only
where QEMU is installed, and both skip otherwise. **Absent by design: none counted.** The README's two
reasons cover the 57 `l` forms of `<math.h>` and `strtold` where `long double` is binary128 (counted
tested, since they exist and are tested where it is binary64) and the locales other than C (no function
missing). Of the optional functions, 8 are present (`ffs`, `ffsl`, `ffsll` and the five pseudo-terminal
functions) and 117 missing.

Of the 135 C-mandatory functions missing (a), 36 are `<stdio.h>`'s, 59 `<wchar.h>`'s, 18
`<wctype.h>`'s, 11 `<fenv.h>`'s, 4 `<uchar.h>`'s, 3 `<stdlib.h>`'s, 2 `<signal.h>`'s and 2 `<time.h>`'s;
the two (b) are `rename` and `system`.

## Declarations that disagree

Every prototype `include/` declares was compiled against the page's own prototype of the same function,
written after the `#include` with the name parenthesised (`size_t (strlen)(const char *);`), for
`aarch64-linux-gnu` and `thumbv7em-none-eabi`; clang reports no conflicting type for any, and two
deliberately wrong lines in the same run (`strlen` answering `int`, `strtol` taking a `long` base) are
reported. The disagreements are in behaviour and in types:

- **The printf family** (`printf`, `dprintf`, `sprintf`, `snprintf`, `vprintf`, `vdprintf`, `vsprintf`,
  `vsnprintf`), in four ways; every other conversion C defines is taken:
  1. `%ls` reads its argument as bytes, not as a `wchar_t` string, so `snprintf(b, 64, "<%ls>", L"hi")`
     writes `<h>` and answers 3, where C writes `<hi>` and answers 4. `%lc` writes the low byte of the
     wide character, where C converts it and fails with `EILSEQ` for one the locale cannot encode
     (*Bugs*).
  2. **A count past `INT_MAX` wraps**: `snprintf(NULL, 0, "%2147483647d%d", 1, 1)` answers
     -2147483648 and leaves `errno` alone, where POSIX requires -1 and `EOVERFLOW` (*Bugs*).
  3. **POSIX's numbered conversions (`%2$d`) are not taken** [CX]: `"<%2$d %1$d>"` is copied out as
     written and its arguments are not read.
  4. **`L` is not taken even where `long double` is a `double`** (arm64 macOS, 32-bit Arm), so
     `%Lf` of `1.5L` writes `%Lf`; C requires it there. Where `long double` is binary128 it is absent by
     design with `strtold`.
- **`time_t` is 32 bits on a Cortex-M and on riscv32** (`typedef long time_t`), and POSIX.1-2024 requires
  at least 64 (Austin Group defect 1462). It reaches every declaration that names `time_t`, `struct
  timespec` and `struct stat`: `time`, `mktime`, `timegm`, `gmtime`, `localtime`, `difftime`,
  `clock_gettime` and the rest on those targets. The README states the 32 bits as a fact
  (*`time_t` is `long`, so 32 bits on a Cortex-M*). On AArch64, keel's target, it is 64.
- **`errno`** is one object on a board, where there is no thread-local storage; POSIX's is per thread.
  With no threads on a board, no program can tell.
- **`brk` and `sbrk`** are declared in `unistd.h` though POSIX.1-2024 has neither, and `sbrk` takes a
  `long` where the historical prototype took an `intptr_t` (the same width on every cstd target). The
  ROADMAP moves them to a header of cstd's own.
- **`<unistd.h>` does not make `intptr_t` visible**, and **`<stdio.h>` does not make `off_t`, `ssize_t`
  or the `SEEK_` macros visible**, though both pages require them.
- `__restrict` is written for `restrict` in every header but `time.h`, so the rest also read as C++;
  the two are equivalent in C.

## Bugs

Found by the audit and not fixed here; each has a reproduction against the v0.1.9 archive, built as
`c_case` builds a case (`clang -std=c11 -O0 -fno-builtin -nostdinc -isystem <resource>/include -I
include prog.c libcstd.a`, `libcstd.a` from `sysl build-c .` on the host):

1. **`%ls` prints a wide string as bytes** (`printf.sysl`, the `s` conversion reads `*u8` whatever the
   length). `wchar_t w[] = L"hi"; snprintf(b, 64, "<%ls>", w)` writes `<h>`, answers 3; C writes `<hi>`,
   answers 4. `%lc` of a wide character above 0x7F writes its low byte instead of failing with `EILSEQ`.
2. **A printf count past `INT_MAX` wraps to a negative number** (`answer` in `printf.sysl` narrows
   `s.total` with `i32(...)`). `snprintf(NULL, 0, "%2147483647d%d", 1, 1)` answers -2147483648 with
   `errno` 0; POSIX: -1 and `EOVERFLOW`.
3. **`<time.h>`, and so `<sys/stat.h>`, do not parse as C++**: they write `restrict`, where every other
   header writes `__restrict` for exactly that reason (`stdio.h`'s comment says so) and each opens an
   `extern "C"` block. `echo '#include <time.h>' | clang -x c++ -std=c++17 --target=aarch64-linux-gnu
   -ffreestanding -nostdinc -isystem $(clang -print-resource-dir)/include -I include -fsyntax-only -`
   fails with `time.h:67:44: error: expected ')'` (seven errors); every other header is clean.
4. **The README contradicts itself on allocation**: its *What is deliberately absent* list says
   *Allocation and `exit` in the portable half ... a board's arena is still to come*, while its *On a
   board* section and `board_defaults_tests.sysl` describe and test the core's weak `malloc` family and
   `exit` over `sbrk`.

## Design questions

1. **Which operating-system headers cstd rules out.** The README rules out none, so `<dlfcn.h>`,
   `<ndbm.h>`, `<utmpx.h>`, `<wordexp.h>`, System V IPC, `<aio.h>`, `<mqueue.h>` and, on a board,
   processes and `<spawn.h>` all count as missing (b): 366 functions. Saying in the README which of them
   cstd will never offer, and why, moves them to *absent by design* and leaves a list that is a plan.
2. **The `FILE` layer**: its buffering; where `stdin`, `stdout` and `stderr` point on a board; whether
   `printf` and `vprintf` become `vfprintf(stdout, …)` (buffered, flushed by `exit`) while `dprintf`
   stays over `write`; and whether the memory-only parts (`sscanf`, `fmemopen`, `open_memstream`) come
   first, since they need no descriptor.
3. **The C locale's `CODESET`**: ASCII, which is what `MB_CUR_MAX` 1 and the multibyte functions say
   today, or UTF-8 (`C.UTF-8`), which would change `MB_CUR_MAX`, `mbtowc` and every wide conversion.
   `nl_langinfo`, `iconv` and the wide-character headers all wait on this one answer.
4. **Threads**: on keel, `clone` and `futex` in the Linux half; on a board, whether cstd offers
   `<pthread.h>` and `<threads.h>` at all, over a scheduler of its own or a board's, or refuses them.
   The heap is not thread-safe until this is answered (the README says so).
5. **Networking on a board**: lwIP defines the socket names itself (`LWIP_COMPAT_SOCKETS`) and has its own
   `netdb` and `inet` functions. Whether cstd's `<sys/socket.h>`, `<netdb.h>`, `<netinet/in.h>` and
   `<arpa/inet.h>` exist only for keel, or defer to lwIP's on a board, decides whether they are (b) or
   absent by design there.
6. **Widening `time_t` to 64 bits** on the 32-bit targets is a breaking change for any package that
   passes a `time_t` across its C boundary on a board (minmea's `timegm` result): its cost is that chain,
   not the one-line `typedef`.
7. **`<fenv.h>` against `<math.h>`**: the mathematics assumes round-to-nearest, as musl does. Under
   `fesetround(FE_UPWARD)` its answers leave their stated error bounds. Either the README says so, as
   musl's documentation does, or the functions that need it save and restore the mode.
8. **`<limits.h>`**: whether cstd ships a `limits.h` of its own, with `#include_next` to clang's, to
   carry POSIX's constants.

## Suggested order

C-required first, small before large; then what the Linux half needs for keel. The board packages
need nothing further (*What the board packages call*), so they set no order.

1. **The printf disagreements** — `%ls`/`%lc`, `EOVERFLOW`, `L` where `long double` is a `double`
   (small each); `%n$` (medium: a first pass that reads the types, then a walk with `va_copy`).
2. **`<signal.h>`'s C half** — `signal`, `raise`, `sig_atomic_t`, `SIG_DFL`/`SIG_IGN`/`SIG_ERR` in the
   core, then strong Linux definitions (small); `psignal`, `psiginfo`, `sig2str`, `str2sig` (small).
3. **Small C gaps** — `asctime`, `ctime`; `aligned_alloc`, `quick_exit`, `at_quick_exit`; `perror`;
   the thirty `errno` codes; `<stdio.h>`'s constants and the `SEEK_` macros (small each).
4. **`time_t` to 64 bits** (small to write; a breaking change, design question 6).
5. **The `FILE` layer** — `fopen` to `vfscanf` over the existing hooks, `stdin`/`stdout`/`stderr`,
   `sscanf` and `vsscanf` first (large; design question 2). This is C's largest gap.
6. **`<wctype.h>` and `<uchar.h>`** (small), then **`<wchar.h>`**'s memory half (medium) and its stream
   half after `FILE` (large).
7. **`<fenv.h>`** (medium), per processor.
8. **`<complex.h>`**, then `<tgmath.h>` comes free (large).
9. **The pure POSIX extras** — `getopt`, `sleep`, `sysconf` and friends, `<libgen.h>`, `<fnmatch.h>`,
   `<arpa/inet.h>`, the `FD_` macros, `qsort_r`, `reallocarray`, `strfmon` (small to medium); then
   `<search.h>` (medium), `<iconv.h>` (medium), `<regex.h>` (large).
10. **The Linux half for keel** — the small system calls of `<unistd.h>` and `<sys/stat.h>` (54 and 11,
    small each, a morning's batch per header); `sigaction` and its waits (medium); `<dirent.h>`,
    `<termios.h>`, `<poll.h>`, `<sys/mman.h>` (small to medium); then `<pthread.h>` and `<threads.h>`
    (large), processes (`fork`, `exec`, `wait`, `<spawn.h>`, large) and sockets (large), after design
    questions 4 and 5. The ROADMAP's busybox build orders this step better than any list can: a trial
    build names what it reaches first.
