#include <stdio.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

static void test(void) {
  for (int i = 0; i < 1000000000000000000UL; i++)
    ;
}

int main() {
  struct rlimit rl;
  getrlimit(RLIMIT_CPU, &rl);

  printf("%zu %zu\n", rl.rlim_cur, rl.rlim_max);

  rl.rlim_cur = 2;
  setrlimit(RLIMIT_CPU, &rl);

  getrlimit(RLIMIT_CPU, &rl);
  printf("%zu %zu\n", rl.rlim_cur, rl.rlim_max);

  test();
  puts("Finish test.\n");
}
