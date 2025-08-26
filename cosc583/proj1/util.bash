#!/usr/bin/env bash

set -euo pipefail

function ffAdd {
    local ffadd_a="${1}"
    local ffadd_b="${2}"
    local -n answer="${3}"

    answer=$((ffadd_a ^ ffadd_b))
}

function xtime {
    local a="${1}"
    local -n answer="${2}"

    mask=$((((a & 0x80) >> 7) * 0x1b))
    answer=$((((a << 1) ^ mask) & 0xff))
}

function ffMultiply {
    local ffmultiply_a="${1}"
    local ffmultiply_b="${2}"
    local -n answer="${3}"

    powers=("${ffmultiply_a}")

    for ((i = 0; i < 7; i++)); do
        mask=$((((powers[i] & 0x80) >> 7) * 0x1b))
        powers+=($((((powers[i] << 1) ^ mask) & 0xff)))
    done

    answer=0
    for ((i = 0; i < 8; i++)); do
        if ((ffmultiply_b & (1 << i))); then
            answer=$((answer ^ powers[i]))
        fi
    done
}
