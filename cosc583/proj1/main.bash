#!/usr/bin/env bash

source cipher.bash
source testing.bash

function splitInput {
    local si_input="${1}"
    local -n si_output="${2}"

    si_output=()

    local -i input_len=${#si_input}
    for ((i = 0; i < input_len / 2; i++)); do
        si_output[i]="0x${si_input[*]:0:2}"
        si_input="${si_input:2}"
    done
}

function encrypt {
    local e_input="${1}"
    local e_key="${2}"
    local e_expected="${3}"

    declare -a in_bytes
    declare -a key_bytes
    declare -a expected_bytes

    splitInput "${input}" in_bytes
    splitInput "${key}" key_bytes
    splitInput "${expected}" expected_bytes

    w=({1..44})
    out=({1..16})

    local -i key_len="${#key_bytes[@]}"
    local -i e_Nk=$((key_len / 4))

    KeyExpansion key_bytes w $((e_Nk))

    Cipher in_bytes out w $((e_Nk))
    expect_equal_int_array expected_bytes out
}

function decrypt {
    local d_input="${1}"
    local d_key="${2}"
    local d_expected="${3}"

    declare -a d_in_bytes=()
    declare -a d_key_bytes=()
    declare -a d_expected_bytes=()

    splitInput "${d_input}" d_in_bytes
    splitInput "${d_key}" d_key_bytes
    splitInput "${d_expected}" d_expected_bytes

    w=({1..44})
    out=({1..16})

    local -i key_len="${#d_key_bytes[@]}"
    local -i d_Nk=$((key_len / 4))

    KeyExpansion d_key_bytes w $((d_Nk))

    ICipher d_in_bytes out w $((d_Nk))
    expect_equal_int_array d_expected_bytes out
}

input='00112233445566778899aabbccddeeff'
key='000102030405060708090a0b0c0d0e0f'
expected='69c4e0d86a7b0430d8cdb78070b4c55a'

encrypt "${input}" "${key}" "${expected}"
decrypt "${expected}" "${key}" "${input}"

echo "Example 1 passed"

input='00112233445566778899aabbccddeeff'
key='000102030405060708090a0b0c0d0e0f1011121314151617'
expected='dda97ca4864cdfe06eaf70a0ec0d7191'

encrypt "${input}" "${key}" "${expected}"
decrypt "${expected}" "${key}" "${input}"

echo "Example 2 passed"

input='00112233445566778899aabbccddeeff'
key='000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f'
expected='8ea2b7ca516745bfeafc49904b496089'

encrypt "${input}" "${key}" "${expected}"
decrypt "${expected}" "${key}" "${input}"

echo "Example 3 passed"
