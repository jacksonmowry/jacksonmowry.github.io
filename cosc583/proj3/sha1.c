#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define rotl(x, n) ((x << n) | (x >> (32 - n)))
/* #define rotr(x, n) ((x >> n) | (x << ((sizeof(w) * 8) - n))) */
#define k1 0x5a827999
#define k2 0x6ed9eba1
#define k3 0x8f1bbcdc
#define k4 0xca62c1d6

#define h0 0x67452301
#define h1 0xefcdab89
#define h2 0x98badcfe
#define h3 0x10325476
#define h4 0xc3d2e1f0

char *pad_message(const char *message, size_t *new_len) {
    const size_t message_len = strlen(message);

    const size_t k = 64 - ((message_len + 9) % 64);

    const size_t new_message_len = message_len + 9 + k;
    char *new_buf = calloc(new_message_len, 1);
    strcpy(new_buf, message);
    new_buf[message_len] = 0x80;
    const size_t message_len_bits = message_len * 8;
    *new_len = new_message_len;

    new_buf[new_message_len - 8] = (message_len_bits >> (8 * 7)) & 0xff;
    new_buf[new_message_len - 7] = (message_len_bits >> (8 * 6)) & 0xff;
    new_buf[new_message_len - 6] = (message_len_bits >> (8 * 5)) & 0xff;
    new_buf[new_message_len - 5] = (message_len_bits >> (8 * 4)) & 0xff;
    new_buf[new_message_len - 4] = (message_len_bits >> (8 * 3)) & 0xff;
    new_buf[new_message_len - 3] = (message_len_bits >> (8 * 2)) & 0xff;
    new_buf[new_message_len - 2] = (message_len_bits >> (8 * 1)) & 0xff;
    new_buf[new_message_len - 1] = (message_len_bits >> (8 * 0)) & 0xff;

    /* for (size_t i = 0; i < new_message_len; i++) { */
    /*     if (i % 8 == 0 && i != 0) { */
    /*         printf("\n"); */
    /*     } */

    /*     printf("%02x ", (uint8_t)new_buf[i]); */
    /* } */
    /* printf("\n"); */

    return new_buf;
}

uint32_t k(size_t t) {
    if (0 <= t && t <= 19) {
        return k1;
    } else if (20 <= t && t <= 39) {
        return k2;
    } else if (40 <= t && t <= 59) {
        return k3;
    } else if (60 <= t && t <= 79) {
        return k4;
    }

    assert(false);
}

uint32_t f(uint32_t x, uint32_t y, uint32_t z, size_t t) {
    if (0 <= t && t <= 19) {
        // ch
        return (x & y) ^ (~x & z);
    } else if (20 <= t && t <= 39) {
        // Parity
        return x ^ y ^ z;
    } else if (40 <= t && t <= 59) {
        // Maj
        return (x & y) ^ (x & z) ^ (y & z);
    } else if (60 <= t && t <= 79) {
        // Parity
        return x ^ y ^ z;
    }

    assert(false);
}

void hash_string(const char *string) {
    size_t new_message_len;
    uint32_t *padded_message =
        (uint32_t *)pad_message(string, &new_message_len);
    new_message_len /= 4;
    for (size_t i = 0; i < new_message_len; i++) {
        padded_message[i] = htobe32(padded_message[i]);
    }

    uint32_t H[5] = {h0, h1, h2, h3, h4};

    for (size_t round = 0; round < new_message_len / 16; round++) {
        uint32_t w[80];
        for (size_t t = 0; t < 80; t++) {
            if (t != 0 && t % 8 == 0) {
                /* printf("\n"); */
            }
            if (t <= 15) {
                w[t] = padded_message[(round * 16) + t];
            } else {
                uint32_t tmp = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
                w[t] = rotl(tmp, 1);
            }

            /* printf("%08x ", w[t]); */
        }
        /* printf("\n"); */

        uint32_t a = H[0];
        uint32_t b = H[1];
        uint32_t c = H[2];
        uint32_t d = H[3];
        uint32_t e = H[4];
        uint32_t T;

        for (size_t t = 0; t < 80; t++) {
            T = rotl(a, 5) + f(b, c, d, t) + e + k(t) + w[t];
            e = d;
            d = c;
            c = rotl(b, 30);
            b = a;
            a = T;
        }

        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;

        /* printf("%08x%08x%08x%08x%08x\n", H[0], H[1], H[2], H[3], H[4]); */
    }
    printf("%08x%08x%08x%08x%08x\n", H[0], H[1], H[2], H[3], H[4]);

    free(padded_message);
}

int main() {
    hash_string("This is a test of SHA-1.");
    hash_string("Kerckhoff's principle is the foundation on which modern "
                "cryptography is built.");
    hash_string("SHA-1 is no longer considered a secure hashing algorithm.");
    hash_string("SHA-2 or SHA-3 should be used in place of SHA-1.");
    hash_string("Never roll your own crypto!");
}
