#include "xml_parser.h"
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
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

typedef enum primitive_t {
    none = -1,
    js_string,    // => 8
    js_i64,       // => 8
    js_u64,       // => 8
    js_i32,       // => 4
    js_u32,       // => 4
    js_i16,       // => 2
    js_u16,       // => 2
    js_i8,        // => 1
    js_u8,        // => 1
    js_char,      // => 1
    js_double,    // => 8
    js_float,     // => 4
    js_half,      // => 2
    js_timestamp, // => 8
    js_boolean    // => 1
} primitive_t;
const size_t primitive_sizes[] = {8, 8, 8, 4, 4, 2, 2, 1, 1, 1, 8, 4, 2, 8, 1};
const char* primitive_names[] = {
    "string", "i64",  "u64",    "i32",   "u32",  "i16",       "u16",    "i8",
    "u8",     "char", "double", "float", "half", "timestamp", "boolean"};
// Returns the enum value for `primitive_t` or -1 if not found
primitive_t primitive_find(const char* typename) {
    for (size_t i = 0; i < sizeof(primitive_names) / sizeof(char*); i++) {
        if (!strcmp(typename, primitive_names[i])) {
            return i;
        }
    }

    return -1;
}

typedef struct field {
    primitive_t type;
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
    size_t base_size;
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

typedef enum state {
    OUTSIDE,
    SCHEMA,
    MESSAGES,
    MESSAGE,
    MESSAGE_FIELD,
    PARAMETER_FIELD,
    RETURNS_FIELD,
    PROCEDURES,
    PROCEDURE,
    PARAMETERS,
    RETURNS,
    DESCRIPTION
} state;

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

    bool array = false;
    if (!strcmp("[]", &type[strlen(type) - 2])) {
        array = true;
        type[strlen(type) - 2] = '\0';
    }

    return (field){.type = primitive_find(type),
                   .type_name = strdup(type),
                   .required = strcmp("false", required ? required : ""),
                   .array = array,
                   .default_value = strdup(default_value ? default_value : "")};
}

void element_print(element_t e) {
    switch (e.tag) {
    case EMPTY:
        printf("Empty!\n");
        break;
    case TEXT_CONTENT:
        printf("Text: %.*s\n", (int)e.e.tc.content.len, e.e.tc.content.array);
        break;
    case ELEMENT:
        if (e.e.e.type == OPENING_TAG) {
            printf("Opening Tag: %s\n", e.e.e.element_type);
        } else {
            printf("Closin Tag: %s\n", e.e.e.element_type);
        }
        break;
    case END_OF_FILE:
        break;
    }
}

