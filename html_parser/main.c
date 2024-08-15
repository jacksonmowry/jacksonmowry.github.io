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

void set_content(text_content* tc, const char* content, uint16_t length) {
    tc->content = vector_char_from(content, length);
}

const char* get_element_type(element* element) {
    return element_types->types[element->tag_size.element_type];
}

element_t new_plain_content(char* text, size_t size) {
    if (size == 0) {
        element_t no_content;
        set_type(&no_content, NO_CONTENT);
        return no_content;
    }
    element_t text_content;
    set_type(&text_content, PLAIN_TEXT);
    set_size(&text_content, size);
    set_cap(&text_content, size);
    set_content(tc, text, size);
    return tc;
}
element* new_element(char* type) {
    element* elem = calloc(1, sizeof(element) + sizeof(element*) * 1);
    set_type((tag_size*)elem, OPENING_TAG);
    int16_t tag_type = add_element_type(type, strlen(type));
    elem->tag_size.element_type = tag_type;
    set_size((tag_size*)elem, 0);
    set_cap((tag_size*)elem, 1);
    elem->properties = plist_init();
    return elem;
}
tag_size new_closing_tag(const char* type) {
    tag_size closing;
    set_type(&closing, CLOSING_TAG);
    int16_t tag_type = add_element_type(type, strlen(type));
    closing.element_type = tag_type;
    return closing;
}

text_content* new_comment(const char* comment, size_t size) {
    text_content* tc = calloc(1, sizeof(tag_size) + size + 1);
    set_type((tag_size*)tc, COMMENT);
    set_size((tag_size*)tc, size);
    set_cap((tag_size*)tc, size);
    set_content(tc, comment, size);
    return tc;
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

void add_child_element(element** parent, element* child) {
    if (get_size((tag_size*)(*parent)) >= get_cap((tag_size*)(*parent))) {
        *parent = (element*)grow_tag_size((tag_size*)(*parent));
    }

    (*parent)->children[get_size((tag_size*)(*parent))] = child;
    set_size((tag_size*)(*parent), get_size((tag_size*)(*parent)) + 1);
}

element* parse_tag(parser* p) {
    if (parser_peek(p) != '<') {
        return NULL;
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
        return NULL;
    }
    element* new_elem = new_element(temp);

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

        plist_add(new_elem->properties, key, value);
    }
    assert(parser_peek(p) == '>');

    parser_get(p);
    return new_elem;
}

// parse_contents will look at the inner xml of an element and parse it
// depending on if it is plain text of a child element.
// In either case contents will be appended to their parents children
element* parse_contents(parser* p, element** parent) {
    // If the first thing we see is '<' we have a child element
    // Otherwise we have a string (plain text)
    if (comment_tag(p)) {
        text_content* comment = parse_comment(p);
        add_child_element(parent, (element*)comment);
        return (element*)comment;
    } else if (parser_peek(p) == '<') {
        element* child = parse_element(p);
        add_child_element(parent, child);
        return child;
    } else {
        text_content* plain_text = parse_text(p);
        add_child_element(parent, (element*)plain_text);
        return (element*)plain_text;
    }
}

text_content* parse_text(parser* p) {
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

    return new_thing_plain_content(&p->input[start],
                                   last_nonwhitespace - start + 1);
}

element* parse_element(parser* p) {
    element* new_element = parse_tag(p);
    size_t closing_tag_len = strlen(get_element_type(new_element)) + 4;
    char closing_tag[closing_tag_len];
    memset(closing_tag, 0, sizeof(closing_tag));
    strcat(closing_tag, "</");
    strcat(closing_tag, get_element_type(new_element));
    strcat(closing_tag, ">");

    // While I am not looking at my own closing tag
    while (!closing_tag_p(closing_tag, closing_tag_len - 1, p)) {
        parse_contents(p, &new_element);
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

text_content* parse_comment(parser* p) {
    p->cursor += 4;

    size_t start = p->cursor;

    while (strncmp("-->", &p->input[p->cursor], 3)) {
        parser_get(p);
    }

    text_content* comment = new_comment(&p->input[start], p->cursor - start);
    p->cursor += 3;
    return comment;
}

void print_indent(size_t depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
}

void in_order_traversal(element* root, vector_tag_size* tsv) {
    switch (get_type((tag_size*)root)) {
    case COMMENT:
    case PLAIN_TEXT: {
        vector_tag_size_pb(tsv, *(tag_size*)root);
        break;
    }
    case OPENING_TAG: {
        // Opening tag
        vector_tag_size_pb(tsv, *(tag_size*)root);

        // All Children
        for (int i = 0; i < get_size((tag_size*)root); i++) {
            in_order_traversal(root->children[i], tsv);
        }

        // Closing tag
        tag_size closing_tag = new_closing_tag(get_element_type(root));
        vector_tag_size_pb(tsv, closing_tag);
        break;
    }
    default:
        break;
    }
}

// In order dfs
tag_size next_element(vector_tag_size tsv) {
    static int pos = 0;

    if (pos >= tsv.len) {
        pos = 0;
        return (tag_size){};
    }

    return tsv.array[pos++];
}

void tree_free(vector_tag_size tsv) {
    tag_size thing;

    while ((thing = next_element(tsv)).cap != 0) {
        switch (get_type(&thing)) {
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

vector_tag_size traverse_tree(element* root, size_t depth) {
    vector_tag_size tsv = vector_tag_size_init(20);
    in_order_traversal(root, &tsv);

    assert(tsv.len > 0);

    tag_size thing;

    while ((thing = next_element(tsv)).cap != 0) {
        switch (get_type(&thing)) {
        case NO_CONTENT:
            break;
        case COMMENT:
            print_indent(depth);
            printf("<!--%*s-->\n", get_size(&thing),
                   get_content((text_content*)&thing));
            break;
        case PLAIN_TEXT: {
            print_indent(depth);
            printf("%s\n", get_content((text_content*)&thing));
            break;
        }
        case OPENING_TAG:
            print_indent(depth);
            depth += 4;
            printf("<%s", get_element_type((element*)&thing));
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
            printf("</%s>\n", get_element_type((element*)&thing));
            break;
        }
    }

    tree_free(tsv);
    return tsv;
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

    element* elem = parse_element(&p);

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
