#!/usr/bin/env python3
import copy


def ffMultiply(a, b):
    powers = [a]

    for i in range(0,7):
        mask = ((powers[i] & 0x80) >> 7) * 0x1b
        powers.append(((powers[i] << 1) ^ mask) & 0xff)

    # for val in powers:
    #     print(hex(val), end=',')
    # print("")

    answer = 0
    for i in range(0,8):
        if b & (1 << i):
            answer ^= powers[i]

    return answer

print(hex(ffMultiply(0x57, 0x13)))

def MixColumns(state):
    answer = [[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
    for col in range(0, 4):
        # row 0
        answer[0][col] = ffMultiply(2, state[0][col]) ^ ffMultiply(3, state[1][col]) ^ state[2][col] ^ state[3][col]
        # row 1
        answer[1][col] = state[0][col] ^ ffMultiply(2, state[1][col]) ^ ffMultiply(3, state[2][col]) ^ state[3][col]
        # row 2
        answer[2][col] = state[0][col] ^ state[1][col] ^ ffMultiply(2, state[2][col]) ^ ffMultiply(3, state[3][col])
        # row 3
        answer[3][col] = ffMultiply(3, state[0][col]) ^ state[1][col] ^ state[2][col] ^ ffMultiply(2, state[3][col])

    return answer

state = [ [0xd4, 0xe0, 0xb8, 0x1e],
                         [0xbf, 0xb4, 0x41, 0x27],
                         [0x5d, 0x52, 0x11, 0x98],
                         [0x30, 0xae, 0xf1, 0xe5]]

mix =    [ [0x04, 0xe0, 0x48, 0x28],
                         [0x66, 0xcb, 0xf8, 0x06],
                         [0x81, 0x19, 0xd3, 0x26],
                         [0xe5, 0x9a, 0x7a, 0x4c]]

print(MixColumns(state) == mix)

answer = MixColumns(state)

for row in answer:
    for col in row:
        print(hex(col), end=',')
    print("")
print("")
