#!/usr/bin/env bash

## load_configuration creates an associative array `configuration` with
## the following keys {title, link, author, description, image, port, base_url}.
## parameters:
## $1:config_file
load_configuration() {
    local config_file="${1}"

    declare -gA configuration
    while read -r line; do
        IFS='=' read -r key value <<<"${line}"""
        case "${key}" in
        title) configuration[title]="${value}" ;;
        link) configuration[link]="${value}" ;;
        author) configuration[author]="${value}" ;;
        description) configuration[description]="${value}" ;;
        image) configuration[image]="${value}" ;;
        port) configuration[port]="${value}" ;;
        base_url) configuration[base_url]="${value}" ;;
        cookie_path) configuration[cookie_path]="${value}" ;;
        esac
    done <"${config_file}"

    # for key in "${!configuration[@]}"; do
    #     echo "${key}" "${configuration[${key}]}"
    # done
    export configuration
}
