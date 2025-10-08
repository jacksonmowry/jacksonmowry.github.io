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

BIGNUM* gcd_big(BIGNUM* a, BIGNUM* b) {
    BIGNUM* tmp_a = BN_dup(a);
    BIGNUM* tmp_b = BN_dup(b);
    BIGNUM* t = BN_new();
    while (!BN_is_zero(tmp_b)) {
        BN_copy(t, tmp_b);
        BN_mod(tmp_b, tmp_a, tmp_b, ctx);
        BN_copy(tmp_a, t);
    }

    BN_free(tmp_b);
    return tmp_a;
}

int main() {
    ctx = BN_CTX_new();

    BIGNUM* p = BN_new();
    BIGNUM* q = BN_new();
    BIGNUM* n = BN_new();
    BIGNUM* phi_n = BN_new();
    BIGNUM* e = BN_new();

    BN_dec2bn(&e, "65537");

    size_t trials = 0;
    while (true) {
        printf("Trials: %zu\n", trials++);
        BN_rand(p, 1024, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        while (miller_rabin_big(p, 128) < 0.99) {
            BN_rand(p, 1300, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        }

        BN_rand(q, 1024, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        while (miller_rabin_big(q, 128) < 0.99) {
            BN_rand(q, 1300, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        }

        BN_mul(n, p, q, ctx);

        BIGNUM* a = BN_dup(p);
        BIGNUM* b = BN_dup(q);
        BN_sub(a, a, BN_value_one());
        BN_sub(b, b, BN_value_one());

        BN_mul(phi_n, a, b, ctx);

        BN_free(a);
        BN_free(b);

        // Now check if phi_n is relatively prime to e = 65537
        BIGNUM* result = gcd_big(phi_n, e);
        if (BN_is_one(result)) {
            break;
        }
    }

    printf("Our two primes are:\n");
    char* str = BN_bn2dec(p);
    puts(str);
    free(str);
    str = BN_bn2dec(q);
    puts(str);
    free(str);

    str = BN_bn2dec(n);
    printf("n: %s\n", str);
    free(str);

    str = BN_bn2dec(phi_n);
    printf("phi_n: %s\n", str);
    free(str);

    CoefficientsBig c = extended_euclidians_big(e, phi_n);

    if (BN_is_negative(c.s)) {
        BN_add(c.s, c.s, phi_n);
    }

    char* s = BN_bn2dec(c.s);
    char* t = BN_bn2dec(c.t);
    printf("Two coefficients: s: %s, t: %s\n", s, t);
    free(s);
    free(t);

    BIGNUM* d = BN_new();
    BN_dec2bn(
        &d, "175611539263570731061665694104756779990427053232717150776491801178"
            "014307908853100289234858686134319940651191979410808907634887751207"
            "494374019766866423771588056923971443604575016362121113548074754898"
            "994718435277520722642641403626148497083761717510587152605901224252"
            "537740689956002300180486990497703469412394285678965569499458072428"
            "190713801119988834435315084452599105646586839043474439805447805541"
            "888052281703687904274010877547896903863938175382065054753833459407"
            "151595827684105361400700562536381385324275946687820179448788848539"
            "057356200270633494377084098581816314791981613203225478891474980015"
            "939271919148837483034816219709319877687440502520120797132692486502"
            "594249208277511908686109434560978686838558830378602899675584197316"
            "966426403562391126221848021814190506360467178447909365137");

    BN_dec2bn(
        &phi_n,
        "1952209086527909048001557925593419460975102247127011553149734229026617"
        "9898603157773273374043683524317180271345039600677442866707179980253740"
        "4306088218182287317003533543093140971729218228086307548458381396717657"
        "5444516034669347193895927906247528664950676855336115248934069206100777"
        "3428314577291189728050480178275506948685900844205187662941249110676531"
        "5114236349865786660114015510265365565180188482523955666893079251321468"
        "4479739545983059829053162533075516840829997369836781038303265277625397"
        "2498603741895499403059454999720586841782070687819548304306732300723880"
        "4760025139922388638399601563859370095533191390870206019146293569387943"
        "5492381893283094731771426449082423467510751314238145257999936028123509"
        "6227680093798955183298062153592100977488257351835759754446862325450245"
        "0621512298792");

    BN_dec2bn(
        &n, "195220908652790904800155792559341946097510224712701155314973422902"
            "661798986031577732733740436835243171802713450396006774428667071799"
            "802537404306088218182287317003533543093140971729218228086307548458"
            "381396717657544451603466934719389592790624752866495067685533611524"
            "893406920610077734283145772911897280504801782755069486859008442051"
            "876629412491106765315114236349865786660114015510265365565180218611"
            "702635407587216549570242734313908734076761310554291479204300374588"
            "941439473042502155646904308750649208944573687550104898129361564041"
            "896021910778223816593904956267594945356417095738515280589324753696"
            "183301050304181676898990319917134505549713376653585744219461340153"
            "312450269549193323910655871597576552740090833367560759892527479323"
            "778900050367856949300683530759538473190499432913059629831");

    // Encrypt pannicle
    char* plaintext = "pannicle";
    BIGNUM* working = BN_new();
    BN_zero(working);

    for (size_t i = 0; i < strlen(plaintext); i++) {
        BN_lshift(working, working, 8);
        BN_add_word(working, plaintext[i]);
    }

    str = BN_bn2hex(working);
    printf("working: %s\n", str);
    free(str);

    BN_dec2bn(&e, "65537");

    BIGNUM* result = fme_big(working, e, n);
    str = BN_bn2dec(result);
    printf("cipher: ");
    puts(str);
    free(str);
    BN_free(result);

    BIGNUM* cipher_text = BN_new();
    BN_dec2bn(
        &cipher_text,
        "8482123970139556829265309956872967603634512306270860377414306322437647"
        "1199886064562595116626921932403610920688622022665152291083470970929078"
        "4225461046522802225258196685663166306073721029907373797822947330850798"
        "5324725494234304623563759990179514184608843224160410520569011342988243"
        "2012380940748584062325745642590092266200154455368338181970711697117128"
        "3395553935106183640923570750524424167470813421154051503688029733505425"
        "4612063239322265734171950628368442104391438927719489072671229194251957"
        "3977143799064700612575896280843318276755943464793079489815579940499280"
        "7860699612816053465262257286253242818566746039243441719323132616070715"
        "9030837428651625714961282462533883398732220301789056410607336127479602"
        "2861536375386036828193743513632180480331928421451284870644597661957181"
        "551710875960");

    result = fme_big(cipher_text, d, n);
    str = BN_bn2hex(result);
    puts(str);
    free(str);
    // Take this string and convert to ascii for submission

    free(e);
    BN_CTX_free(ctx);
}
