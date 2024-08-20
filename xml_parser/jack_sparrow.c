#include "xml_parser.h"
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct field {
    enum type { STRING, INTEGER, TIMESTAMP } type;
    bool required;
    bool array;
    char* default_value;
    char* name;
} field;

VECTOR_PROTOTYPE(field);
VECTOR(field);

typedef struct message {
    char* name;
    vector_field fields;
} message;

VECTOR_PROTOTYPE(message);
VECTOR(message);

typedef struct schema {
    char* version;
    vector_message messages;
} schema;

char* parse_opening_schema(element e) {
    assert(e.type == OPENING_TAG);

    // grab version, should be the only prop
    kv_pair version_prop = plist_get_pair(e.properties, 0);
    // Key must be "version"
    if (!strcmp("version", version_prop.key)) {
        fprintf(stderr,
                "%s:%s:%d: tag '<schema>' must have exactly "
                "one property 'version'\n",
                __FILE__, __func__, __LINE__);
        exit(1);
    }

    // Check to ensure that we have a version number
    if (version_prop.val == NULL || strlen(version_prop.val) == 0) {
        fprintf(stderr,
                "%s:%s:%d: Version string must be non-null and "
                "non-zero length\n",
                __FILE__, __func__, __LINE__);
        exit(1);
    }

    return version_prop.val;
}

int main() {
    struct stat sb;

    int fd = open("example.xml", O_RDONLY);
    fstat(fd, &sb);
    char* file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    streaming_parser p = (streaming_parser){
        .input = file, .length = sb.st_size, .current = file[0]};

    element_t e;
    schema s = {};
    message temp_message = {};
    field temp_field = {};

    while ((e = next_element(&p)).tag != END_OF_FILE) {
        switch (e.tag) {
        case EMPTY:
            fprintf(stderr, "%s:%s:%d: Ran into an empty element, exiting\n",
                    __FILE__, __func__, __LINE__);
            exit(1);
            break;
        case ELEMENT: {
            element element = e.e.e;
            // Parse opening schema
            switch (element.type) {
            case OPENING_TAG: {
                if (s.version == NULL &&
                    !strcmp("schema", element.element_type)) {
                    s.version = parse_opening_schema(element);
                }
            } break;
            case CLOSING_TAG:
                break;
            }
        } break;
        CLOSING_TAG:
        case TEXT_CONTENT:
        case END_OF_FILE:
        }
    }

    print_document(&p);
}
