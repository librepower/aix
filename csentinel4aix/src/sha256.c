/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * sha256.c - SHA256 implementation (no external dependencies)
 *
 * Based on RFC 6234 / FIPS 180-4
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* SHA256 context structure */
typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} sha256_ctx_t;

/* SHA256 constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes) */
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* Rotate right */
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

/* SHA256 functions */
#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)       (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x)       (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x)      (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x)      (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

/* Process a 64-byte block */
static void sha256_transform(sha256_ctx_t *ctx, const uint8_t *data) {
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    uint32_t W[64];
    int i;
    
    /* Prepare message schedule */
    for (i = 0; i < 16; i++) {
        W[i] = ((uint32_t)data[i * 4] << 24) |
               ((uint32_t)data[i * 4 + 1] << 16) |
               ((uint32_t)data[i * 4 + 2] << 8) |
               ((uint32_t)data[i * 4 + 3]);
    }
    for (i = 16; i < 64; i++) {
        W[i] = SIG1(W[i - 2]) + W[i - 7] + SIG0(W[i - 15]) + W[i - 16];
    }
    
    /* Initialize working variables */
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];
    
    /* Main loop */
    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + W[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    
    /* Add to state */
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

/* Initialize SHA256 context */
void sha256_init(sha256_ctx_t *ctx) {
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->count = 0;
}

/* Update SHA256 with data */
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len) {
    size_t index = (ctx->count / 8) % 64;
    
    ctx->count += len * 8;
    
    /* Fill buffer and transform if full */
    if (index) {
        size_t left = 64 - index;
        if (len < left) {
            memcpy(ctx->buffer + index, data, len);
            return;
        }
        memcpy(ctx->buffer + index, data, left);
        sha256_transform(ctx, ctx->buffer);
        data += left;
        len -= left;
    }
    
    /* Transform full blocks */
    while (len >= 64) {
        sha256_transform(ctx, data);
        data += 64;
        len -= 64;
    }
    
    /* Buffer remaining */
    if (len) {
        memcpy(ctx->buffer, data, len);
    }
}

/* Finalize SHA256 and output hash */
void sha256_final(sha256_ctx_t *ctx, uint8_t *hash) {
    uint8_t pad[64];
    uint64_t bits = ctx->count;
    size_t index = (bits / 8) % 64;
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);
    int i;
    
    /* Padding */
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;
    sha256_update(ctx, pad, pad_len);
    
    /* Append length (big-endian) */
    for (i = 0; i < 8; i++) {
        pad[i] = (bits >> (56 - i * 8)) & 0xff;
    }
    sha256_update(ctx, pad, 8);
    
    /* Output hash (big-endian) */
    for (i = 0; i < 8; i++) {
        hash[i * 4]     = (ctx->state[i] >> 24) & 0xff;
        hash[i * 4 + 1] = (ctx->state[i] >> 16) & 0xff;
        hash[i * 4 + 2] = (ctx->state[i] >> 8) & 0xff;
        hash[i * 4 + 3] = ctx->state[i] & 0xff;
    }
}

/* Compute SHA256 of a file, output as hex string */
int sha256_file(const char *path, char *out, size_t out_size) {
    FILE *f;
    sha256_ctx_t ctx;
    uint8_t buffer[4096];
    uint8_t hash[32];
    size_t bytes;
    int i;
    
    if (out_size < 65) {
        return -1;  /* Need 64 chars + null */
    }
    
    f = fopen(path, "rb");
    if (!f) {
        strncpy(out, "error", out_size - 1);
        out[out_size - 1] = '\0';
        return -1;
    }
    
    sha256_init(&ctx);
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha256_update(&ctx, buffer, bytes);
    }
    
    fclose(f);
    sha256_final(&ctx, hash);
    
    /* Convert to hex string */
    for (i = 0; i < 32; i++) {
        sprintf(out + i * 2, "%02x", hash[i]);
    }
    out[64] = '\0';
    
    return 0;
}

/* Compute SHA256 of a string, output as hex string */
int sha256_string(const char *str, char *out, size_t out_size) {
    sha256_ctx_t ctx;
    uint8_t hash[32];
    int i;
    
    if (out_size < 65) {
        return -1;
    }
    
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)str, strlen(str));
    sha256_final(&ctx, hash);
    
    for (i = 0; i < 32; i++) {
        sprintf(out + i * 2, "%02x", hash[i]);
    }
    out[64] = '\0';
    
    return 0;
}
