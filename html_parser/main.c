#include "parser.h"
#include "plist.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

types* element_types;

type get_type(element_t e) {
    switch (e.tag) {
    TS:
        return e.e.ts.type;
    E:
        return e.e.e.tag_size.type;
    TC:
        return e.e.tc.tag_size.type;
    default:
        exit(1);
    }
}
void set_type(element_t* e, type type) {
    switch (e->tag) {
    TS:
        e->e.ts.type = type;
    E:
        e->e.e.tag_size.type = type;
    TC:
        e->e.tc.tag_size.type = type;
    default:
        exit(1);
    }
}

uint16_t get_size(element_t e) {
    switch (e.tag) {
    TS:
        return e.e.ts.size;
    E:
        return e.e.e.tag_size.size;
    TC:
        return e.e.tc.tag_size.size;
    default:
        exit(1);
    }

    return 0;
}
void set_size(element_t* e, uint16_t size) {
    switch (e->tag) {
    TS:
        e->e.ts.size = size;
    E:
        e->e.e.tag_size.size = size;
    TC:
        e->e.tc.tag_size.size = size;
    default:
        exit(1);
    }
}
uint16_t get_cap(element_t e) {
    switch (e.tag) {
    TS:
        return e.e.ts.cap;
    E:
        return e.e.e.tag_size.cap;
    TC:
        return e.e.tc.tag_size.cap;
    default:
        exit(1);
    }

    return 0;
}
void set_cap(element_t* e, uint16_t cap) {
    switch (e->tag) {
    TS:
        e->e.ts.cap = cap;
    E:
        e->e.e.tag_size.cap = cap;
    TC:
        e->e.tc.tag_size.cap = cap;
    default:
        exit(1);
    }
}

void set_content(element_t* e, const char* content, uint16_t length) {
    switch (e->tag) {
    TC:
        e->e.tc.content = vector_char_from(content, length);
    TS:
    E:
    default:
        exit(1);
    }
}

const char* get_element_type(element_t e) {
    switch (e.tag) {
    E:
        return element_types->types[e.e.e.tag_size.element_type];
    TS:
    TC:
    default:
        exit(1);
    }
}

element_t new_plain_content(char* text, size_t size) {
    if (size == 0) {
        element_t no_content;
        set_type(&no_content, NO_CONTENT);
        return no_content;
    }
    element_t text_content;
    text_content.tag = TC;
    set_type(&text_content, PLAIN_TEXT);
    set_size(&text_content, size);
    set_cap(&text_content, size);
    set_content(&text_content, text, size);
    return text_content;
}
element_t new_element(char* type) {
    return (element_t){.tag = E,
                       .e.e = {.tag_size = {.type = OPENING_TAG,
                                            .element_type = add_element_type(
                                                type, strlen(type)),
                                            .size = 0,
                                            .cap = 1},
                               .properties = plist_init()}};
}

element_t new_closing_tag(const char* type) {
    return (element_t){.tag = E,
                       .e.e = {.tag_size = {.type = CLOSING_TAG,
                                            .element_type = add_element_type(
                                                type, strlen(type))}}};
}

element_t new_comment(const char* comment, size_t size) {
    return (element_t){
        .tag = TC,
        .e.tc = {.tag_size = {.type = COMMENT, .size = size, .cap = size},
                 .content = vector_char_from(comment, size)}};
}

void initialize_element_types() {
    element_types = calloc(1, sizeof(types) + (8 * sizeof(char*)));
    element_types->size = 0;
    element_types->cap = 8;
}

int16_t find_element_type_idx(char* needle) {
    for (int i = 0; i < element_types->size; i++) {
        if (!strcmp(needle, element_types->types[i])) {
            return i;
        }
    }
    return -1;
}

int16_t add_element_type(const char* type, size_t size) {
    if (element_types->size >= element_types->cap) {
        types* new =
            calloc(1, sizeof(types) + (element_types->cap * GROWTH_FACTOR));
        memcpy(new, element_types, sizeof(types) + (element_types->cap));
        free(element_types);
        element_types = new;
        element_types->cap *= GROWTH_FACTOR;
    }

    for (int i = 0; i < element_types->cap; i++) {
        if (element_types->types[i] == NULL) {
            element_types->types[i] = strndup(type, size);
            element_types->size++;
            return i;
        }
        if (!strcmp(type, element_types->types[i])) {
            return i;
        }
    }

    return -1;
}

void add_child_element(element_t parent, element_t child) {
    switch (parent.tag) {
    E:
        vector_element_t_pb(parent.e.e.children, child);
    TS:
    TC:
    default:
        exit(1);
    }
}

