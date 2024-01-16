#include "buffer.h"
#include "buffer_string.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *generic_print_macro =
    "#define print_generic(value) _Generic((value), \\\n"
    "   char: \"%c\", \\\n"
    "   char*: \"%s\", \\\n"
    "   const char*: \"%s\", \\\n"
    "   float: \"%.2f\", \\\n"
    "   double: \"%.2f\", \\\n"
    "   signed char: \"%d\", \\\n"
    "   unsigned char: \"%u\", \\\n"
    "   signed short: \"%d\", \\\n"
    "   unsigned short: \"%u\", \\\n"
    "   signed int: \"%d\", \\\n"
    "   unsigned int: \"%u\", \\\n"
    "   signed long: \"%ld\", \\\n"
    "   unsigned long: \"%lu\", \\\n"
    "   signed long long: \"%lld\", \\\n"
    "   unsigned long long: \"%llu\" \\\n"
    ")\n"
    "\n"
    "#define print_value_generic(buf, value) ({ \\\n"
    "    int length = snprintf(NULL, 0, print_generic(value), value); \\\n"
    "    buffer_resize(&buf, length); \\\n"
    "    buf.len += sprintf(&buf.buffer[buf.len], print_generic(value), "
    "value); \\\n"
    "    buf.buffer[buf.len] = '\\0'; \\\n"
    "})\n"
    "#define print_value(buf, fmt, value) ({ \\\n"
    "    int length = snprintf(NULL, 0, fmt, value); \\\n"
    "    buffer_resize(&buf, length); \\\n"
    "    buf.len += sprintf(&buf.buffer[buf.len], fmt, value); \\\n"
    "    buf.buffer[buf.len] = '\\0'; \\\n"
    "})\n";

const char *WARNING = "// THIS IS A GENERATED FILE DO NOT EDIT";

const char *indent_level = "\t\t\t\t\t\t\t\t";
#define MAX_INDENT 8

const char *get_indent(int level) { return &indent_level[MAX_INDENT - level]; }

#define OUTPUT_BUF_SIZE 10000

void trim_newline(char *line, int len) { line[len - 1] = '\0'; }

// convert_filename removes non-alphabetic chars replacing them with `_`
// Returns a new string with the converted name, must be freed
char *convert_filename(const char *const filename) {
  char *converted = strdup(filename);
  for (int i = 0; converted[i]; i++) {
    if (!((converted[i] >= 'a' && converted[i] <= 'z') ||
          (converted[i] >= 'A' && converted[i] <= 'Z'))) {
      converted[i] = '_';
    }
  }

  return converted;
}

// code_line_p will check for a line starting with `@`
// Returns the position just after the found `@`
// or -1 if the line does not begin in `@`
int code_line_p(char *line) {
  int pos = 0;
  while (line[pos] == ' ' || line[pos] == '\t') {
    ++pos;
  }
  if (!line[pos] || line[pos] != '@') {
    return -1;
  } else {
    return pos + 1; // One past the `@`
  }
}

bool include_p(char *line) { return strncmp(line, "include", 7) == 0; }

