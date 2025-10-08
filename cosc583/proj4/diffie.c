#include <math.h>
#include <openssl/aes.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BN_CTX* ctx;

unsigned char* hex2bytes(const char* hex) {
    size_t len = strlen(hex);
    unsigned char* buf = malloc(len / 2);

    for (size_t i = 0; i < len; i += 2) {
        char tmp[3] = {0};
        tmp[0] = hex[i];
        tmp[1] = hex[i + 1];

        sscanf(tmp, "%hhx", buf + (i / 2));
    }

    return buf;
}

BIGNUM* fme_big(BIGNUM* val, BIGNUM* exp, BIGNUM* p) {
    BIGNUM* accumulator = BN_new();
    BN_one(accumulator);

    while (!BN_is_zero(exp)) {
        /* char* str = BN_bn2dec(exp); */
        /* printf("%s\n", str); */
        /* free(str); */

        if (BN_is_odd(exp)) {
            BN_mul(accumulator, accumulator, val, ctx);
            BN_mod(accumulator, accumulator, p, ctx);
        }
        BN_rshift1(exp, exp);
        BN_mul(val, val, val, ctx);
        BN_mod(val, val, p, ctx);
    }

    return accumulator;
}

double miller_rabin_big(BIGNUM* p, int64_t k) {
    BIGNUM* two = BN_new();
    BN_add(two, BN_value_one(), BN_value_one());
    int gt_2 = BN_cmp(p, two);
    if (gt_2 < 0 || !BN_is_odd(p)) {
        return false;
    }

    // Find 2^s * d + 1 = p
    BIGNUM* tmp = BN_new();
    BN_sub(tmp, p, BN_value_one());
    BIGNUM* s = BN_new();
    BN_one(s);
    BIGNUM* d = BN_new();
    BN_copy(d, tmp);

    BIGNUM* working = BN_new();
    BN_mul(working, s, d, ctx);
    BN_sub(working, tmp, working);
    while (!BN_is_zero(working)) {
        BN_lshift1(s, s);
        BN_div(d, NULL, tmp, s, ctx);
        BN_div(tmp, NULL, tmp, s, ctx);
    }

    BIGNUM* a = BN_new();
    for (int64_t i = 0; i < k; i++) {
        BN_rand_range(a, p);
        BN_add(a, a, BN_value_one());

        BIGNUM* res = fme_big(a, d, p);
        BIGNUM* abs = BN_new();
        BN_copy(abs, res);
        BN_add(abs, abs, BN_value_one());
        BN_add(abs, abs, BN_value_one());
        if (BN_is_one(res) || BN_is_one(abs)) {
            BN_free(abs);
            BN_free(res);
            continue;
        }

        BN_free(abs);

        BIGNUM* loop = BN_new();
        BN_zero(loop);
        int cmp = BN_cmp(loop, s);
        for (; cmp < -1; BN_add(loop, loop, BN_value_one())) {
            BIGNUM* x = fme_big(res, two, p);

            BIGNUM* abs = BN_new();
            BN_copy(abs, x);
            BN_add(abs, abs, BN_value_one());
            BN_add(abs, abs, BN_value_one());
            if (BN_is_one(x) || BN_is_one(abs)) {
                BN_free(x);
                BN_free(abs);
                goto end;
            }

            BN_copy(res, x);

            BN_free(abs);
            BN_free(x);
            BN_add(loop, loop, BN_value_one());
            cmp = BN_cmp(loop, s);
        }

        BN_free(loop);
        BN_free(res);
        BN_free(a);

        return 0;
    end:
        continue;
    }

    BN_free(two);
    BN_free(working);
    BN_free(a);
    BN_free(tmp);
    BN_free(s);
    BN_free(d);

    return 1 - (pow((double)1 / 4, k));
}

typedef struct {
    BIGNUM* s;
    BIGNUM* t;
} CoefficientsBig;

