#!/usr/bin/env bash

## add_video takes in the db_file and a youtube video url
## parameters:
## $1:db_file
## $2:video_url
## $3:base_url
## $4:cookie_path
add_video() {
    local db_file=$1
    local video_url=$2
    local base_url=$3
    local cookie_path=$4

    IFS=$'\n'
    local response=($(curl -b "${cookie_path}" -s "${video_url}" | grep -Po '<meta name="description"(.*?)>|<title>(.*?)</title>|"approxDurationMs":"(.*?)"'))

    local title="${response[0]}"
    local description="${response[1]}"
    title=${title#<title>}
    title=${title% - YouTube</title>}
    description=${description#<meta name=\"description\" content=\"}
    description=${description%\">}

    local guid
    local file_length
    local type
    local url
    local pubDate
    local duration

    guid=$(cat /proc/sys/kernel/random/uuid)
    yt-dlp -q -i --cookies "${cookie_path}" --format "bestaudio" -x --add-metadata --embed-metadata --audio-format "best" -o "assets/${guid}.%(ext)s" "${video_url}"
    file_length=$(wc -c assets/"${guid}".*)
    file_length=${file_length% *}
    type='audio/opus'
    url="${base_url}"/episode/"${guid}".opus
    pubDate=$(date --rfc-2822)
    duration=${response[2]:-0}
    duration=$(cut -d: -f2 <<<"${duration}")
    duration=${duration//\"/}

    printf '%s|%s|%s|%s|%s|%s|%s|%s|%s\n' "${video_url}" "${title}" "${description}" "${file_length}" "${type}" "${url}" "${guid}" "${pubDate}" "${duration}" >>"${db_file}"
}

## serve_episode takes in a guid, downloads the episode and puts it on stdout, also takes in the db_file
## parameters:
## $1:db_file
## $2:cookie_path
## $3:guid
serve_episode() {
    local cookie_path="${2}"
    local guid="${3}"

    local url

    url=$(grep -i \|"${guid}"\| "${db_file}" | cut -d\| -f1)
    if [ -z "${url}" ]; then
        printf "No episode found matching %s\n" "${guid}"
    fi

    cat "assets/${guid}.opus"
}
