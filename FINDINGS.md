# cstd — findings

Gaps and bugs in sysl found while writing cstd. Each is reported to the sysl compiler and fixed there,
never worked around here.

## 1. On a hosted target, a function exported under a C runtime name compiles into a call to itself (bug)

**Fixed in sysl 0.1.0-alpha.7.** On a hosted target, `_strlen` now compiles to a byte loop and
`_memset` to a store loop. The first two cases in `string_tests.sysl` are the regression tests.

    module mylib

    @export("strlen")
    c_strlen(s: *u8) -> usize
        var n: usize = 0
        while s[n] != 0 do n += 1
        n

    @export("memset")
    c_memset(dst: *u8, c: i32, n: usize) -> *u8
        for i in 0..<n do dst[i] = u8(c)
        dst

    $ sysl build-c . -o host.a          # 0.1.0-alpha.6, aarch64-macos
    $ otool -tv sysl.code.o
    _strlen:  ret
    _memset:  cbz x2, …; b _memset

No diagnostic. Linked into C, `strlen("hello")` answers garbage and `memset` with `n > 0` never
returns.

The `emit-llvm` text is right. `toolchain.sysl`'s `function_attributes` adds `"no-builtins"` only for
a target that is not hosted, so LLVM's loop-idiom pass rewrites the loops into calls to `@strlen` and
`@memset` — which here are the exports themselves. LLVM's own guard compares the name of the function
being compiled, and the body is compiled as `mylib$c_strlen`, so it never fires. The freestanding
build is correct; Apple clang at `-O2`, hosted, compiles the same C to a byte loop and a vectorized
store loop.

What should happen: a body reachable from an `@export` of a C runtime name is compiled
`"no-builtins"` (or the manifest can declare freestanding code, C's `-ffreestanding`, whatever the
target). The workaround not taken: disguising the loops (`volatile`) so LLVM does not recognise them.

## 2. A C runtime cannot call `main(argc, argv, envp)` (gap)

**Fixed in sysl 0.1.0-alpha.10.** `linux/`'s crt0 declares the three-argument form and
`sysl build-c linux --target aarch64-freestanding` builds; keel's boot test runs a C program through it.

Found writing `linux/`'s crt0 (sysl 0.1.0-alpha.9). The archive names its entry
(`targets.aarch64-freestanding.entry = "_start"`), so `extern "main"` is allowed — but only at
`(int, **byte) -> int`:

    extern "main" program_main(argc: int, argv: **u8, envp: **u8) -> int

    $ sysl build-c linux --target aarch64-freestanding -o libcstd.a
    error: 'main' is defined at C's signature, '(int, **byte) -> int' — the argument count and the
    argument vector — so an 'extern' naming it takes those, where this one is
    '(int, **byte, **byte) -> int'

`reference/packages.md § A C runtime written in sysl calls the C program's main` holds the extern to
the two-argument form "as above", whose reason is about a *sysl* program's `main`, which the compiler
emits at two parameters. A C runtime calls a *C* program's `main`, and every Unix crt0 (musl's
`__libc_start_main`, glibc's) passes three: `int main(int argc, char **argv, char **envp)` is the
common extension C11 J.5.1 names, and a C program declaring it reads garbage from x2 under a crt0 that
passes two. Passing an argument a two-parameter `main` does not read is harmless in every C
convention.

What should happen: in a C runtime's archive, `extern "main"` also takes the three-argument form
`(int, **byte, **byte) -> int`. The workaround not taken: loading x2 with `envp` in `_start`'s asm
before branching, or calling the two-argument form and leaving three-argument programs broken.
`linux/sh/sysl/cstd_linux/crt0.sysl` was written in the wanted form and did not compile until then.

## 3. Nothing an `asm` arm or `#if` can test tells an Arm target with a floating-point unit from one without (gap)

