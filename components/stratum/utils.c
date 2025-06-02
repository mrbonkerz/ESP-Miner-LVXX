
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include "esp_private/periph_ctrl.h"
#include "soc/hwcrypto_reg.h"
#include "sha/sha_core.h"


__attribute__((hot, always_inline)) 
inline void flip32bytes(void *restrict dest_p, const void *restrict src_p) {
	
    uint32_t temp[8];
    memcpy(temp, src_p, 32);

    temp[0] = __builtin_bswap32(temp[0]);
    temp[1] = __builtin_bswap32(temp[1]);
    temp[2] = __builtin_bswap32(temp[2]);
    temp[3] = __builtin_bswap32(temp[3]);
    temp[4] = __builtin_bswap32(temp[4]);
    temp[5] = __builtin_bswap32(temp[5]);
    temp[6] = __builtin_bswap32(temp[6]);
    temp[7] = __builtin_bswap32(temp[7]);

    memcpy(dest_p, temp, 32);
}


__attribute__((hot))
size_t hex2bin(const char *restrict hex, uint8_t *restrict bin, size_t bin_len) {
	
    size_t count = 0;
    
    while (*hex && count < bin_len) {
        uint8_t msn = hex_vals[(uint8_t)*hex++];
        if (!*hex) break;
        
        uint8_t lsn = hex_vals[(uint8_t)*hex++];
        bin[count++] = (msn << 4) | lsn;
    }
    
    return count;
}


__attribute__((hot))
size_t bin2hex(const uint8_t *restrict bin, size_t bin_len, char *restrict hex, size_t hex_len) {
	
    if (hex_len < (bin_len * 2 + 1)) return 0;
    
    for (size_t i = 0; i < bin_len; i++) {
        hex[i*2]   = hex_digits[bin[i] >> 4];
        hex[i*2+1] = hex_digits[bin[i] & 0x0F];
    }
    hex[bin_len*2] = '\0';
    
    return bin_len * 2;
}


uint8_t* double_sha256_bin(const uint8_t* data, const size_t data_len) {
	
    uint8_t* output = malloc(32);
    if (!output) return NULL;
    
    uint8_t first_hash[32];

    esp_sha(SHA2_256, data, data_len, first_hash);
    esp_sha(SHA2_256, first_hash, 32, output);
    
    return output;
}


void midstate_sha256_bin(const uint8_t *data, const size_t data_len, uint8_t *dest) {

    if (data_len < 64) {
        memset(dest, 0, 32);
        return;
    }

    periph_module_enable(PERIPH_SHA_MODULE);
    REG_WRITE(SHA_MODE_REG, 2); //#define SHA_MODE_SHA256	2

    for (int i = 0; i < 16; i++) {
        REG_WRITE(SHA_TEXT_BASE + (i * 4), 
                 *(uint32_t*)(data + (i * 4)));
    }
    
    REG_WRITE(SHA_START_REG, 1);
    
    while (REG_READ(SHA_BUSY_REG));
    
    for (int i = 0; i < 8; i++) {
        uint32_t word = REG_READ(SHA_H_BASE + (i * 4));
        dest[i*4]   = (word >> 24) & 0xff;
        dest[i*4+1] = (word >> 16) & 0xff;
        dest[i*4+2] = (word >> 8)  & 0xff;
        dest[i*4+3] = word & 0xff;
    }
}


void swap_endian_words(const char *hex_words, uint8_t *output) {
	
    size_t hex_length = strlen(hex_words);
    if (hex_length % 8 != 0)
    {
        fprintf(stderr, "Must be 4-byte word aligned\n");
        exit(EXIT_FAILURE);
    }
	for(size_t i=0; i<hex_length; i+=8) {
	    uint32_t word;
	    hex2bin(hex_words+i, (uint8_t*)&word, 4);
	    *(uint32_t*)(output+i/2) = __builtin_bswap32(word);
	}
}


__attribute__((hot)) 
void reverse_bytes(uint8_t *data, size_t len) {
	
    uint8_t *start = data;
    uint8_t *end = data + len - 1;
    
    while (start < end) {
        uint8_t temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}


__attribute__((hot))
double le256todouble(const void *restrict target) {
	
    const uint8_t *t = target;
    uint64_t chunk;
    
    memcpy(&chunk, t + 24, 8);
    double result = (double)chunk * pow2_192;
    
    memcpy(&chunk, t + 16, 8);
    result += (double)chunk * pow2_128;
    
    memcpy(&chunk, t + 8, 8);
    result += (double)chunk * pow2_64;
    
    memcpy(&chunk, t, 8);
    result += (double)chunk;
    
    return result;
}


void prettyHex(unsigned char *buf, int len) {
	
    int i;
    printf("[");
    for (i = 0; i < len - 1; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("%02X]", buf[len - 1]);
}

void print_hex(const uint8_t *b, size_t len, const size_t in_line, const char *prefix) {
	
    size_t i = 0;
    const uint8_t *end = b + len;

    if (prefix == NULL)
    {
        prefix = "";
    }

    printf("%s", prefix);
    while (b < end)
    {
        if (++i > in_line)
        {
            printf("\n%s", prefix);
            i = 1;
        }
        printf("%02X ", (uint8_t)*b++);
    }
    printf("\n");
    fflush(stdout);
}
