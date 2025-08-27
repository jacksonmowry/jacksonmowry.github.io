#!/usr/bin/env bash

function expect_equal_int {
    local expected="${1}"
    local actual="${2}"

    if ((expected != actual)); then
        echo "Expected ${expected}, got ${actual}"
        exit 1
    fi
}

function expect_equal_int_array {
    local -n testing_expected="${1}"
    local -n testing_actual="${2}"

    local len=${#testing_expected[@]}
    for ((i = 0; i < len; i++)); do
        if ((testing_expected[i] != testing_actual[i])); then
            echo "Expected [${testing_expected[*]}]"
            echo "Got [${testing_actual[*]}]"
            printf "Element %d (0x%08x != 0x%08x)\n" $((i)) $((testing_expected[i])) $((testing_actual[i]))
            exit 1
        fi
    done
}
