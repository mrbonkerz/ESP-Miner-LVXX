#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stddef.h>
#include <stdint.h>

#define STRATUM_DEFAULT_VERSION_MASK 0x1fffe000


static const char hex_digits[16] = "0123456789abcdef";
static const uint8_t hex_vals[256] = {
    ['0'] = 0,  ['1'] = 1,  ['2'] = 2,  ['3'] = 3,
    ['4'] = 4,  ['5'] = 5,  ['6'] = 6,  ['7'] = 7,
    ['8'] = 8,  ['9'] = 9,  ['a'] = 10, ['b'] = 11,
    ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15,
    ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13,
    ['E'] = 14, ['F'] = 15
};

// Constants as power-of-two for exact representation
static const double pow2_192 = 6277101735386680763835789423207666416102355444464034512896.0; // 2^192
static const double pow2_128 = 340282366920938463463374607431768211456.0; // 2^128
static const double pow2_64  = 18446744073709551616.0; // 2^64


size_t hex2bin(const char *restrict hex, uint8_t *restrict bin, size_t bin_len);
size_t bin2hex(const uint8_t *restrict bin, size_t bin_len, char *restrict hex, size_t hex_len);
uint8_t *double_sha256_bin(const uint8_t *data, const size_t data_len);
void midstate_sha256_bin(const uint8_t *data, const size_t data_len, uint8_t *dest);
void swap_endian_words(const char *hex_words, uint8_t *output);
void reverse_bytes(uint8_t *data, size_t len);
double le256todouble(const void *restrict target);
void prettyHex(unsigned char *buf, int len);
void print_hex(const uint8_t *b, size_t len, const size_t in_line, const char *prefix);

#endif /* INC_UTILS_H_ */
