/* <locale.h> from sh.sysl.cstd. The C locale (also named "POSIX") is the only locale: setlocale and
 * newlocale accept "C", "POSIX" and "" and refuse every other name. The current locale is one per
 * program, not per thread. */
#ifndef CSTD_LOCALE_H
#define CSTD_LOCALE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CSTD_LOCALE_T
#define __CSTD_LOCALE_T
typedef struct __cstd_locale *locale_t;
#endif

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    char int_p_cs_precedes;
    char int_n_cs_precedes;
    char int_p_sep_by_space;
    char int_n_sep_by_space;
    char int_p_sign_posn;
    char int_n_sign_posn;
};

#define LC_CTYPE 0
#define LC_NUMERIC 1
#define LC_TIME 2
#define LC_COLLATE 3
#define LC_MONETARY 4
#define LC_MESSAGES 5
#define LC_ALL 6

#define LC_CTYPE_MASK (1 << LC_CTYPE)
#define LC_NUMERIC_MASK (1 << LC_NUMERIC)
#define LC_TIME_MASK (1 << LC_TIME)
#define LC_COLLATE_MASK (1 << LC_COLLATE)
#define LC_MONETARY_MASK (1 << LC_MONETARY)
#define LC_MESSAGES_MASK (1 << LC_MESSAGES)
#define LC_ALL_MASK (LC_CTYPE_MASK | LC_NUMERIC_MASK | LC_TIME_MASK | LC_COLLATE_MASK | \
                     LC_MONETARY_MASK | LC_MESSAGES_MASK)

#define LC_GLOBAL_LOCALE ((locale_t)-1)

char *setlocale(int category, const char *locale);
struct lconv *localeconv(void);
locale_t newlocale(int category_mask, const char *locale, locale_t base);
locale_t duplocale(locale_t locobj);
void freelocale(locale_t locobj);
locale_t uselocale(locale_t newloc);
const char *getlocalename_l(int category, locale_t locobj);

#ifdef __cplusplus
}
#endif

#endif
