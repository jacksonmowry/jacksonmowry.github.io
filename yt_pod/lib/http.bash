#!/usr/bin/env bash

declare -rg not_found='HTTP/1.1 404 Not Found\nConnection: close'
export not_found

## parse_request takes in an HTTP 1.1 request (stdin) and sets `method` and `route`
parse_request() {
    echo "are we getting here?"
    method=
    route=
    {
        read -r __method __route _
        method="${__method}"
        route="${__route}"
    }
    echo "${method}" "${route}"
}
