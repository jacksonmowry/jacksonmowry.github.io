#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define binary_op(operand)                                                     \
  do {                                                                         \
    size_t rs1_loc = vm.tape[vm.ip + 1];                                       \
    size_t rs2_loc = vm.tape[vm.ip + 2];                                       \
    size_t rd_loc = vm.tape[vm.ip + 3];                                        \
    int32_t rs1 = vm.tape[rs1_loc];                                            \
    int32_t rs2 = vm.tape[rs2_loc];                                            \
                                                                               \
    vm.tape[rd_loc] = rs1 operand rs2;                                         \
                                                                               \
    vm.ip += 4;                                                                \
  } while (false)

typedef struct Intcode_VM {
  int32_t tape[256];
  size_t tape_len;
  size_t ip;
  bool running;
} Intcode_VM;

int32_t vm_run(Intcode_VM vm) {
  while (vm.running) {
    switch (vm.tape[vm.ip]) {
    case 1:
      binary_op(+);
      break;
    case 2:
      binary_op(*);
      break;
    case 99:
      vm.running = false;
      break;
    default:
      fprintf(stderr, "Invalid Opcode: %d\n", vm.tape[vm.ip]);
      vm.running = false;
      break;
    }
  }

  return vm.tape[0];
}

int main() {
  Intcode_VM vm = {.running = true};

  int32_t tmp;

  while (scanf("%d", &tmp) == 1) {
    vm.tape[vm.tape_len++] = tmp;
    if (getc(stdin) == EOF) {
      break;
    }
  }

  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      Intcode_VM copy = vm;
      copy.tape[1] = i;
      copy.tape[2] = j;

      if (vm_run(copy) == 19690720) {
        printf("Noun: %d, Verb: %d, 100 * noun + verb = %d\n", i, j,
               100 * i + j);
        exit(0);
      }
    }
  }

  printf("Never found!\n");
}
