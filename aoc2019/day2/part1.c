#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define binary_op(operand)                                                     \
  do {                                                                         \
    size_t rs1_loc = tape[instruction_pointer + 1];                            \
    size_t rs2_loc = tape[instruction_pointer + 2];                            \
    size_t rd_loc = tape[instruction_pointer + 3];                             \
    int32_t rs1 = tape[rs1_loc];                                               \
    int32_t rs2 = tape[rs2_loc];                                               \
                                                                               \
    tape[rd_loc] = rs1 operand rs2;                                            \
                                                                               \
    instruction_pointer += 4;                                                  \
  } while (false)

int main() {
  int32_t tape[256];
  size_t tape_len = 0;

  int32_t tmp;

  while (scanf("%d", &tmp) == 1) {
    tape[tape_len++] = tmp;
    if (getc(stdin) == EOF) {
      break;
    }
  }

  size_t instruction_pointer = 0;
  tape[1] = 12;
  tape[2] = 2;

  bool running = true;
  while (running) {
    switch (tape[instruction_pointer]) {
    case 1:
      binary_op(+);
      break;
    case 2:
      binary_op(*);
      break;
    case 99:
      running = false;
      break;
    default:
      fprintf(stderr, "Invalid Opcode: %d\n", tape[instruction_pointer]);
      running = false;
      break;
    }
  }

  printf("%d\n", tape[0]);
}
