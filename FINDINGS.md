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
`linux/sh/sysl/cstd_linux/crt0.sysl` is written in the wanted form and does not compile until then.
