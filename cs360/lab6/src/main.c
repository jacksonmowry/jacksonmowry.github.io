#include "tpool.h"

int main(void) {
    void* tp = thread_pool_init(11);

    thread_pool_hash(tp, "/home/jackson/jacksonmowry.github.io", 64);
    thread_pool_hash(tp, ".", 64);

    thread_pool_shutdown(tp);
}
