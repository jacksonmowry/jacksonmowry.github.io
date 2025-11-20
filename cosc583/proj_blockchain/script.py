#!/usr/bin/env python3

from hashlib import sha256
import binascii

def hexstr_to_int(h):
    return int.from_bytes(bytes.fromhex(h), byteorder='big')

def int_to_be(i):
    return i.to_bytes((i.bit_length()+7)//8, byteorder='big')

def str_encode(s):
    return s.encode('ascii')

def count_leading_zero_bits(byte_array):
    binary_string = ''.join(format(byte, '08b') for byte in byte_array)

    return len(binary_string) - len(binary_string.lstrip('0'))

genesis_block = '3a240c9b8e9f3c09989ceef5b537e57c440f8144dd9d962f424d983a37e07293'

# Quote 1
quote_1 = 'Only make new mistakes. -- Phil Dourado'
nonce_1 = 2428597
block_1 = '0000008b171c0c7902771cf1a1de580d2696cd7a02fa8d45743f9d3835183a56'

# Quote 2
quote_2 = 'For the things we have to learn before we can do them, we learn by doing them. -- Aristotle.'
nonce_2 = 11936780
block_2 = '000000ae1a89cc6ecdbdc33957d05c013c7ca5d74965d733ce99527e01f85ad0'

# Quote 3
quote_3 = 'Premature abstraction is an equally grevious sin as premature optimization. -- Keith Devens'
nonce_3 = 11668658
block_3 = '000000324573fd068fe621fd187451ba2fbca5f7ec06b365a5b21a36449ddd2a'

# Quote 4
quote_4 = 'Argue with idiots, and you become an idiot. If you compete with slaves you become a slave. -- Paul Graham and Norbert Weiner, respectively'

nonce = 0

while True:
    x = sha256(bytes.fromhex(block_3) + int_to_be(nonce) + str_encode(quote_4)).digest()
    leading_zero_bits = count_leading_zero_bits(x)
    if (leading_zero_bits >= 24):
        print(f"Nonce: {nonce}")
        print(f"block hash {sha256(bytes.fromhex(block_3) + int_to_be(nonce) + str_encode(quote_4)).hexdigest()}")
        break
    nonce += 1