element_t parse_tag(parser* p) {
    if (parser_peek(p) != '<') {
        return (element_t){};
    }
    // Cursor is now on '<'
    parser_get(p);

    // temporary buffer for parsing a tag name
    char temp[256] = {0};
    size_t temp_pos = 0;

    // parsing loop for tag name
    while (parser_peek(p) != ' ' && parser_peek(p) != '>') {
        temp[temp_pos++] = parser_get(p);
    }

    // temp should now contain the name,unless it is empty
    if (temp_pos == 0) {
        return (element_t){};
    }
    element_t new_elem = new_element(temp);

    parser_skip_whitespace(p);

    // If we're at '>' return, otherwise we need to parse attributes
    if (parser_peek(p) == '>') {
        parser_get(p);
        return new_elem;
    }

    // Larger loop that will continue until we reach a '>' character
    // KEY=VALUE, 3 step parsing for each property
    while (parser_peek(p) != '>') {
        // Skip whitespace until the properties start
        parser_skip_whitespace(p);

        size_t size = 0;
        size_t start = p->cursor;
        // Parse key
        while (parser_peek(p) != '=') {
            parser_get(p);
            size++;
        }
        char* key = strndup(&p->input[start], size);
        /* puts(key); */
        // Skip equals
        parser_get(p);
        parser_skip_whitespace(p);
        // Parse value
        if (parser_peek(p) != '"') {
            fprintf(stderr, "Values must be strings");
            exit(1);
        }
        parser_get(p);

        size = 0;
        start = p->cursor;

        while (parser_peek(p) != '"') {
            parser_get(p);
            size++;
        }
        parser_get(p);
        char* value = strndup(&p->input[start], size);
        /* puts(value); */

        plist_add(new_elem.e.e.properties, key, value);
    }
    assert(parser_peek(p) == '>');

    parser_get(p);
    return new_elem;
}

// parse_contents will look at the inner xml of an element and parse it
// depending on if it is plain text of a child element.
// In either case contents will be appended to their parents children
element_t parse_contents(parser* p, element_t parent) {
    // If the first thing we see is '<' we have a child element
    // Otherwise we have a string (plain text)
    if (comment_tag(p)) {
        element_t comment = parse_comment(p);
        add_child_element(parent, comment);
        return comment;
    } else if (parser_peek(p) == '<') {
        element_t child = parse_element(p);
        add_child_element(parent, child);
        return child;
    } else {
        element_t plain_text = parse_text(p);
        add_child_element(parent, plain_text);
        return plain_text;
    }
}

element_t parse_text(parser* p) {
    parser_skip_whitespace(p);

    size_t start = p->cursor;
    size_t size = 0;
    size_t last_nonwhitespace = p->cursor;

    while (parser_peek(p) != '<') {
        if (parser_peek(p) != ' ') {
            last_nonwhitespace = p->cursor;
        }
        parser_get(p);
        size++;
    }

    return new_plain_content(&p->input[start], last_nonwhitespace - start + 1);
}

element_t parse_element(parser* p) {
    element_t new_element = parse_tag(p);
    size_t closing_tag_len = strlen(get_element_type(new_element)) + 4;
    char closing_tag[closing_tag_len];
    memset(closing_tag, 0, sizeof(closing_tag));
    strcat(closing_tag, "</");
    strcat(closing_tag, get_element_type(new_element));
    strcat(closing_tag, ">");

    // While I am not looking at my own closing tag
    while (!closing_tag_p(closing_tag, closing_tag_len - 1, p)) {
        parse_contents(p, new_element);
    }

    // Advance past closing tag
    p->cursor += closing_tag_len - 1;

    return new_element;
}

bool closing_tag_p(const char* closing_tag, size_t closing_tag_len, parser* p) {
    parser_skip_whitespace(p);
    if (closing_tag_len + p->cursor > p->len) {
        // crash here?
        fprintf(stderr,
                "closing_tag_p | EOF without matching closing tag (%s):%d\n",
                closing_tag, __LINE__);
        exit(1);
    }

    return !strncmp(closing_tag, &p->input[p->cursor], closing_tag_len);
}

bool comment_tag(parser* p) {
    return !strncmp("<!--", &p->input[p->cursor], 4);
}

element_t parse_comment(parser* p) {
    p->cursor += 4;

    size_t start = p->cursor;

    while (strncmp("-->", &p->input[p->cursor], 3)) {
        parser_get(p);
    }

    element_t comment = new_comment(&p->input[start], p->cursor - start);
    p->cursor += 3;
    return comment;
}

void print_indent(size_t depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
}

