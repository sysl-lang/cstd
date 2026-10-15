/* <ctype.h> from sh.sysl.cstd, for the C locale: only ASCII bytes belong to a class.
 * A true answer is 1. */
#ifndef CSTD_CTYPE_H
#define CSTD_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

int isalnum(int c);
int isalpha(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

#ifndef __CSTD_LOCALE_T
#define __CSTD_LOCALE_T
typedef struct __cstd_locale *locale_t;
#endif

/* Every locale is the C locale, so each answers as its plain twin whatever locale it is given. */
int isalnum_l(int c, locale_t loc);
int isalpha_l(int c, locale_t loc);
int isblank_l(int c, locale_t loc);
int iscntrl_l(int c, locale_t loc);
int isdigit_l(int c, locale_t loc);
int isgraph_l(int c, locale_t loc);
int islower_l(int c, locale_t loc);
int isprint_l(int c, locale_t loc);
int ispunct_l(int c, locale_t loc);
int isspace_l(int c, locale_t loc);
int isupper_l(int c, locale_t loc);
int isxdigit_l(int c, locale_t loc);
int tolower_l(int c, locale_t loc);
int toupper_l(int c, locale_t loc);

#ifdef __cplusplus
}
#endif

#endif