buffer template_to_macro(const char *const file, int indent_level,
                         bool include) {
  FILE *fp = fopen(file, "r");
  if (!fp) {
    if (errno == ENOENT) {
      fprintf(stderr, "error: File \"%s\" does not exist.\n", file);
    } else if (errno == EMFILE) {
      fprintf(stderr,
              "error: Too many open files, do you have a circular import?\n");
    } else {
      perror("fopen");
    }
    return (buffer){.buffer = NULL, .len = 0, .cap = 0};
  }

  buffer macro_buffer = (buffer){.buffer = malloc(4096), .len = 0, .cap = 4096};

  if (!include) {
    char *const filename = convert_filename(file);
    buffer_write(&macro_buffer, "// GENERATED FUNCTION from file %s\n",
                 filename);
    buffer_strcpy(
        &macro_buffer,
        "// Memory returned by %s is heap allocated, and must be freed\n");
    buffer_write(&macro_buffer, "#define %s()({\\\n", filename);
    buffer_strcpy(&macro_buffer, "\tbuffer output_buffer = (buffer){.buffer = "
                                 "malloc(4096), .len = 0, .cap = 4096};\\\n");
    buffer_strcpy(&macro_buffer, "\tdo {\\\n");
    free(filename);
  }

  char *input_line = NULL;
  size_t size;
  size_t line_number = 0;
  int read = 0;
  int pos = 0;
  const char *indent = get_indent(indent_level);

  while ((read = getline(&input_line, &size, fp)) != -1) {
    char *line = input_line;
    trim_newline(line, read);
    line_number++;

    if ((pos = code_line_p(line)) != -1) {
      if (include_p(&line[pos])) {
        size_t include_indent =
            (&line[pos] - line) / 4; // Calculate indentation
        line += pos + 9;             // skip past @include{"
        strtok(line, "\"");          // null terminate filename

        buffer parsed_file =
            template_to_macro(line, indent_level + include_indent, true);
        if (!parsed_file.buffer) {
          fprintf(stderr, "error: %10s:%-5ld failed while importing \"%s\"\n",
                  file, line_number, line);
          free(input_line);
          free(macro_buffer.buffer);
          return (buffer){.buffer = NULL, .len = 0, .cap = 0};
        }
        buffer_append(&macro_buffer, &parsed_file);
        free(parsed_file.buffer);
        continue;
      }
      buffer_write(&macro_buffer, "\t\t%s \\\n", &line[pos]);
      continue;
    }

    buffer_write(&macro_buffer,
                 "\t\tbuffer_write(&output_buffer, \"%%s\", \"%s\"); \\\n",
                 indent);

    char temp[500] = {0};
    int temp_pos = 0;
    bool inside_variable = false;
    while (*line) {
      if (*line == '$' && *(line + 1) && *(line + 1) == '{') {
        inside_variable = true;
        // Write out line before variable decl
        if (temp_pos > 0) {
          buffer_write(
              &macro_buffer,
              "\t\tbuffer_write(&output_buffer, \"%%s\", \"%s\"); \\\n", temp);
          temp[0] = '\0';
          temp_pos = 0;
        }
        line += 2;
      } else if (inside_variable) {
        bool generic_variable = false;
        bool parsing_error = false;
        while (*line && *line != ':') {
          if (*line == '}' && strlen(temp) != 0) {
            generic_variable = true;
            break;
          }
          temp[temp_pos++] = *line++;
        }
        if (generic_variable) {
          temp[temp_pos++] = '\0';
          buffer_write(&macro_buffer,
                       "\t\tprint_value_generic(output_buffer, %s); \\\n",
                       temp);
          temp[0] = '\0';
          temp_pos = 0;
          line++;
          inside_variable = false;
          continue;
        }
        temp[temp_pos++] = '\0';
        if (!*line) {
          parsing_error = true;
        }
        line++;
        char *fmt = &temp[temp_pos];
        while (*line && *line != '}' && !parsing_error) {
          temp[temp_pos++] = *line++;
        }
        temp[temp_pos] = '\0';
        if (!*line || strlen(fmt) == 0) {
          parsing_error = true;
        }
        if (parsing_error) {
          temp[temp_pos] = '\0';
          fprintf(stderr,
                  "error: %10s:%-5ld malformed variable block around \"%s\" "
                  "hint: ${variable:%%s} \n",
                  file, line_number, temp);
          free(macro_buffer.buffer);
          free(input_line);
          return (buffer){.buffer = NULL, .len = 0, .cap = 0};
        }
        buffer_write_var(&macro_buffer,
                         "\t\tprint_value(output_buffer, \"%s\", %s); \\\n",
                         fmt, temp);
        temp[0] = '\0';
        temp_pos = 0;
        inside_variable = false;
        line++;
      } else {
        if (*line == '"') {
          temp[temp_pos++] = '\\';
        }
        temp[temp_pos++] = *line++;
        temp[temp_pos] = '\0';
      }
    }
    temp[temp_pos] = '\0';
    buffer_write(&macro_buffer,
                 "\t\tbuffer_write(&output_buffer, \"%%s\\n\", \"%s\"); \\\n",
                 temp);
  }

  if (!include) {
    buffer_strcpy(&macro_buffer, "\t} while (0); \\\n");
    buffer_strcpy(&macro_buffer, "\toutput_buffer.buffer;\\\n");
    buffer_strcpy(&macro_buffer, "})");
  }

  free(input_line);
  fclose(fp);
  return macro_buffer;
}

const char *usage = "usage: cats files...";

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("%s\n", usage);
    exit(0);
  }
  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    printf("%s\n",
           "cats (C Auto Template System)\nPass templates files individually "
           "or with *.html/*.txt\ngithub.com/jacksonmowry/cats\n");
    exit(0);
  }

  FILE *header = fopen("buffer.h", "w");
  fprintf(header, "%.*s\n", buffer_h_len, buffer_h);
  fclose(header);

  FILE *fp = fopen("templates.h", "w");
  if (!fp) {
    perror("fopen");
    exit(1);
  }

  fprintf(fp, "%s\n", "#include <stdio.h>");
  fprintf(fp, "%s\n", "#include <stdlib.h>");
  fprintf(fp, "%s\n", "#include \"buffer.h\"");
  fprintf(fp, "%s\n", generic_print_macro);

  for (int i = 1; i < argc; i++) {
    buffer output = template_to_macro(argv[i], 0, false);
    if (!output.buffer) {
      fprintf(stderr, "error: %16s failed while creating template for %s\n",
              " ", argv[i]);
      return 1;
    }
    fprintf(fp, "%s\n", output.buffer);
    free(output.buffer);
  }
}
