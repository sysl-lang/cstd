/* <endian.h> from sh.sysl.cstd, as POSIX.1-2024 (Issue 8) specifies it. The byte order is the
 * compiler's own answer for the target, so the same header serves a big-endian machine. The twelve
 * conversions are functions only: the header says "functions, or macros, or both". */
#ifndef CSTD_ENDIAN_H
#define CSTD_ENDIAN_H

#include <stdint.h>

#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define BYTE_ORDER __BYTE_ORDER__

#ifdef __cplusplus
extern "C" {
#endif

uint16_t be16toh(uint16_t big_endian_16bits);
uint32_t be32toh(uint32_t big_endian_32bits);
uint64_t be64toh(uint64_t big_endian_64bits);
uint16_t htobe16(uint16_t host_16bits);
uint32_t htobe32(uint32_t host_32bits);
uint64_t htobe64(uint64_t host_64bits);
uint16_t htole16(uint16_t host_16bits);
uint32_t htole32(uint32_t host_32bits);
uint64_t htole64(uint64_t host_64bits);
uint16_t le16toh(uint16_t little_endian_16bits);
uint32_t le32toh(uint32_t little_endian_32bits);
uint64_t le64toh(uint64_t little_endian_64bits);

#ifdef __cplusplus
}
#endif

#endif
