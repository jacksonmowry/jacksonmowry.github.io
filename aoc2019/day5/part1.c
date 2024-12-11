#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define imm_or_mem(num)                                                        \
  ((instruction.r##num##_mode == IMMEDIATE) ? vm.tape[vm.ip + num]             \
                                            : vm.tape[vm.tape[vm.ip + num]])

#define binary_op(operand)                                                     \
  do {                                                                         \
    assert(instruction.r3_mode == LOCATION);                                   \
    int32_t rs1 = imm_or_mem(1);                                               \
    int32_t rs2 = imm_or_mem(2);                                               \
    vm.tape[vm.tape[vm.ip + 3]] = rs1 operand rs2;                             \
                                                                               \
    vm.ip += 4;                                                                \
  } while (false)

#define j_op(bang)                                                             \
  do {                                                                         \
    int32_t rs1 = imm_or_mem(1);                                               \
    vm.ip = bang rs1 ? imm_or_mem(2) : vm.ip + 3;                              \
  } while (false)

#define comparison(comparator)                                                 \
  do {                                                                         \
    assert(instruction.r3_mode == LOCATION);                                   \
    vm.tape[vm.tape[vm.ip + 3]] = imm_or_mem(1) comparator imm_or_mem(2);      \
    vm.ip += 4;                                                                \
  } while (false)

#define input()                                                                \
  do {                                                                         \
    int in;                                                                    \
    scanf("%d", &in);                                                          \
    vm.tape[vm.tape[vm.ip + 1]] = in;                                          \
                                                                               \
    vm.ip += 2;                                                                \
  } while (false)

#define output()                                                               \
  do {                                                                         \
    if (instruction.r1_mode == IMMEDIATE) {                                    \
      printf("%d\n", vm.tape[vm.ip + 1]);                                      \
    } else {                                                                   \
      printf("%d\n", vm.tape[vm.tape[vm.ip + 1]]);                             \
    }                                                                          \
                                                                               \
    vm.ip += 2;                                                                \
  } while (false)

typedef enum Opcode {
  ADD = 1,
  MUL = 2,
  INPUT = 3,
  OUPUT = 4,
  JTRUE = 5,
  JFALSE = 6,
  LT = 7,
  EQ = 8,
  HALT = 99
} Opcode_t;
typedef enum Parameter { LOCATION, IMMEDIATE } Parameter_t;

typedef struct Instruction {
  Opcode_t opcode;
  Parameter_t r1_mode;
  Parameter_t r2_mode;
  Parameter_t r3_mode;
} Instruction_t;

Instruction_t instruction_decode(int instruction) {
  return (Instruction_t){.opcode = instruction % 100,
                         .r1_mode = (instruction / 100) % 10,
                         .r2_mode = (instruction / 1000) % 10,
                         .r3_mode = (instruction / 10000) % 10};
}

typedef struct Intcode_VM {
  int32_t tape[1024];
  size_t tape_len;
  size_t ip;
  bool running;
} Intcode_VM;

int32_t vm_run(Intcode_VM vm) {
  while (vm.running) {
    const Instruction_t instruction = instruction_decode(vm.tape[vm.ip]);
    switch (instruction.opcode) {
    case ADD:
      binary_op(+);
      break;
    case MUL:
      binary_op(*);
      break;
    case INPUT:
      input();
      break;
    case OUPUT:
      output();
      break;
    case JTRUE:
      j_op();
      break;
    case JFALSE:
      j_op(!);
      break;
    case LT:
      comparison(<);
      break;
    case EQ:
      comparison(==);
      break;
    case HALT:
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

int main(int argc, char *argv[]) {
  Intcode_VM vm = {.running = true};

  int32_t tmp;

  FILE *fp = fopen(argv[1], "r");

  while (fscanf(fp, "%d", &tmp) == 1) {
    vm.tape[vm.tape_len++] = tmp;
    if (getc(fp) == EOF) {
      break;
    }
  }

  vm_run(vm);
  fclose(fp);
}
