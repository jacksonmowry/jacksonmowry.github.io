#include "xml_parser.h"
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define strdupa(str)                                                           \
    ({                                                                         \
        char* alloced_str = alloca(strlen(str) + 1);                           \
        strcpy(alloced_str, str);                                              \
        alloced_str;                                                           \
    })

#define RED(str)                                                               \
    "\033[31m"                                                                 \
    "\033[1m" str "\033[0m"

#define ERROR RED("error")

#define log(level, fmt_string, ...)                                            \
    fprintf(stderr, "%-10s %s:%s:%d: ", level, __FILE__, __func__, __LINE__);  \
    fprintf(stderr, fmt_string, ##__VA_ARGS__)

/* The list of values we want to turn into an enum and a string array */
#define LIST_OF_TYPES                                                          \
    X(STRING)                                                                  \
    X(INTEGER)                                                                 \
    X(TIMESTAMP)                                                               \
    X(FLOAT)                                                                   \
    X(DOUBLE)

/* Create the enumeration */
#define X(name) name,
typedef enum { LIST_OF_TYPES } types_t;
#undef X

/* Create the strings mapped to the enumeration */
#define X(name) #name,
static char* type_strings[] = {LIST_OF_TYPES};
#undef X

types_t find_type(const char* type) {
    char* copy = strdupa(type);
    // Convert to upper
    for (size_t i = 0; copy[i]; i++) {
        if (copy[i] >= 'a' && type[i] <= 'z') {
            copy[i] -= 'a' - 'A';
        }
    }

    for (size_t i = 0; i < sizeof(type_strings) / sizeof(char*); i++) {
        if (!strcmp(type_strings[i], copy)) {
            return i;
        }
    }

    return -1;
}

typedef struct field {
    types_t type;
    char* type_name;
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
    char* version = strdup(plist_get_val(e.properties, "version"));
    // Key must be "version"

    // Check to ensure that we have a version number
    if (version == NULL || strlen(version) == 0) {
        log(ERROR, "Version string must be non-null and non-zero length\n");
        exit(1);
    }

    return version;
}

message parse_message(vector_element_t v) {
    message m = {.name =
                     strdup(plist_get_val(v.array[0].e.e.properties, "name")),
                 .fields = vector_field_init(1)};

    for (size_t i = 1; i < v.len; i += 3) {
        if (v.array[i].tag != ELEMENT || v.array[i + 1].tag != TEXT_CONTENT ||
            v.array[i + 2].tag != ELEMENT) {
            log(ERROR,
                "Incorrect element pattern when parsing field of message: "
                "'%s', should be "
                "ELEMENT, TEXT, ELEMENT\n",
                m.name);
            exit(1);
        }

        if (v.array[i].e.e.type != OPENING_TAG ||
            v.array[i + 2].e.e.type != CLOSING_TAG) {
            log(ERROR,
                "Malformed opening/closing tags when parsing field: '%.*s' of "
                "message: '%s'\n",
                (int)v.array[i + 1].e.tc.content.len,
                v.array[i + 1].e.tc.content.array, m.name);
            exit(1);
        }
        field f = {};
        plist* p = v.array[i].e.e.properties;
        const char* type = plist_get_val(p, "type");
        const char* required = plist_get_val(p, "required");
        const char* default_val = plist_get_val(p, "default");

        if (!type) {
            log(ERROR,
                "Field: '%.*s' without a type specifer within message: '%s'!\n",
                (int)v.array[i + 1].e.tc.content.len,
                v.array[i + 1].e.tc.content.array, m.name);
            exit(1);
        }

        f.type = find_type(type);
        f.type_name = strdup(type);

        if (required) {
            f.required =
                strcmp("false", required); // any non-false value is true
        }

        if (default_val) {
            f.default_value = strdup(default_val);
        }

        f.name = strndup(v.array[i + 1].e.tc.content.array,
                         v.array[i + 1].e.tc.content.len);

        vector_field_pb(&m.fields, f);
    }

    return m;
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
            log(ERROR, "Ran into an empty element\n");
            exit(1);
            break;
        case ELEMENT: {
            element element = e.e.e;
            // Parse opening schema
            switch (element.type) {
            case OPENING_TAG: {
                // Opening schema tag
                if (s.version == NULL &&
                    !strcmp("schema", element.element_type)) {
                    s.version = parse_opening_schema(element);
                }

                // Opening messages tag
                if (s.messages.array == NULL &&
                    !strcmp("messages", element.element_type)) {
                    s.messages = vector_message_init(1);
                }

                // Opening message tag
                if (s.messages.array &&
                    !strcmp("message", element.element_type)) {
                    vector_element_t v = vector_element_t_init(10);

                    while (true) {
                        vector_element_t_pb(&v, e);

                        e = next_element(&p);
                        if (e.tag == ELEMENT && e.e.e.type == CLOSING_TAG &&
                            // Find closing message tag
                            !strcmp(element.element_type, e.e.e.element_type)) {
                            vector_element_t_pb(&v, e);
                            break;
                        }
                    }

                    temp_message = parse_message(v);
                    vector_message_pb(&s.messages, temp_message);

                    for (size_t i = 0; i < v.len; i++) {
                        element_t_free(v.array[i]);
                    }

                    vector_element_t_free(v);
                }
            } break;
            case CLOSING_TAG: {
                // Closing schema tag
                if (s.version == NULL &&
                    !strcmp("schema", element.element_type)) {
                    log(ERROR, "Encountered closing schema tag before schema "
                               "was opened, check for typos\n");
                    exit(1);
                }

                // Closing messages tag
                if (s.messages.array == NULL &&
                    !strcmp("messages", element.element_type)) {
                    log(ERROR, "Encounted closing messages tag before messages "
                               "was opened, check for typos\n");
                    exit(1);
                }
            } break;
            }
        } break;
        case TEXT_CONTENT:
        case END_OF_FILE:
            break;
        }

        element_t_free(e);
    }

    print_document(&p);
}
