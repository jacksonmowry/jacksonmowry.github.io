#!/usr/bin/env bash

set -euo pipefail

db_file="${YT_POD_DB_FILE:-episodes.db}"
xml_file="${YT_POD_XML_FILE:-podcast.xml}"
config_file="${YT_POD_CONFIG_FILE:-configure.txt}"
adding_video=false

## serve_feed hosts a simple HTTP server on `port` serving `feed_file`
## parameters:
## $1:port
## $2:feed_file
## $3:cookie_path
serve_feed() {
    local port=$1
    local xml_file=$2
    local cookie_path=$3
    echo "Serving application on port ${port:?Please provide a port in ${config_file}}"
    running=true
    trap "running=false;" SIGTERM SIGINT
    set +e
    while [ "$running" = true ]; do
        coproc (nc -l -p "${port}" localhost >&1)
        #coproc (nc -l localhost "${port}" 2>&1)

        {
            read -r method route _
            case "${method}" in
            GET | get)
                case "${route}" in
                /episode/*.opus)
                    local filename
                    local guid

                    guid=${route#/episode/}
                    guid=${guid%.opus}
                    #filename=$(ls assets/"${guid}".*)
                    #printf 'HTTP/1.1 200\r\nContent-Type: audio/opus\r\nContent-Disposition: attachment; filename="%s"\r\nContent-Length: %s\r\n\r\n' "${filename#assets/}" "$(wc -c "${filename}" | cut -d' ' -f1)"
                    printf 'HTTP/1.1 200\r\n\r\n'
                    serve_episode "${db_file}" "${cookie_path}" "${guid}"
                    ;;
                /img)
                    printf 'HTTP/1.1 200\r\nContent-Type: image/jpeg\r\nContent-Length: %s\r\n\r\n' "$(wc -c img | cut -d' ' -f1)"
                    cat img
                    ;;
                *)
                    printf 'HTTP/1.1 200\r\nContent-Type: text/xml\r\nContent-Length: %s\r\n\r\n' "$(wc -c "${xml_file}" | cut -d' ' -f1)"
                    cat "${xml_file}"
                    ;;
                esac
                ;;
            HEAD | head)
                case "${route}" in
                /img)
                    printf 'HTTP/1.1 200\r\nContent-Type: image/jpeg\r\nContent-Length: %s\r\n\r\n' "$(wc -c img | cut -d' ' -f1)"
                    ;;
                *)
                    printf 'HTTP/1.1 200\r\nContent-Type: text/xml\r\nContent-Length: %s\r\n\r\n' "$(wc -c "${xml_file}" | cut -d' ' -f1)"
                    ;;
                esac
                ;;
            *) echo "${not_found}" ;;
            esac
        } <&"${COPROC[0]}" >&"${COPROC[1]}"

        local fd="${COPROC[1]}"
        exec {fd}>&-
        fd="${COPROC[0]}"
        exec {fd}>&-

        kill "$COPROC_PID"

    done

    set -e
}

main() {
    declare -r usage="usage: ${0} [(-a/--add_video)] [(-d/--db_file) db_file] [(-x/--xml_file) xml_file] [(-c/--config_file) config_file]"
    valid_args=$(getopt --options='ad:x:c:' --longoptions='add_video,db_file:,xml_file:,config_file:' --name "${0}" -- "$@" || exit 2)

    eval set -- "${valid_args}"
    while true; do
        case "${1}" in
        -a | --add_video)
            adding_video=true
            shift
            ;;
        -d | --db_file)
            db_file=$2
            shift 2
            ;;
        -x | --xml_file)
            xml_file=$2
            shift 2
            ;;
        -c | --config_file)
            config_file=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        esac
    done

    load_configuration "${config_file}"
    if ! [ -f "${xml_file}" ] || [ "${db_file}" -nt "${xml_file}" ] || [ "${config_file}" -nt "${xml_file}" ] || [ "${BASH_SOURCE[0]}" -nt "${xml_file}" ]; then
        generate_xml "${xml_file}" "${db_file}" "${configuration[title]}" "${configuration[link]}" "${configuration[author]}" "${configuration[description]}" "${configuration[image]}"
    fi

    if [ "${adding_video}" = true ]; then
        if [ $# -ne 1 ]; then
            echo "${0}: a url is required with passing (-a/--add_video)"
            exit 1
        fi
        add_video "${db_file}" "${1}" "${configuration[base_url]}" "${configuration[cookie_path]}"
        exit 0
    fi

    serve_feed "${configuration[port]}" "${xml_file}" "${configuration[cookie_path]}"
}

source lib/db.bash
source lib/rss_xml.bash
source lib/configuration.bash
source lib/http.bash
# add_video "${db_file}" "${1}" "${base_url}"
# exit 0
# serve_episode "${db_file}" "${1}" > test.opus
# exit 0

main "${@}"