CoefficientsBig extended_euclidians_big(BIGNUM* a, BIGNUM* b) {
    if (BN_is_zero(a)) {
        BIGNUM* s = BN_new();
        BIGNUM* t = BN_new();
        BN_copy(s, a);
        BN_copy(t, b);
        return (CoefficientsBig){.s = s, .t = t};
    }

    BIGNUM* mod = BN_new();
    BN_mod(mod, b, a, ctx);
    CoefficientsBig c = extended_euclidians_big(mod, a);
    BN_free(mod);

    BIGNUM* s = BN_new();
    BIGNUM* t = BN_new();

    BN_div(s, NULL, b, a, ctx);
    BN_mul(s, s, c.s, ctx);
    BN_sub(s, c.t, s);

    BN_copy(t, c.s);
    BN_free(c.s);
    BN_free(c.t);
    return (CoefficientsBig){.s = s, .t = t};
}

int main() {
    ctx = BN_CTX_new();

    BIGNUM* a = BN_new();
    BIGNUM* b = BN_new();
    BIGNUM* c_b = BN_new();

    // Some initial testing is still here, the real computation begins once we get
    // to rand and safe_prime
    BIGNUM* result = fme_big(a, b, c_b);
    char* str = BN_bn2dec(result);

    CoefficientsBig cb = extended_euclidians_big(a, b);
    free(str);
    str = BN_bn2dec(cb.s);
    char* str2 = BN_bn2dec(cb.t);

    BIGNUM* two = BN_new();
    BN_add(two, BN_value_one(), BN_value_one());

    BIGNUM* rand = BN_new();
    BIGNUM* safe_prime = BN_new();
    // This would be uncommented to actually generate our safe prime, but it is slow so I don't always regenerate it
    /* while (true) { */
    /*     BN_rand(rand, 1024, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY); */
    /*     while (miller_rabin_big(rand, 128) < 0.99) { */
    /*         str = BN_bn2dec(rand); */

    /*         BN_rand(rand, 1024, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY); */
    /*     } */
    /*     BN_copy(safe_prime, rand); */
    /*     BN_mul(safe_prime, safe_prime, two, ctx); */
    /*     BN_add(safe_prime, safe_prime, BN_value_one()); */
    /*     if (miller_rabin_big(safe_prime, 128) > 0.99) { */
    /*         break; */
    /*     } */
    /* } */
    str = BN_bn2dec(rand);
    printf("rand: %s\n", str);
    free(str);
    str = BN_bn2dec(safe_prime);
    printf("safe: %s\n", str);
    free(str);

    BN_dec2bn(&b, "5");
    BN_rand(a, 8, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
    str = BN_bn2dec(a);
    printf("a: %s\n", str);
    free(str);
    BIGNUM* public_key = fme_big(b, a, safe_prime);

    str = BN_bn2dec(public_key);
    printf("public key: %s\n", str);
    free(str);

    // These are now hard coded as I can't change the values once we pass the first part of the pass-off server
    BIGNUM* p = BN_new();
    BIGNUM* ga = BN_new();
    BN_dec2bn(&p,
              "2258866767280561571182544383691532301640219211462819610680721553"
              "4984559967592434788903061766968527904355320758976782353753434323"
              "3563012415545891817069934959509132276106178205850839859123373899"
              "0877658683313195699591738245913863646266155748777027789394995139"
              "59031543578283805539147163714117192893242410140443279");
    BN_dec2bn(&ga, "1");
    BN_dec2bn(&a, "177");

    BIGNUM* server_public = BN_new();
    BN_dec2bn(&server_public,
              "1763456430781931171080274549971432907705384857885697938938413972"
              "3323255114556760435445661155312821851789888391488038086713389993"
              "2715529167428237061440399752566926124382287290482432364370463180"
              "8380844241790657114175621103360788974598590001648453525343368421"
              "93568509522753013713141943588766846366836509921300623");
    BIGNUM* shared_key = fme_big(server_public, a, p);
    str = BN_bn2dec(shared_key);
    printf("shared_key: %s\n", str);

    // at this point I was done messing with the OpenSSL library and just moved over to python lol
    // So I take the shared key and have it hardcoded in the decrypt.py file to sha256 hash it
    // I take the first 16 bytes as the key

    BN_CTX_free(ctx);
    BN_free(a);
    BN_free(b);
    BN_free(c_b);
    BN_free(result);
    BN_free(cb.s);
    BN_free(cb.t);
    BN_free(safe_prime);
    BN_free(two);
    BN_free(public_key);
    free(str2);
}
