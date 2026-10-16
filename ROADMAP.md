# cstd roadmap

## The target: busybox, built from source against cstd, running on keel

keel already runs Alpine's busybox unmodified: a static binary with musl inside it, talking to keel
through Linux's system-call ABI. The target is to build the same busybox from source with cstd as its
C library, in place of musl, and run it on keel. That makes every layer, from the kernel up through the
C library, sysl.

It is the measure of the OS-facing half of cstd: stdio's `FILE` layer, `unistd.h`, `fcntl.h`,
`sys/stat.h`, `time.h`, `signal.h`, and the headers busybox reaches beyond them (`dirent.h`, `pwd.h`,
`grp.h`, `termios.h`, `poll.h`, `glob.h`, `regex.h`, the socket headers, `getopt`). Each header cstd
offers keeps the rule in `POSIX.md`: once offered, it carries every in-scope POSIX.1-2024 function.

How it is checked:

- **Busybox as the oracle.** Each applet built against cstd runs on keel beside the same applet linked
  against musl, and their output must agree.
- **What busybox needs that is out of scope.** busybox uses some optional ([XSI]) and obsolescent
  functions, which cstd leaves out. A build against cstd lists exactly which. Each is then decided case
  by case: added, or its applet configured out.

## Before it

1. **`errno` as a thread-local macro**, in place of today's plain `extern int errno`.
2. **The OS-facing headers**, one per item: stdio's `FILE` layer (buffered over `read`/`write`, with
   memory streams in the core), `unistd.h`, `fcntl.h`, `sys/stat.h`, `time.h` (with `struct tm` and
   `timegm`), `signal.h`. Each new operating-system call gets its weak `ENOSYS` default in
   `stubs.sysl` beside its Linux definition.
3. **`brk`/`sbrk` out of `unistd.h`**, into a header of cstd's own.
4. **The headers busybox adds**, in the order a trial build asks for them.
5. **busybox from source against musl first.** keel's milestone 18 runs Alpine's prebuilt
   `busybox-static` package, unmodified (pinned by SHA-256 in keel's boot test); nothing builds busybox
   from source yet. So the first step is a reproducible from-source build: busybox 1.37 cross-compiled
   with clang to static aarch64 Linux against musl, which must pass keel's milestone-18 transcript as
   Alpine's binary does. That gives the build recipe and the oracle.
6. **The same build against cstd:** a minimal applet set first (`echo`, `cat`, `ls`, `sh`), then the
   defconfig, each applet's output compared with the musl build's on keel.

## Meanwhile

- `math.h` (in progress, against an MPFR oracle).
- The remaining `math.h` constants written in decimal (fdlibm's `erf`, `lgamma`/`tgamma`, inverse
  trigonometric and trigonometric coefficient tables) as hex floats, and a fused `fma` in the
  double-double and logarithm kernels (`math_dd.sysl`, `math_log.sysl`) where `#if fma_dp`.
