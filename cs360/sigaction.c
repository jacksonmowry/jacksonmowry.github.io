#include <signal.h>
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>

/* int i = 0; */
/* int j = 22; */
int i;

/* static void handleaction(int sig, siginfo_t* info, void* context) { */
/*     ucontext_t* uc = (ucontext_t*)context; */
/*     printf("signal %d caught, context %p\n", sig, context); */
/*     i = 42; */

/*     for (int i = 0; */
/*          i < (sizeof(uc->uc_mcontext.gregs) /
 * sizeof(uc->uc_mcontext.gregs[0])); */
/*          i++) { */
/*         printf("    %-2u: %016llx   %lld\n", i, uc->uc_mcontext.gregs[i], */
/*                uc->uc_mcontext.gregs[i]); */
/*     } */
/* } */

int main() {
    struct sigaction sa;

    ucontext_t uc;
    getcontext(&uc);
    i++;
    printf("i = %d\n", i);
    usleep(i * 10000);
    setcontext(&uc);

    /* sigemptyset(&sa.sa_mask); */
    /* sa.sa_flags = SA_SIGINFO; */
    /* sa.sa_sigaction = handleaction; */

    /* sigaction(SIGFPE, &sa, NULL); */
    /* printf("%d\n", j / i); */
}
