#include "tokenizer.h"
#include "allocator.h"
#include "fat_pointer.h"
#include "vector.h"

/* Tokenizer tokenizer_init(Allocator allocator, FILE* input) { */
/*     int* p = (int*)allocator.vtable.allocate_many_virtual(allocator, 4, 4);
 */
/*     struct { */
/*         FatPointer(float) fat_ptr; */
/*         size_t size; */
/*         Allocator allocator; */
/*     } y = vector_init(float, allocator, 5.0); */
/*     /\* Vector(int) v = vector_init(int, allocator, 5); *\/ */
/*     /\* vector_pb(a, 5); *\/ */
/*     /\*     return (Tokenizer){ *\/ */
/*     /\*     .input = input, *\/ */
/*     /\*     .tokens = *\/ */
/*     /\* } *\/ */
/* } */

Tokenizer tokenizer_init(Allocator allocator, FILE* input) {
    /* int* p = (int*)allocator.vtable.allocate_many_virtual(allocator, 4, 4);
     */
    auto y = (struct {
        struct {
            float* ptr;
            size_t len;
        } fat_ptr;
        size_t size;
        Allocator allocator;
    }){
        .fat_ptr = (struct {
            float* ptr;
            size_t len;
        }){.ptr = (float*)allocator.vtable.allocate_many_virtual(allocator,
                                                                 sizeof(float),
                                                                 5.0),
           .len = 5.0},
        .size = 0,
        .allocator = allocator,
    };
}
