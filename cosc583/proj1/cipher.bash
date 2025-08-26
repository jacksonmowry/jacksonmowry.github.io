#!/usr/bin/env bash

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
        ffMultiply 2 "${mixcolumns_state[$((0 + col))]}" a
        ffMultiply 3 "${mixcolumns_state[$((4 + col))]}" b
        c="${mixcolumns_state[$((8 + col))]}"
        d="${mixcolumns_state[$((12 + col))]}"

        copy_state[$((0 + col))]=$((a ^ b ^ c ^ d))

        # row 1
        a="${mixcolumns_state[$((0 + col))]}"
        ffMultiply 2 "${mixcolumns_state[$((4 + col))]}" b
        ffMultiply 3 "${mixcolumns_state[$((8 + col))]}" c
        d="${mixcolumns_state[$((12 + col))]}"

        copy_state[$((4 + col))]=$((a ^ b ^ c ^ d))

        # row 2
        a="${mixcolumns_state[$((0 + col))]}"
        b="${mixcolumns_state[$((4 + col))]}"
        ffMultiply 2 "${mixcolumns_state[$((8 + col))]}" c
        ffMultiply 3 "${mixcolumns_state[$((12 + col))]}" d

        copy_state[$((8 + col))]=$((a ^ b ^ c ^ d))

        # row 3
        ffMultiply 3 "${mixcolumns_state[$((0 + col))]}" a
        b="${mixcolumns_state[$((4 + col))]}"
        c="${mixcolumns_state[$((8 + col))]}"
        ffMultiply 2 "${mixcolumns_state[$((12 + col))]}" d

        copy_state[$((12 + col))]=$((a ^ b ^ c ^ d))
    done

    mixcolumns_state=()
    mixcolumns_state=("${copy_state[@]}")
}
