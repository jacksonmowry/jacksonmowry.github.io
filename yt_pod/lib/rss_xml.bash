#!/usr/bin/env bash

__items_loop() {
    local db_file="${1}"
    tail -n +2 "${db_file}" | while read -r line; do
        IFS='|' read -r _ title description length type url guid pubDate duration <<<"${line}"
        printf '    <item>\n'
        printf '      <itunes:episodeType>full</itunes:episodeType>\n'
        printf '      <itunes:title>%s</itunes:title>\n' "${title}"
        printf '      <description>\n'
        printf '          <![CDATA[%s]]>\n' "${description}"
        printf '      </description>\n'
        printf '      <enclosure\n'
        printf '        length="%s"\n' "${length}"
        printf '        type="%s"\n' "${type}"
        printf '        url="%s"\n' "${url}"
        printf '      />\n'
        printf '      <guid>%s</guid>\n' "${guid}"
        printf '      <pubDate>%s</pubDate>\n' "${pubDate}"
        printf '      <itunes:duration>%s</itunes:duration>\n' "$((duration / 1000))"
        printf '      <itunes:explicit>false</itunes:explicit>\n'
        printf '    </item>\n'
    done
}

## generate_xml creates an up to date copy of the rss feed
## $1:output_file
## $2:db_file
## $3:title
## $4:link
## $5:author
## $6:description
## $7:image
generate_xml() {
    local output_file=$1
    local db_file=$2
    local title=$3
    local link=$4
    local author=$5
    local description=$6
    local image=$7
    local xml
    xml=$(
        printf '<?xml version="1.0" encoding="UTF-8"?>\n'
        printf '<rss version="2.0" xmlns:itunes="http://www.itunes.com/dtds/podcast-1.0.dtd" xmlns:content="http://purl.org/rss/1.0/modules/content/">\n'
        printf '  <channel>\n'
        printf '    <title>%s</title>\n' "${title:?Please provide a title in ${config_file}}"
        printf '    <link>%s</link>\n' "${link:?Please provide a link in ${config_file}}"
        printf '    <language>%s</language>\n' "en-us"
        printf '    <copyright>%s</copyright>\n' "&#169; 2025 ${author:?Please provide a author in ${config_file}}"
        printf '    <itunes:author>%s</itunes:author>\n' "${author}"
        printf '    <description>%s</description>\n' "${description:?Please provide a description in ${config_file}}"
        printf '    <itunes:type>%s</itunes:type>\n' "serial"
        printf '    <itunes:image href="%s" />\n' "${image:?Please provide an image link in ${config_file}}"
        printf '    <itunes:category text="True Crime" />'
        printf '    <itunes:explicit>%s</itunes:explicit>\n' "false"
        printf '    <itunes:summary>sumtin</itunes:summary>\n'
        __items_loop "${db_file}"
        printf '  </channel>\n'
        printf '</rss>\n'
    ) || exit 1
    echo "${xml}" >"${output_file}"
}
