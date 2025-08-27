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

function InvMixColumns {
    # 1D array of 16 elements
    local -n mixcolumns_state="${1}"
    local copy_state=(0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    local -i a=0
    local -i b=0
    local -i c=0
    local -i d=0

    # 0e 0b 0d 09
    # 09 0e 0b 0d
    # 0d 09 0e 0b
    # 0b 0d 09 0e

    for ((col = 0; col < 4; col++)); do
        # row 0
        ffMultiply 0x0e "${mixcolumns_state[0 + col]}" a
        ffMultiply 0x0b "${mixcolumns_state[4 + col]}" b
        ffMultiply 0x0d "${mixcolumns_state[8 + col]}" c
        ffMultiply 0x09 "${mixcolumns_state[12 + col]}" d

        copy_state[0 + col]=$((a ^ b ^ c ^ d))

        # row 1
        ffMultiply 0x09 "${mixcolumns_state[0 + col]}" a
        ffMultiply 0x0e "${mixcolumns_state[4 + col]}" b
        ffMultiply 0x0b "${mixcolumns_state[8 + col]}" c
        ffMultiply 0x0d "${mixcolumns_state[12 + col]}" d

        copy_state[4 + col]=$((a ^ b ^ c ^ d))

        # row 2
        ffMultiply 0x0d "${mixcolumns_state[0 + col]}" a
        ffMultiply 0x09 "${mixcolumns_state[4 + col]}" b
        ffMultiply 0x0e "${mixcolumns_state[8 + col]}" c
        ffMultiply 0x0b "${mixcolumns_state[12 + col]}" d

        copy_state[8 + col]=$((a ^ b ^ c ^ d))

        # row 3
        ffMultiply 0x0b "${mixcolumns_state[0 + col]}" a
        ffMultiply 0x0d "${mixcolumns_state[4 + col]}" b
        ffMultiply 0x09 "${mixcolumns_state[8 + col]}" c
        ffMultiply 0x0e "${mixcolumns_state[12 + col]}" d

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
        ark_state[(0 * Nb) + ark_col]=$((ark_state[(0 * Nb) + ark_col] ^ (ark_w[(round) * Nb + ark_col] >> 24) & 0xFF))
        ark_state[(1 * Nb) + ark_col]=$((ark_state[(1 * Nb) + ark_col] ^ (ark_w[(round) * Nb + ark_col] >> 16) & 0xFF))
        ark_state[(2 * Nb) + ark_col]=$((ark_state[(2 * Nb) + ark_col] ^ (ark_w[(round) * Nb + ark_col] >> 8) & 0xFF))
        ark_state[(3 * Nb) + ark_col]=$((ark_state[(3 * Nb) + ark_col] ^ (ark_w[(round) * Nb + ark_col] >> 0) & 0xFF))
    done
}

function SubBytes {
    local -n sb_state="${1}"

    for ((i = 0; i < 16; i++)); do
        sb_state[i]="${sbox[sb_state[i]]}"
    done
}

function InvSubBytes {
    local -n sb_state="${1}"

    for ((i = 0; i < 16; i++)); do
        sb_state[i]="${isbox[sb_state[i]]}"
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

function InvShiftRows {
    local -n sr_state="${1}"

    # row 1
    local -a copy=("${sr_state[@]:4:8}")
    sr_state[4]=$((copy[3]))
    sr_state[5]=$((copy[0]))
    sr_state[6]=$((copy[1]))
    sr_state[7]=$((copy[2]))
    # row 2
    copy=("${sr_state[@]:8:12}")
    sr_state[8]=$((copy[2]))
    sr_state[9]=$((copy[3]))
    sr_state[10]=$((copy[0]))
    sr_state[11]=$((copy[1]))
    # row 3
    copy=("${sr_state[@]:12:16}")
    sr_state[12]=$((copy[1]))
    sr_state[13]=$((copy[2]))
    sr_state[14]=$((copy[3]))
    sr_state[15]=$((copy[0]))
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

declare -ra Rcon=(0x00000000
    0x01000000 0x02000000 0x04000000 0x08000000
    0x10000000 0x20000000 0x40000000 0x80000000
    0x1B000000 0x36000000 0x6C000000 0xD8000000
    0xAB000000 0x4D000000 0x9A000000 0x2F000000
    0x5E000000 0xBC000000 0x63000000 0xC6000000
    0x97000000 0x35000000 0x6A000000 0xD4000000
    0xB3000000 0x7D000000 0xFA000000 0xEF000000
    0xC5000000 0x91000000 0x39000000 0x72000000
    0xE4000000 0xD3000000 0xBD000000 0x61000000
    0xC2000000 0x9F000000 0x25000000 0x4A000000
    0x94000000 0x33000000 0x66000000 0xCC000000
    0x83000000 0x1D000000 0x3A000000 0x74000000
    0xE8000000 0xCB000000 0x8D000000)

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
        elif ((ke_nk > 6 && (ke_i % ke_nk) == 4)); then
            local -i ke_first=0
            subWord $((ke_temp)) ke_first
            ke_temp=$((ke_first))
        fi

        w[ke_i]=$((w[ke_i - ke_nk] ^ ke_temp))
    done
}

function Cipher {
    local -n c_in="${1}"
    local -n c_out="${2}"
    local -n c_w="${3}"
    local -i c_nk="${4}"

    local -a state=("${c_in[@]}")
    transpose state

    local -i rounds=0
    Nr $((c_nk)) rounds

    AddRoundKey state c_w 0

    for ((c_round = 1; c_round < rounds; c_round++)); do
        SubBytes state
        ShiftRows state
        MixColumns state
        AddRoundKey state c_w $((c_round))
    done

    SubBytes state
    ShiftRows state
    AddRoundKey state c_w $((rounds))

    c_out=("${state[@]}")
    transpose c_out
}

function ICipher {
    local -n ic_in="${1}"
    local -n ic_out="${2}"
    local -n ic_w="${3}"
    local -i ic_nk="${4}"

    local -a state=("${ic_in[@]}")
    transpose state

    local -i rounds=0
    Nr $((ic_nk)) rounds

    AddRoundKey state ic_w rounds

    for ((ic_round = rounds - 1; ic_round >= 1; ic_round--)); do
        InvShiftRows state
        InvSubBytes state
        AddRoundKey state ic_w $((ic_round))
        InvMixColumns state
    done

    InvShiftRows state
    InvSubBytes state
    AddRoundKey state ic_w $((0))

    ic_out=("${state[@]}")
    transpose ic_out
}
