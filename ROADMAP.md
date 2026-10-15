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

1. **Weak exports** (sysl alpha.14, branch `weak-export`). The OS-level calls (`open`, `read`, `write`,
   `lseek`, `close`, `sbrk`, `clock_gettime`, …) become weak `ENOSYS` stubs in the core. The Linux half
   gives them real system calls, and a board overrides any one.
2. **`errno` as a thread-local macro**, in place of today's plain `extern int errno`.
3. **The OS-facing headers**, one per item: stdio's `FILE` layer (buffered over `read`/`write`, with
   memory streams in the core), `unistd.h`, `fcntl.h`, `sys/stat.h`, `time.h` (with `struct tm` and
   `timegm`), `signal.h`.
4. **`brk`/`sbrk` out of `unistd.h`**, into a header of cstd's own.
5. **The headers busybox adds**, in the order a trial build asks for them.
6. **The busybox build itself:** a minimal applet set first (`echo`, `cat`, `ls`, `sh`), then the
   defconfig, with the musl-linked busybox as the oracle.

## Meanwhile

- `math.h` (in progress, against an MPFR oracle).
- On alpha.14: `setjmp`'s s16–s31 arm for Arm targets with an FPU (`target-fpu`); cstd as a
  freestanding-only dependency of the vendored packages (`vendored-c`, `target-deps`); `inttypes.h`
  et al. reached from them.
