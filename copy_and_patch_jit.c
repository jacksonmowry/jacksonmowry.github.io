#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// #include "cnp_stencils.h"
uint8_t *cnp_copy_load_int_reg1(uint8_t *stencil_start);
void cnp_patch_load_int_reg1(uint8_t *stencil_start, int value);
uint8_t *cnp_copy_load_int_reg2(uint8_t *stencil_start);
void cnp_patch_load_int_reg2(uint8_t *stencil_start, int value);
uint8_t *cnp_copy_add_int1_int2(uint8_t *stencil_start);
uint8_t *cnp_copy_return_int1(uint8_t *stencil_start);

typedef int (*jit_func)() __attribute__((preserve_none));

jit_func create_add_1_2() {
    // Allocate memory as read/write, to eventually switch it to read/execute
    uint8_t *codedata = mmap(NULL, 256, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    assert(codedata != MAP_FAILED);
    jit_func ret = (jit_func)codedata;

    uint8_t *load_int_reg1_location = codedata;
    codedata = cnp_copy_load_int_reg1(codedata);
    uint8_t *load_int_reg2_location = codedata;
    codedata = cnp_copy_load_int_reg2(codedata);
    codedata = cnp_copy_add_int1_int2(codedata);
    codedata = cnp_copy_return_int1(codedata);

    // Patch
    cnp_patch_load_int_reg1(load_int_reg1_location, -1);
    cnp_patch_load_int_reg2(load_int_reg1_location, 2);

    int rc = mprotect(ret, 256, PROT_READ | PROT_EXEC);
    if (rc) {
        perror("mprotect");
    }

    return ret;
}

int main() {
    jit_func add_1_2 = create_add_1_2();
    int result = add_1_2();
    printf("JIT output: 1+2=%d\n", result);
    return 0;
}