void in_order_traversal(element_t root, vector_element_t* elements) {
    switch (get_type(root)) {
    case COMMENT:
    case PLAIN_TEXT: {
        vector_element_t_pb(elements, root);
        break;
    }
    case OPENING_TAG: {
        // Opening tag
        vector_element_t_pb(elements, root);

        // All Children
        for (int i = 0; i < get_size(root); i++) {
            in_order_traversal(root.e.e.children->array[i], elements);
        }

        // Closing tag
        element_t closing_tag = new_closing_tag(get_element_type(root));
        vector_element_t_pb(elements, closing_tag);
        break;
    }
    default:
        break;
    }
}

// In order dfs
element_t next_element(vector_element_t elements) {
    static int pos = 0;

    if (pos >= elements.len) {
        pos = 0;
        return (element_t){};
    }

    return elements.array[pos++];
}

void tree_free(vector_element_t elements) {
    element_t thing;

    while (get_type(thing = next_element(elements)) != NO_CONTENT) {
        switch (get_type(thing)) {
        case OPENING_TAG:
            plist_free(((element*)&thing)->properties);
        case PLAIN_TEXT:
        case CLOSING_TAG:
        case COMMENT:
        case NO_CONTENT:
            break;
        }
    }
}

vector_element_t traverse_tree(element_t root, size_t depth) {
    vector_element_t elements = vector_element_t_init(20);
    in_order_traversal(root, &elements);

    assert(elements.len > 0);

    element_t thing;

    for (int i = 0; i < elements.len; i++) {
        thing = elements.array[i];
        switch (get_type(thing)) {
        case NO_CONTENT:
            break;
        case COMMENT:
            print_indent(depth);
            printf("<!--%*s-->\n", (int)thing.e.tc.content.len,
                   thing.e.tc.content.array);
            break;
        case PLAIN_TEXT: {
            print_indent(depth);
            printf("%*s\n", (int)thing.e.tc.content.len,
                   thing.e.tc.content.array);
            break;
        }
        case OPENING_TAG:
            print_indent(depth);
            depth += 4;
            printf("<%s", get_element_type(thing));
            if (((element*)&thing)->properties->size > 0) {
                for (size_t i = 0; i < ((element*)&thing)->properties->size;
                     i++) {
                    kv_pair kv =
                        plist_get_pair(((element*)&thing)->properties, i);
                    printf(" %s=\"%s\"", kv.key, kv.val);
                }
            }
            printf(">\n");

            break;
        case CLOSING_TAG:
            depth -= 4;
            print_indent(depth);
            printf("</%s>\n", get_element_type(thing));
            break;
        }
    }

    /* tree_free(elements); */
    return elements;
}

parser parser_from_stdin() {
    char* buffer = calloc(1, 1);
    size_t total_len = 0;

    char* line;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, stdin)) != -1) {
        buffer = realloc(buffer, total_len + nread);
        strncpy(&buffer[total_len], line, nread);
        total_len += nread;
    }

    free(line);

    buffer[total_len - 1] = '\0';
    assert(strlen(buffer) == total_len - 1);
    return (parser){
        .input = buffer, .next = buffer[0], .cursor = 0, .len = total_len - 1};
}

schema parse_schema(){};

VECTOR(int);

int main() {
    vector_int v = {.array = calloc(4, sizeof(int)), .len = 0, .cap = 4};
    vector_int_pb(&v, 4);

    parser p = parser_from_stdin();

    initialize_element_types();

    element_t elem = parse_element(&p);

    traverse_tree(elem, 0);

    field test_emoji = {.type = string,
                        .name = "Emoji",
                        .required = true,
                        .default_value = NULL};

    field test_count = {
        .type = i32, .name = "Count", .required = false, .default_value = "0"};

    field test_author = {.type = string, .name = "Author", .required = true};

    field test_message_sent = {
        .type = timestamp, .name = "Message Sent", .required = true};

    field test_likes = {.type = i32, .name = "Likes"};

    field test_body = {.type = string, .name = "Body", .required = true};

    field test_reactions = {
        .type = substruct, .name = "Reactions", .substruct_type = "reaction"};

    field reaction_fields[] = {test_emoji, test_count};

    message test_reaction = {.fields = vector_field_from(reaction_fields, 2),
                             .name = "Reaction"};

    field chat_message_fields[] = {test_author, test_message_sent, test_likes,
                                   test_body, test_reactions};

    message test_chat_message = {.fields =
                                     vector_field_from(chat_message_fields, 5),
                                 .name = "Chat Message"};

    message schema_messages[] = {test_reaction, test_chat_message};

    schema test_schema = {.messages = vector_message_from(schema_messages, 2),
                          .version = "1.0.0"};

    schema parsed_schema = parse_schema(elem);

    parser_free(p);
}
