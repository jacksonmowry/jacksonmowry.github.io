.section .rodata
filename:    .asciz "/home/jackson/jacksonmowry.github.io/aoc2024/day1/asm/part1.input"
.section .text
# a0 = Exit Condition Number
exit:
    li a7, 93
    ecall


# INPUT
# a0 = char* filename
# OUPUT
# a0 = void* buffer
# a1 = size_t len
map_input:
    addi sp, sp, -32
    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)

    mv a1, a0
    li a0, 0
    li a2, 0
    li a7, 56               # read
    ecall
    mv s0, a0               # fd

    # Seek end
    li a1, 0
    li a2, 2
    li a7, 62
    ecall
    mv s1, a0               # len

    # Seek beg
    mv a0, s0
    li a1, 0
    li a2, 0
    li a7, 62
    ecall

    li a0, 0
    mv a1, s1
    li a2, 0x1
    li a3, 0x2
    mv a4, s0
    li a5, 0
    li a7, 222
    ecall                   # mmap

    mv a0, s0
    li a7, 57
    ecall

    mv a1, s1               # move len to return reg

    ld s1, 16(sp)
    ld s0, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 32

    addi a1, a1, -1

    ret

print_fname:
    addi sp, sp, -16
    sd ra, 0(sp)

    li a0, 1
    la a1, filename
    li a2, 53
    li a7, 64
    ecall

    ld ra, 0(sp)
    addi sp, sp, 16
    ret

.global _start
_start:
    # a0 now holds fd
    la a0, filename
    call print_fname

    la a0, filename
    call map_input

    mv a0, a1

    call exit