void schema_print(schema s) {
    printf("Schema: \n");

    printf("\tMessages:\n");
    for (size_t i = 0; i < s.messages.len; i++) {
        printf("\t\t%s:\n", s.messages.array[i].name);
        printf("\t\t\tFields:\n");

        for (size_t j = 0; j < s.messages.array[i].fields.len; j++) {
            field f = s.messages.array[i].fields.array[j];
            printf("\t\t\t\t%s: %s(%d), array: %s, %s, default: %s\n", f.name,
                   f.type_name, f.type, f.array ? "t" : "f",
                   f.required ? "required" : "optional", f.default_value);
        }
    }

    printf("\tProcedures:\n");
    for (size_t i = 0; i < s.procedures.len; i++) {
        printf("\t\t%s:\n", s.procedures.array[i].name);

        printf("\t\t\tParameters:\n");
        for (size_t j = 0; j < s.procedures.array[i].parameters.len; j++) {
            field f = s.procedures.array[i].parameters.array[j];
            printf("\t\t\t\t%s: %s(%d), array: %s, %s, default: %s\n", f.name,
                   f.type_name, f.type, f.array ? "t" : "f",
                   f.required ? "required" : "optional", f.default_value);
        }

        printf("\t\t\tReturn Values:\n");
        for (size_t j = 0; j < s.procedures.array[i].return_values.len; j++) {
            field f = s.procedures.array[i].return_values.array[j];
            printf("\t\t\t\t%s: %s(%d), array: %s, %s, default: %s\n", f.name,
                   f.type_name, f.type, f.array ? "t" : "f",
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

bool needs_sizes(schema s) {
    for (size_t i = 0; i < s.messages.len; i++) {
        if (s.messages.array[i].base_size == 0) {
            return true;
        }
    }

    return false;
}

typedef struct graph_node {
    char* type_name;
    uint8_t* edges;
    size_t num_edges;
    size_t in_degree;
} graph_node;

typedef struct queue_int_node {
    int val;
    struct queue_int_node* previous;
} queue_int_node;

typedef struct queue_int {
    queue_int_node* head;
    queue_int_node* tail;
} queue_int;

void queue_int_add(queue_int* q, int element) {
    queue_int_node* node = malloc(sizeof(queue_int_node));
    node->val = element;

    if (!q->head) {
        q->head = q->tail = node;
        node->previous = NULL;
        return;
    }

    q->tail->previous = node;
    q->tail = node;
}

int queue_int_take(queue_int* q) {
    if (!q->head) {
        fprintf(stderr, "Oops you are atempting to take from a null queue\n");
        return -1; // ?
    }

    if (q->head == q->tail) {
        // Only 1 element
        int ret_val = q->head->val;
        free(q->head);
        q->head = q->tail = NULL;
        return ret_val;
    }

    int ret_val = q->head->val;
    queue_int_node* qin = q->head;
    q->head = qin->previous;
    free(qin);
    return ret_val;
}

bool has_circular_deps(schema s) {
    vector_message m = s.messages;
    graph_node nodes[s.messages.len];

    for (size_t i = 0; i < m.len; i++) {
        nodes[i].type_name = m.array[i].name;
        nodes[i].edges = alloca(m.len);
        nodes[i].num_edges = 0;
        nodes[i].in_degree = 0;

        vector_field f = m.array[i].fields;
        for (size_t j = 0; j < f.len; j++) {
            bool found = false;
            if (primitive_find(f.array[j].type_name) != none) {
                continue;
            }
            for (size_t k = 0; k < m.len; k++) {
                if (!strcmp(f.array[j].type_name, m.array[k].name)) {
                    nodes[i].edges[nodes[i].num_edges++] = k;
                    found = true;
                    break;
                }
            }

            if (!found) {
                fprintf(stderr,
                        "Error while resolving dependencies of %s, cannot find "
                        "type definition for field of type %s\n",
                        nodes[i].type_name, f.array[j].type_name);
                exit(1);
            }
        }
    }

    for (size_t i = 0; i < m.len; i++) {
        for (size_t j = 0; j < nodes[i].num_edges; j++) {
            nodes[nodes[i].edges[j]].in_degree++;
        }
    }

    int32_t visited = 0;
    queue_int q = {};

    for (size_t i = 0; i < m.len; i++) {
        if (nodes[i].in_degree == 0) {
            queue_int_add(&q, i);
        }
    }

    while (q.head != NULL) {
        int val = queue_int_take(&q);
        visited++;

        for (size_t i = 0; i < nodes[val].num_edges; i++) {
            nodes[nodes[val].edges[i]].in_degree--;

            if (nodes[nodes[val].edges[i]].in_degree == 0) {
                queue_int_add(&q, nodes[val].edges[i]);
            }
        }
    }

    if (visited == m.len) {
        return false;
    }

DEBUG:
    for (size_t i = 0; i < m.len; i++) {
        printf("%s (%lu): [", nodes[i].type_name, nodes[i].in_degree);
        for (ssize_t j = 0; j < (ssize_t)nodes[i].num_edges - 1; j++) {
            printf("%d, ", nodes[i].edges[j]);
        }
        if (nodes[i].num_edges > 0) {
            printf("%d", nodes[i].edges[nodes[i].num_edges - 1]);
        }
        printf("]\n");
    }

    printf("%d %d\n", visited, m.len);

    return true;
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

    state current_state = OUTSIDE;

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
                if (current_state == OUTSIDE &&
                    !strcmp("schema", element.element_type)) {
                    current_state = SCHEMA;
                    schema.version = parse_opening_schema(element);
                    break;
                }

                // Opening messages tag
                if (current_state == SCHEMA &&
                    !strcmp("messages", element.element_type)) {
                    current_state = MESSAGES;
                    schema.messages = vector_message_init(1);
                    break;
                }

                // Opening message tag
                if (current_state == MESSAGES &&
                    !strcmp("message", element.element_type)) {
                    current_state = MESSAGE;

                    temp_message = parse_message(element);
                    break;
                }

                // Opening procedures tag
                if (current_state == SCHEMA &&
                    !strcmp("procedures", element.element_type)) {
                    current_state = PROCEDURES;

                    schema.procedures = vector_procedure_init(1);
                    break;
                }

                // Opening procedure tag
                if (current_state == PROCEDURES &&
                    !strcmp("procedure", element.element_type)) {
                    current_state = PROCEDURE;

                    temp_procedure = parse_procedure(element);
                    break;
                }

                // Opening description tag
                if (current_state == PROCEDURE &&
                    !strcmp("description", element.element_type)) {
                    current_state = DESCRIPTION;
                    break;
                }

                // Opening parameters tag
                if (current_state == PROCEDURE &&
                    !strcmp("parameters", element.element_type)) {
                    current_state = PARAMETERS;

                    break;
                }

                // Opening returns tag
                if (current_state == PROCEDURE &&
                    !strcmp("returns", element.element_type)) {
                    current_state = RETURNS;

                    break;
                }

                // Opening field tag
                if ((current_state == MESSAGE || current_state == PARAMETERS ||
                     current_state == RETURNS) &&
                    !strcmp("field", element.element_type)) {
                    if (current_state == MESSAGE) {
                        current_state = MESSAGE_FIELD;
                    } else if (current_state == PARAMETERS) {
                        current_state = PARAMETER_FIELD;
                    } else if (current_state == RETURNS) {
                        current_state = RETURNS_FIELD;
                    }

                    temp_field = parse_field(element);
                    break;
                }

                assert("we can't get here" == false);
            } break;
            case CLOSING_TAG: {
                // Closing field tag
                if ((current_state == MESSAGE_FIELD ||
                     current_state == PARAMETER_FIELD ||
                     current_state == RETURNS_FIELD) &&
                    !strcmp("field", element.element_type)) {
                    if (current_state == MESSAGE_FIELD) {
                        vector_field_pb(&temp_message.fields, temp_field);

                        current_state = MESSAGE;
                    } else if (current_state == PARAMETER_FIELD) {
                        vector_field_pb(&temp_procedure.parameters, temp_field);

                        current_state = PARAMETERS;
                    } else if (current_state == RETURNS_FIELD) {
                        vector_field_pb(&temp_procedure.return_values,
                                        temp_field);

                        current_state = RETURNS;
                    }
                    temp_field = (field){};

                    break;
                }

                // Closing message tag
                if (current_state == MESSAGE &&
                    !strcmp("message", element.element_type)) {
                    vector_message_pb(&schema.messages, temp_message);
                    temp_message = (message){};

                    current_state = MESSAGES;
                    break;
                }

                // Closing messages tag
                if (current_state == MESSAGES &&
                    !strcmp("messages", element.element_type)) {
                    current_state = SCHEMA;
                    break;
                }

                // Closing description tag
                if (current_state == DESCRIPTION &&
                    !strcmp("description", element.element_type)) {
                    current_state = PROCEDURE;
                    break;
                }

                // Closing parameters tag
                if (current_state == PARAMETERS &&
                    !strcmp("parameters", element.element_type)) {
                    current_state = PROCEDURE;
                    break;
                }

                // Closing returns tag
                if (current_state == RETURNS &&
                    !strcmp("returns", element.element_type)) {
                    current_state = PROCEDURE;
                    break;
                }

                // Closing procedure tag
                if (current_state == PROCEDURE &&
                    !strcmp("procedure", element.element_type)) {
                    current_state = PROCEDURES;

                    vector_procedure_pb(&schema.procedures, temp_procedure);
                    temp_procedure = (procedure){};

                    break;
                }

                // Closing procedures tag
                if (current_state == PROCEDURES &&
                    !strcmp("procedures", element.element_type)) {
                    current_state = SCHEMA;

                    break;
                }

                // Closing schema tag
                if (current_state == SCHEMA &&
                    !strcmp("schema", element.element_type)) {
                    current_state = OUTSIDE;

                    break;
                }

                assert("we can't get here" == false);
            } break;
            }
            break;
        }
        case TEXT_CONTENT: {
            text_content tc = e.e.tc;
            switch (tc.type) {
            case PLAIN_TEXT: {
                // Name of a field
                if (current_state == MESSAGE_FIELD ||
                    current_state == PARAMETER_FIELD ||
                    current_state == RETURNS_FIELD) {
                    temp_field.name = strndup(tc.content.array, tc.content.len);
                    break;
                }

                // Description of a procedure
                if (current_state == DESCRIPTION) {
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

    if (has_circular_deps(schema)) {
        fprintf(stderr, "Circular deps!\n");
        exit(1);
    }

    // We need to make at least one pass over the messages to determine their
    // sizes, Likely we will have to loop around again once the size of a
    // messages dependants has been resolved
    do {
        for (size_t i = 0; i < schema.messages.len; i++) {
            message* m = &schema.messages.array[i];
            // This element already has a size
            if (m->base_size) {
                continue;
            }

            size_t message_size = 0;
            for (size_t j = 0; j < m->fields.len; j++) {
                field f = m->fields.array[j];
                // if we have a `type` look up its size, if we don't have a
                // `type` try to add one with `type_name` then resolve its size
                // We need to have some mechanism to differentiate between
                // primitive values and user-defined values can't simply use
                // f.type, or maybe we don't set f.type for user-defined values
                // and always do the lookup through `type_name` and store those
                // sizes in a cache?
                if (f.type != -1) {
                    // Value is primitive
                    size_t primitive_size = primitive_sizes[f.type];
                    message_size += primitive_size;
                } else {
                    // Value is user defined, but might not be "sized" yet
                    // Lookup by `type_name`, if not defined skip this entire
                    // message
                    // ssize_t user_type_size = user_types_find(f.type_name)
                    // if (user_type_size == -1) {
                    //     continue;
                    // }
                    // message_size += user_type_size;
                }
            }

            // If we get here that means the message has a "true" size now
            m->base_size = message_size;

            // Now add its size to the user-defined sizes cache
            // user_type_add(&user_type_cache, m->type_name, message_size);
        }
    } while (needs_sizes(schema));

    schema_free(schema);
}
