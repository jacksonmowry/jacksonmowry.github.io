#!/usr/bin/env bash

set -euo pipefail

source util.bash

function MixColumns {
    # 1D array of 16 elements
    local -n mixcolumns_state="${1}"
    local copy_state=(0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    local -i a=0
    local -i b=0
    local -i c=0
    local -i d=0

    for ((col = 0; col < 4; col++)); do
        # row 0
        ffMultiply 2 "${mixcolumns_state[0 + col]}" a
        ffMultiply 3 "${mixcolumns_state[4 + col]}" b
        c="${mixcolumns_state[8 + col]}"
        d="${mixcolumns_state[12 + col]}"

        copy_state[0 + col]=$((a ^ b ^ c ^ d))

        # row 1
        a="${mixcolumns_state[0 + col]}"
        ffMultiply 2 "${mixcolumns_state[4 + col]}" b
        ffMultiply 3 "${mixcolumns_state[8 + col]}" c
        d="${mixcolumns_state[12 + col]}"

        copy_state[4 + col]=$((a ^ b ^ c ^ d))

        # row 2
        a="${mixcolumns_state[0 + col]}"
        b="${mixcolumns_state[4 + col]}"
        ffMultiply 2 "${mixcolumns_state[8 + col]}" c
        ffMultiply 3 "${mixcolumns_state[12 + col]}" d

        copy_state[8 + col]=$((a ^ b ^ c ^ d))

        # row 3
        ffMultiply 3 "${mixcolumns_state[0 + col]}" a
        b="${mixcolumns_state[4 + col]}"
        c="${mixcolumns_state[8 + col]}"
        ffMultiply 2 "${mixcolumns_state[12 + col]}" d

        copy_state[12 + col]=$((a ^ b ^ c ^ d))
    done

    mixcolumns_state=()
    mixcolumns_state=("${copy_state[@]}")
}

function AddRoundKey {
    local -n ark_state="${1}"
    local -n ark_w="${2}"
    local -i round="${3}"

    for ((ark_col = 0; ark_col < 4; ark_col++)); do
        ark_state[(0 * Nb) + ark_col]=$(((ark_w[round * Nb + ark_col] >> 24) & 0xFF))
        ark_state[(1 * Nb) + ark_col]=$(((ark_w[round * Nb + ark_col] >> 16) & 0xFF))
        ark_state[(2 * Nb) + ark_col]=$(((ark_w[round * Nb + ark_col] >> 8) & 0xFF))
        ark_state[(3 * Nb) + ark_col]=$(((ark_w[round * Nb + ark_col] >> 0) & 0xFF))
    done
}

function SubBytes {
    local -n sb_state="${1}"

    for ((i = 0; i < 16; i++)); do
        sb_state[i]="${sbox[sb_state[i]]}"
    done
}

function subWord {
    local -i sw_word="${1}"
    local -n sw_answer="${2}"

    sw_answer=0
    for ((i = 0; i < 4; i++)); do
        local -i sw_result=0
        SubByte $(((sw_word >> (i * 8)) & 0xFF)) sw_result
        sw_answer=$((sw_answer | (sw_result << (i * 8))))
    done
}

function ShiftRows {
    local -n sr_state="${1}"

    # row 1
    local -a copy=("${sr_state[@]:4:8}")
    sr_state[4]=$((copy[1]))
    sr_state[5]=$((copy[2]))
    sr_state[6]=$((copy[3]))
    sr_state[7]=$((copy[0]))
    # row 2
    copy=("${sr_state[@]:8:12}")
    sr_state[8]=$((copy[2]))
    sr_state[9]=$((copy[3]))
    sr_state[10]=$((copy[0]))
    sr_state[11]=$((copy[1]))
    # row 3
    copy=("${sr_state[@]:12:16}")
    sr_state[12]=$((copy[3]))
    sr_state[13]=$((copy[0]))
    sr_state[14]=$((copy[1]))
    sr_state[15]=$((copy[2]))
}

function rotWord {
    local -i rw_word="${1}"
    local -n rw_answer="${2}"

    rw_answer=$((((rw_word << 8) & 0xFF000000) | ((rw_word << 8) & 0xFF0000) | ((rw_word << 8) & 0xFF00) | ((rw_word & 0xFF000000) >> 24)))
}

declare -ri Nb=4

function Nr {
    local -i nr_nk="${1}"
    local -n nr_answer="${2}"

    nr_answer=$((10 + (nr_nk - 4)))
}

declare -ra Rcon=(0x0 0x01000000 0x02000000 0x04000000 0x08000000 0x10000000 0x20000000 0x40000000 0x80000000 0x1B000000 0x36000000)

function KeyExpansion {
    local -n ke_key="${1}"
    local -n ke_w="${2}"
    local -i ke_nk="${3}"

    local -i ke_temp=0
    local -i ke_nr=0
    Nr ke_nk ke_nr

    for ((i = 0; i < ke_nk; i++)); do
        ke_w[i]=$(((ke_key[4 * i] << 24) | (ke_key[4 * i + 1] << 16) | (ke_key[4 * i + 2] << 8) | ke_key[4 * i + 3]))
    done

    for ((ke_i = ke_nk; ke_i < Nb * (ke_nr + 1); ke_i++)); do
        ke_temp=$((ke_w[ke_i - 1]))
        if ((ke_i % ke_nk == 0)); then
            local -i ke_first=0
            local -i ke_second=0
            rotWord $((ke_temp)) ke_first
            subWord $((ke_first)) ke_second
            ke_temp=$((ke_second ^ Rcon[ke_i / ke_nk]))
        elif ((ke_nk > 6 && (ke_i % 4) == 4)); then
            local -i ke_first=0
            subWord $((ke_temp)) ke_first
            ke_temp=$((ke_first))
        fi

        w[ke_i]=$((w[ke_i - ke_nk] ^ ke_temp))
    done
}
