#include <linux/sched.h>
#include <sched.h>

int clone(int (*fn)(void *), void *stack, int flags, void *args, ...);

int main() {
    int child1_pid;
    int child2_pid;


}
