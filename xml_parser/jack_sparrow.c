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

// Not super happy with this part yet, needs to be dynamic so that types can
// added at runtime, although the primitive types needs to be defined somewhere
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

typedef struct procedure {
    char* name;
    char* description;
    vector_field parameters;
    vector_field return_values;
} procedure;

VECTOR_PROTOTYPE(procedure);
VECTOR(procedure);

typedef struct schema {
    char* version;
    vector_message messages;
    vector_procedure procedures;
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

message parse_message(element e) {
    char* name = strdup(plist_get_val(e.properties, "name"));

    if (!name) {
        log(ERROR, "Opening message tag must have a `name` property\n");
        exit(1);
    }

    return (message){.name = name, .fields = vector_field_init(1)};
}

procedure parse_procedure(element e) {
    char* name = strdup(plist_get_val(e.properties, "name"));

    if (!name) {
        log(ERROR, "Opening procedure tag must have a `name` property\n");
        exit(1);
    }

    return (procedure){.name = name,
                       .parameters = vector_field_init(1),
                       .return_values = vector_field_init(1)};
}

field parse_field(element e) {
    char* type = (char*)plist_get_val(e.properties, "type");
    char* required = (char*)plist_get_val(e.properties, "required");
    char* default_value = (char*)plist_get_val(e.properties, "default");

    if (!type) {
        log(ERROR, "Field must have type property\n");
        exit(1);
    }

    return (field){.type_name = strdup(type),
                   .required = strcmp("false", required ? required : ""),
                   .default_value = strdup(default_value ? default_value : "")};
}

void schema_print(schema s) {
    printf("Schema: \n");

    printf("\tMessages:\n");
    for (size_t i = 0; i < s.messages.len; i++) {
        printf("\t\t%s:\n", s.messages.array[i].name);
        printf("\t\t\tFields:\n");

        for (size_t j = 0; j < s.messages.array[i].fields.len; j++) {
            field f = s.messages.array[i].fields.array[j];
            printf("\t\t\t\t%s: %s, %s, default: %s\n", f.name, f.type_name,
                   f.required ? "required" : "optional", f.default_value);
        }
    }

    printf("\tProcedures:\n");
    for (size_t i = 0; i < s.procedures.len; i++) {
        printf("\t\t%s:\n", s.procedures.array[i].name);

        printf("\t\t\tParameters:\n");
        for (size_t j = 0; j < s.procedures.array[i].parameters.len; j++) {
            field f = s.procedures.array[i].parameters.array[j];
            printf("\t\t\t\t%s: %s, %s, default: %s\n", f.name, f.type_name,
                   f.required ? "required" : "optional", f.default_value);
        }

        printf("\t\t\tReturn Values:\n");
        for (size_t j = 0; j < s.procedures.array[i].return_values.len; j++) {
            field f = s.procedures.array[i].return_values.array[j];
            printf("\t\t\t\t%s: %s, %s, default: %s\n", f.name, f.type_name,
                   f.required ? "required" : "optional", f.default_value);
        }
    }
}

void schema_free(schema s) {
    free(s.version);

    for (size_t i = 0; i < s.messages.len; i++) {
        free(s.messages.array[i].name);
        for (size_t j = 0; j < s.messages.array[i].fields.len; j++) {
            field f = s.messages.array[i].fields.array[j];
            free(f.type_name);
            free(f.default_value);
            free(f.name);
        }
        vector_field_free(s.messages.array[i].fields);
    }

    vector_message_free(s.messages);

    for (size_t i = 0; i < s.procedures.len; i++) {
        free(s.procedures.array[i].name);
        free(s.procedures.array[i].description);
        for (size_t j = 0; j < s.procedures.array[i].parameters.len; j++) {
            field f = s.procedures.array[i].parameters.array[j];
            free(f.type_name);
            free(f.default_value);
            free(f.name);
        }
        vector_field_free(s.procedures.array[i].parameters);

        for (size_t j = 0; j < s.procedures.array[i].return_values.len; j++) {
            field f = s.procedures.array[i].return_values.array[j];
            free(f.type_name);
            free(f.default_value);
            free(f.name);
        }
        vector_field_free(s.procedures.array[i].return_values);
    }

    vector_procedure_free(s.procedures);
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
    schema schema = {};
    message temp_message = {};
    procedure temp_procedure = {};
    field temp_field = {};

    bool inside_schema = false;
    bool inside_messages = false;
    bool inside_message = false;
    bool inside_field = false;
    bool inside_procedures = false;
    bool inside_procedure = false;
    bool inside_parameters = false;
    bool inside_returns = false;
    bool inside_description = false;

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
                if (!inside_schema && !strcmp("schema", element.element_type)) {
                    inside_schema = true;
                    schema.version = parse_opening_schema(element);
                    break;
                }

                // Opening messages tag
                if (inside_schema && !inside_messages &&
                    !strcmp("messages", element.element_type)) {
                    inside_messages = true;
                    schema.messages = vector_message_init(1);
                    break;
                }

                // Opening message tag
                if (inside_schema && inside_messages && !inside_message &&
                    !strcmp("message", element.element_type)) {
                    inside_message = true;

                    temp_message = parse_message(element);
                    break;
                }

                // Opening procedures tag
                if (inside_schema && !inside_procedures &&
                    !strcmp("procedures", element.element_type)) {
                    inside_procedures = true;

                    schema.procedures = vector_procedure_init(1);
                    break;
                }

                // Opening procedure tag
                if (inside_schema && inside_procedures && !inside_procedure &&
                    !strcmp("procedure", element.element_type)) {
                    inside_procedure = true;

                    temp_procedure = parse_procedure(element);
                    break;
                }

                // Opening description tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    !inside_description &&
                    !strcmp("description", element.element_type)) {
                    inside_description = true;
                    break;
                }

                // Opening parameters tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    !inside_parameters &&
                    !strcmp("parameters", element.element_type)) {
                    inside_parameters = true;

                    break;
                }

                // Opening returns tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    !inside_returns &&
                    !strcmp("returns", element.element_type)) {
                    inside_returns = true;

                    break;
                }

                // Opening field tag
                if (inside_schema &&
                    ((inside_messages && inside_message) ||
                     (inside_procedures && inside_procedure &&
                      (inside_returns || inside_parameters))) &&
                    !inside_field && !strcmp("field", element.element_type)) {
                    inside_field = true;

                    temp_field = parse_field(element);
                    break;
                }

                assert("we can't get here" == false);
            } break;
            case CLOSING_TAG: {
                // Closing field tag
                if (inside_schema && !strcmp("field", element.element_type)) {
                    if (inside_messages && inside_message) {
                        vector_field_pb(&temp_message.fields, temp_field);
                    } else if (inside_procedures && inside_procedure) {
                        if (inside_parameters) {
                            vector_field_pb(&temp_procedure.parameters,
                                            temp_field);
                        } else if (inside_returns) {
                            vector_field_pb(&temp_procedure.return_values,
                                            temp_field);
                        }
                    }

                    temp_field = (field){};

                    inside_field = false;
                    break;
                }

                // Closing message tag
                if (inside_schema && inside_messages && inside_message &&
                    !strcmp("message", element.element_type)) {
                    vector_message_pb(&schema.messages, temp_message);
                    temp_message = (message){};

                    inside_message = false;
                    break;
                }

                // Closing messages tag
                if (inside_schema && inside_messages &&
                    !strcmp("messages", element.element_type)) {
                    inside_messages = false;
                    break;
                }

                // Closing description tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    inside_description &&
                    !strcmp("description", element.element_type)) {
                    inside_description = false;
                    break;
                }

                // Closing parameters tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    inside_parameters &&
                    !strcmp("parameters", element.element_type)) {
                    inside_parameters = false;
                    break;
                }

                // Closing returns tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    inside_returns &&
                    !strcmp("returns", element.element_type)) {
                    inside_returns = false;
                    break;
                }

                // Closing procedure tag
                if (inside_schema && inside_procedures && inside_procedure &&
                    !strcmp("procedure", element.element_type)) {
                    inside_procedure = false;

                    vector_procedure_pb(&schema.procedures, temp_procedure);
                    temp_procedure = (procedure){};

                    break;
                }

                // Closing procedures tag
                if (inside_schema && inside_procedures &&
                    !strcmp("procedures", element.element_type)) {
                    inside_procedures = false;

                    break;
                }

                // Closing schema tag
                if (inside_schema && !strcmp("schema", element.element_type)) {
                    inside_schema = false;

                    break;
                }

                // Closing schema tag error
                if (!inside_schema && !strcmp("schema", element.element_type)) {
                    log(ERROR, "Encountered closing schema tag before schema "
                               "was opened, check for typos\n");
                    exit(1);
                }

                // Closing messages tag error
                if (inside_schema && !inside_messages &&
                    !strcmp("messages", element.element_type)) {
                    log(ERROR, "Encounted closing messages tag before messages "
                               "was opened, check for typos\n");
                    exit(1);
                }

                assert("we can't get here" == false);
            } break;
            }
        } break;
        case TEXT_CONTENT: {
            text_content tc = e.e.tc;
            switch (tc.type) {
            case PLAIN_TEXT: {
                // Name of a field
                if (inside_field) {
                    temp_field.name = strndup(tc.content.array, tc.content.len);
                    break;
                }

                // Description of a procedure
                if (inside_description) {
                    temp_procedure.description =
                        strndup(tc.content.array, tc.content.len);
                    break;
                }

                assert("we can't get here" == false);
            } break;
            default:
                break;
            }
        }
        case END_OF_FILE:
            break;
        }

        element_t_free(e);
    }

    schema_print(schema);
    schema_free(schema);
}
