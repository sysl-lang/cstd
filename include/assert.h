/* <assert.h> from sh.sysl.cstd.
 *
 * Deliberately NO include guard: the macro is redefined from the state of NDEBUG each time the
 * header is included, as C requires. A failed assertion writes
 *     Assertion failed: <expression> (<file>: <function>: <line>)
 * to descriptor 2 and calls abort(). */

#undef assert

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(e) ((e) ? (void)0 : __cstd_assert_fail(#e, __func__, __FILE__, __LINE__))
#endif

#ifndef CSTD_ASSERT_H
#define CSTD_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

_Noreturn void __cstd_assert_fail(const char *expr, const char *func, const char *file, int line);

#ifdef __cplusplus
}
#endif

/* static_assert is a macro for _Static_assert before C23 and a keyword from C23 on (and in C++). */
#if !defined(__cplusplus) && !defined(static_assert) && \
    (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L)
#define static_assert _Static_assert
#endif

#endif
