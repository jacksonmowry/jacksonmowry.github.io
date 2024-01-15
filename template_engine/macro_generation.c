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
    "#define print_value(buf, value) ({ \\\n"
    "    int result = sprintf(buf, print_generic(value), value);\\\n"
    "    result; \\\n"
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

char *template_to_macro(const char *const file, int indent_level,
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
    return NULL;
  }

  char *macro_buffer = calloc(1, 10000);
  char *mb_ptr = macro_buffer;

  if (!include) {
    char *const filename = convert_filename(file);
    mb_ptr += sprintf(mb_ptr, "// GENERATED FUNCTION from file %s\n", file);
    mb_ptr += sprintf(
        mb_ptr,
        "// Memory returned by %s is heap allocated, and must be freed\n",
        filename);
    mb_ptr += sprintf(mb_ptr, "#define %s()({\\\n", filename);
    mb_ptr += sprintf(mb_ptr, "\tchar *output_buffer = malloc(%d);\\\n",
                      OUTPUT_BUF_SIZE);
    mb_ptr += sprintf(mb_ptr, "\tdo {\\\n");
    mb_ptr += sprintf(mb_ptr, "\t\tchar *ob_ptr = output_buffer;\\\n");
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

        char *parsed_file =
            template_to_macro(line, indent_level + include_indent, true);
        if (!parsed_file) {
          fprintf(stderr, "error: %10s:%-5ld failed while importing \"%s\"\n",
                  file, line_number, line);
          free(input_line);
          free(macro_buffer);
          return NULL;
        }
        mb_ptr = stpcpy(mb_ptr, parsed_file);
        free(parsed_file);
        continue;
      }
      mb_ptr += sprintf(mb_ptr, "\t\t%s \\\n", &line[pos]);
      continue;
    }

    mb_ptr += sprintf(
        mb_ptr, "\t\tob_ptr += sprintf(ob_ptr, \"%%s\", \"%s\"); \\\n", indent);
    char temp[500] = {0};
    int temp_pos = 0;
    bool inside_variable = false;
    while (*line) {
      if (*line == '$' && *(line + 1) && *(line + 1) == '{') {
        inside_variable = true;
        // Write out line before variable decl
        if (temp_pos > 0) {
          mb_ptr += sprintf(
              mb_ptr, "\t\tob_ptr += sprintf(ob_ptr, \"%%s\", \"%s\"); \\\n",
              temp);
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
          mb_ptr += sprintf(
              mb_ptr, "\t\tob_ptr += print_value(ob_ptr, %s); \\\n", temp);
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
          free(macro_buffer);
          free(input_line);
          return NULL;
        }
        mb_ptr +=
            sprintf(mb_ptr, "\t\tob_ptr += sprintf(ob_ptr, \"%s\", %s); \\\n",
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
    mb_ptr += sprintf(mb_ptr,
                      "\t\tob_ptr += sprintf(ob_ptr, \"%%s\\n\", \"%s\"); \\\n",
                      temp);
  }

  if (!include) {
    mb_ptr += sprintf(mb_ptr, "\t} while (0); \\\n");
    mb_ptr += sprintf(mb_ptr, "\toutput_buffer;\\\n");
    mb_ptr += sprintf(mb_ptr, "})");
  }

  free(input_line);
  fclose(fp);
  return macro_buffer;
}

int main(int argc, char **argv) {
  FILE *fp = fopen("templates.h", "w");
  if (!fp) {
    perror("fopen");
    exit(1);
  }

  fprintf(fp, "%s\n", "#include <stdio.h>");
  fprintf(fp, "%s\n", "#include <stdlib.h>");
  fprintf(fp, "%s\n", generic_print_macro);

  for (int i = 1; i < argc; i++) {
    char *output = template_to_macro(argv[i], 0, false);
    if (!output) {
      fprintf(stderr, "error: %16s failed while creating template for %s\n",
              " ", argv[i]);
      return 1;
    }
    fprintf(fp, "%s\n", output);
    free(output);
  }
}
