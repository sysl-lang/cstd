# cstd — findings

Gaps and bugs in sysl found while writing cstd. Each is reported to the sysl compiler and fixed there,
never worked around here.

## 1. On a hosted target, a function exported under a C runtime name compiles into a call to itself (bug)

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
