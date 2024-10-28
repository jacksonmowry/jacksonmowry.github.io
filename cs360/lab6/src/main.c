#include "tpool.h"

int main(void) {
    void* tp = thread_pool_init(12);

    thread_pool_hash(tp, "/home/jackson/jacksonmowry.github.io", 64);
    thread_pool_hash(tp, ".", 32);
    thread_pool_hash(tp, "/bin", 64);
    thread_pool_hash(tp, "/usr/bin", 64);
    if (!thread_pool_hash(tp, "directory_that_doesn't exist", 64)) {
        printf("Failed to open dir!\n");
    }

    thread_pool_shutdown(tp);
}
