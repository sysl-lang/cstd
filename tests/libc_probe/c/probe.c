/* A vendored C file of the kind a package carries: it includes the standard headers by their system
 * names and calls one function from each. On a machine with no libc, the headers and the functions
 * are cstd's; on a hosted machine they are the real libc's. The answer is the same either way: 0, or
 * a bit for each header whose call came back wrong. */

#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

/* The format macros expand to string literals that concatenate with the rest of a format. */
static const char print_formats[] = "%" PRId8 "%" PRIi16 "%" PRIu32 "%" PRIx64 "%" PRIXMAX "%" PRIdPTR
                                    "%" PRIoLEAST16 "%" PRIuFAST32;
static const char scan_formats[] = "%" SCNd8 "%" SCNi16 "%" SCNu32 "%" SCNx64 "%" SCNdMAX "%" SCNuPTR
                                   "%" SCNoLEAST16 "%" SCNxFAST32;

int cstd_libc_probe(void) {
    int wrong = 0;

    if (strlen("vendored") != 8) wrong |= 1;

    if (toupper('q') != 'Q' || isdigit('x')) wrong |= 2;

    char *end;
    intmax_t n = strtoimax("  -0x7fffffffffffffff!", &end, 16);
    imaxdiv_t d = imaxdiv(n, 1000);
    if (n != -INTMAX_MAX || *end != '!' || d.quot != -INTMAX_MAX / 1000 || d.rem != -INTMAX_MAX % 1000
        || imaxabs(-5) != 5)
        wrong |= 4;
    if (sizeof print_formats < 17 || sizeof scan_formats < 17 || print_formats[0] != '%') wrong |= 4;

    volatile double two = 2.0;
    if (sqrt(two * two) != 2.0 || isnan(sqrt(-two)) == 0) wrong |= 8;

    return wrong;
}
