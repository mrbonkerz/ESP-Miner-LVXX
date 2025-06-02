#include "sha/sha_core.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "mining.h"
#include "utils.h"
#include "mbedtls/sha256.h"


static const double truediffone = 26959535291011309493156476344723991336010898738574164086137773096960.0;


void free_bm_job(bm_job *job) {
    if (!job) return;
    free(job->jobid);
    free(job->extranonce2);
    free(job);
}


char *construct_coinbase_tx(const char *coinbase_1, const char *coinbase_2, const char *extranonce, const char *extranonce_2) {

    const size_t len1 = strlen(coinbase_1);
    const size_t len2 = strlen(coinbase_2);
    const size_t len_ex1 = strlen(extranonce);
    const size_t len_ex2 = strlen(extranonce_2);

    char *coinbase_tx = malloc(len1 + len_ex1 + len_ex2 + len2 + 1);
    if (!coinbase_tx) return NULL;
    
    char *ptr = coinbase_tx;
    memcpy(ptr, coinbase_1, len1); ptr += len1;
    memcpy(ptr, extranonce, len_ex1); ptr += len_ex1;
    memcpy(ptr, extranonce_2, len_ex2); ptr += len_ex2;
    memcpy(ptr, coinbase_2, len2); ptr += len2;
    *ptr = '\0';
    
    return coinbase_tx;
}


char *calculate_merkle_root_hash(const char *coinbase_tx, const uint8_t merkle_branches[][32], const int num_merkle_branches) {
	
    const size_t coinbase_tx_bin_len = strlen(coinbase_tx) / 2;
    uint8_t *coinbase_tx_bin = malloc(coinbase_tx_bin_len);
    if (!coinbase_tx_bin) return NULL;
    
    hex2bin(coinbase_tx, coinbase_tx_bin, coinbase_tx_bin_len);
    
    uint8_t both_merkles[64];
    uint8_t *current_hash = double_sha256_bin(coinbase_tx_bin, coinbase_tx_bin_len);
    free(coinbase_tx_bin);
    
    memcpy(both_merkles, current_hash, 32);
    free(current_hash);
    
    for (int i = 0; i < num_merkle_branches; i++) {
        memcpy(both_merkles + 32, merkle_branches[i], 32);
        current_hash = double_sha256_bin(both_merkles, 64);
        memcpy(both_merkles, current_hash, 32);
        free(current_hash);
    }
    
    char *merkle_root_hash = malloc(65);
    if (merkle_root_hash) {
        bin2hex(both_merkles, 32, merkle_root_hash, 65);
    }
    return merkle_root_hash;
}


bm_job construct_bm_job(Stratum_Mining_Notify_t *params, const char *merkle_root, const uint32_t version_mask) {
	
    bm_job new_job = {0}; 
    
    new_job.version = params->version;
    new_job.starting_nonce = 0;
    new_job.target = params->target;
    new_job.ntime = params->ntime;
    new_job.pool_diff = params->difficulty;

    hex2bin(merkle_root, new_job.merkle_root, 32);
    swap_endian_words(merkle_root, new_job.merkle_root_be);
    reverse_bytes(new_job.merkle_root_be, 32);

    swap_endian_words(params->prev_block_hash, new_job.prev_block_hash);
    hex2bin(params->prev_block_hash, new_job.prev_block_hash_be, 32);
    reverse_bytes(new_job.prev_block_hash_be, 32);

    uint8_t midstate_data[64];
    memcpy(midstate_data, &new_job.version, 4);
    memcpy(midstate_data + 4, new_job.prev_block_hash, 32);
    memcpy(midstate_data + 36, new_job.merkle_root, 28);

    midstate_sha256_bin(midstate_data, 64, new_job.midstate);
    reverse_bytes(new_job.midstate, 32);
    
    if (version_mask != 0) {
        uint32_t rolled_version = increment_bitmask(new_job.version, version_mask);
        for (int i = 0; i < 3; i++) {
            memcpy(midstate_data, &rolled_version, 4);
            midstate_sha256_bin(midstate_data, 64, new_job.midstate1 + (i * 32));
            reverse_bytes(new_job.midstate1 + (i * 32), 32);
            rolled_version = increment_bitmask(rolled_version, version_mask);
        }
        new_job.num_midstates = 4;
    } else {
        new_job.num_midstates = 1;
    }
    
    return new_job;
}


char *extranonce_2_generate(uint32_t extranonce_2, uint32_t length)
{
    char *extranonce_2_str = malloc(length * 2 + 1);    
    
    memset(extranonce_2_str, '0', length * 2);
    extranonce_2_str[length * 2] = '\0';
    bin2hex((uint8_t *)&extranonce_2, length, extranonce_2_str, length * 2 + 1);
    
    if (length > 4)
    {
        extranonce_2_str[8] = '0';
    }
    return extranonce_2_str;
}


double test_nonce_value(const bm_job *job, const uint32_t nonce, const uint32_t rolled_version)
{
    uint8_t header[80] = {0};
    uint8_t hash_result[32];
    
    *(uint32_t*)(header) = rolled_version;
    memcpy(header + 4, job->prev_block_hash, 32);
    memcpy(header + 36, job->merkle_root, 32);
    *(uint32_t*)(header + 68) = job->ntime;
    *(uint32_t*)(header + 72) = job->target;
    *(uint32_t*)(header + 76) = nonce;

    esp_sha(SHA2_256, header, 80, hash_result);
    esp_sha(SHA2_256, hash_result, 32, hash_result);

    return truediffone / le256todouble(hash_result);
}


uint32_t increment_bitmask(const uint32_t value, const uint32_t mask) {
    if (mask == 0) return value;
    
    const uint32_t masked_bits = value & mask;
    const uint32_t increment = mask & -mask; 

    uint32_t new_value = (value & ~mask) | ((masked_bits + increment) & mask);
    
    const uint32_t overflow = (masked_bits + increment) & ~mask;
    if (overflow) {
        new_value = increment_bitmask(new_value, overflow << 1);
    }
    
    return new_value;
}