Found writing `setjmp` (sysl 0.1.0-alpha.12). On a Cortex-M with a unit, s16–s31 are callee-saved
whatever the float ABI, so `setjmp` must store them, and on a part without one the same instructions
do not assemble. `thumb-freestanding`, `thumb-freestanding-softfp` and `thumbv7em-freestanding` have a
unit; `thumb-freestanding-soft`, `thumbv6m-`, `thumbv7m-` and `thumbv7em-freestanding-soft` do not. All
seven select the one `[thumb]` arm, and `#if`'s vocabulary has `thumb` and nothing finer, so one
archive cannot carry the right `setjmp` for its target. The target row knows (`fpu`, `no_fpu` in
`target.sysl`, which is also what decides `__ARM_FP` for C):

    #if thumb && fpu
    asm
        [thumb]
            "vstmia r0!, {{s16-s31}}"
            ...
    #endif

    $ sysl build-c . --target thumb-freestanding -o probe.a
    error: 'fpu' is not something a target says about itself — sysl knows aarch64, android, bsd,
    craft, freestanding, hosted, linux, macos, posix, riscv32, riscv64, thumb, wasi, wasm32, windows,
    x86, x86_64. A feature this package's manifest declares is named 'feature_fpu'

A manifest feature is the consumer's choice, not the target's, so it would let a hard-float build
forget to ask for it — the silent loss this is meant to prevent.

What should happen: a symbol for it (`fpu`, or `arm_fp`, by analogy with `__ARM_FP`), or an arm
qualifier such as `[thumb+fpu]`. The same question will come back for riscv64's F/D
(`riscv64-linux` has D, `riscv64-freestanding` does not), where `#if linux` happens to answer it.

The workaround not taken: both variants in the archive, the floating one assembled under a local
`.arch armv7e-m` / `.fpu fpv4-sp-d16`, and `include/setjmp.h` picking a symbol by `__ARM_FP`. Instead
`setjmp.sysl`'s thumb arm saves the core registers only, which is right without a unit, and
`include/setjmp.h` stops with `#error` wherever `__ARM_FP` is defined, so no build silently loses
s16–s31.

**Fixed** by the target-derived symbols `fpu`, `fpu_dp`, `hardfloat` and `softfp`, defined true or
false on every target and usable in `#if` inside an `asm` arm. The thumb arm saves s16–s31 under
`#if fpu`, the header's `#error` is gone, and the Cortex-M4F QEMU case in `setjmp_tests.sysl` is the
regression test.

## 4. A `c"…"` literal is refused as an exported object's initializer, though the reference calls it constant data (bug)

Found writing `<time.h>` (sysl 0.1.2). POSIX's `extern char *tzname[]` is exported C data whose two
pointers name static text, which in sysl is `c"…"`:

    module sh.sysl.probe

    @export("p")
    var p: *u8 = c"UTC"

    $ sysl build-c . -o p.a
    error: 'sh.sysl.probe.p' is exported, so C may read it before anything of this program has run —
    an exported object starts at the value the object file carries, and this initializer is code that
    runs before the program's statements. Make it constant data — a literal, a 'const', the address of
    other module storage, and the arrays and structs built from those — or leave the value off to
    start it at its type's zero and fill it from a function C calls first

`val p: *u8 = c"UTC"` and `var tzname: [2]*u8 = [c"UTC", c"UTC"]` are refused the same way.
`reference/ffi.md § What constant data is` names a literal as constant data, and
`reference/strings.md` says `c"…"` is the address of a string literal already in read-only data: a
relocation, like the `&files[1]` the same section accepts. The address of module storage is accepted
(`[&name[0], &name[0]]` over a `[4]u8` builds).

What should happen: `c"…"` is constant data, laid down as a pointer to the literal's bytes.

The workaround not taken: the name spelled as a byte array (`[85, 84, 67, 0]`) and `tzname` built
from its address. `tzname` is left out of cstd, and `time.h` does not declare it, until this is
fixed.
