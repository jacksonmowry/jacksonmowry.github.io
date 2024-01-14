#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *WARNING = "// THIS IS A GENERATED FILE DO NOT EDIT";

const char *indent_level = "\t\t\t\t\t\t\t\t";
#define MAX_INDENT 8

const char *get_indent(int level) { return &indent_level[MAX_INDENT - level]; }

typedef struct parameter {
  char *type;
  char *name;
} parameter;

typedef struct parameter_set {
  size_t size;
  parameter *params;
} parameter_set;

// attempts to insert a new parameter, returns the collision if one is found, or
// null if it is unique
parameter *ps_insert(parameter_set *ps, parameter p) {
  for (int i = 0; i < ps->size; i++) {
    if (strcmp(p.name, ps->params[i].name) == 0) {
      if (strcmp(p.type, ps->params[i].type) != 0) {
        return &ps->params[i];
      }
      free(p.name);
      free(p.type);
      return NULL;
    }
  }

  ps->params[ps->size++] = p;
  return NULL;
}

char *param_type(char *format_string) {
  while (!(*format_string >= 'a' && *format_string <= 'z')) {
    format_string++;
  }
  if (strcmp(format_string, "c") == 0) {
    return strdup("char");
  } else if (strcmp(format_string, "s") == 0) {
    return strdup("const char*");
  } else if (strcmp(format_string, "f") == 0) {
    return strdup("double");
  } else if (strcmp(format_string, "d") == 0) {
    return strdup("int");
  } else if (strcmp(format_string, "u") == 0) {
    return strdup("unsigned int");
  } else if (strcmp(format_string, "ld") == 0) {
    return strdup("signed long");
  } else if (strcmp(format_string, "lu") == 0) {
    return strdup("unsigned long");
  } else if (strcmp(format_string, "lld") == 0) {
    return strdup("signed long long");
  } else if (strcmp(format_string, "llu") == 0) {
    return strdup("unsigned long long");
  } else {
    return NULL;
  }
}

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

// Returns a new buffer with the function body
// Must be freed
char *template_to_function(const char *const file, parameter_set *p_set,
                           int indent_level, bool include) {
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

  char *function_buffer = calloc(sizeof(char), 10000);
  char *fb_ptr = function_buffer;
  if (!function_buffer) {
    perror("calloc");
    return NULL;
  }

  // Get current indentation
  const char *indent = get_indent(indent_level);

  if (!include) {
    fb_ptr += sprintf(fb_ptr, "\tchar *output_buffer = (char*)malloc(4096);\n");
    fb_ptr += sprintf(fb_ptr, "\tchar *output_ptr = output_buffer;\n");
  }

  char *input_line = NULL;
  size_t len = 0;
  ssize_t read;
  size_t line_number = 0;
  int pos;

  while ((read = getline(&input_line, &len, fp)) != -1) {
    line_number++;
    char *line = input_line;
    line[read - 1] = '\0'; // Removing the trailing newline

    // Handle @ symbol
    if ((pos = code_line_p(line)) != -1) {
      if (include_p(&line[pos])) { // line[pos] is the first char after `@`
        // Include another template
        size_t include_indent =
            (&line[pos] - line) / 4; // Calculate indentation level
        line += pos + 9;             // Skip past @inc...
        strtok(line, "\"");          // null terminate filename
        char *parsed_include = template_to_function(
            line, p_set, indent_level + include_indent, true);
        if (!parsed_include) {
          fprintf(stderr, "error: %10s:%-5ld failed while importing \"%s\"\n",
                  file, line_number, line);
          free(function_buffer);
          return NULL;
        }
        fb_ptr = stpcpy(fb_ptr, parsed_include);
        free(parsed_include);
        continue;
      }

      // Line of code, append it!
      fb_ptr += sprintf(fb_ptr, "\t%s\n", line + pos);
      continue;
    }

    // Handle Line
    // Indent line to correct level
    fb_ptr += sprintf(fb_ptr,
                      "\toutput_ptr += sprintf(output_ptr, \"%%s\", \"%s\");\n",
                      indent);
    char temporary_buffer[500];
    int tb_len = 0;
    bool inside_variable = false;
    while (*line) {
      if (*line == '$' && *(line + 1) && *(line + 1) == '{') {
        // Found a variable declaration
        inside_variable = true;
        // Flush the buffer if it has something in it
        if (tb_len > 0) {
          fb_ptr += sprintf(
              fb_ptr, "\toutput_ptr += sprintf(output_ptr, \"%%s\", \"%s\");\n",
              temporary_buffer);
          temporary_buffer[0] = '\0';
          tb_len = 0;
        }
        line += 2; // Skip past `${`
      } else if (inside_variable) {
        bool local_variable = false;
        if (*line == '$') {
          local_variable = true;
          ++line;
        }
        while (*line && *line != ':') {
          if (*line == '}' || *line == '%') {
            // Missing format string or colon
            temporary_buffer[tb_len] = '\0';
            fprintf(
                stderr,
                "error: %10s:%-5ld missing fmt specifer or colon for \"%s\"\n",
                file, line_number, temporary_buffer);
            free(function_buffer);
            return NULL;
          }
          temporary_buffer[tb_len++] = *line++;
        }
        line++; // skip past `:`
        if (!*line || *line == '}') {
          // Missing format string or colon
          temporary_buffer[tb_len] = '\0';
          fprintf(stderr,
                  "error: %10s:%-5ld malformed variable block around \"%s\" "
                  "hint: ${variable:%%s} \n",
                  file, line_number, temporary_buffer);
          free(function_buffer);
          return NULL;
        }
        temporary_buffer[tb_len++] = '\0';
        char *fmt_string =
            &temporary_buffer[tb_len]; // Beginning of %s or %d etc.
        while (*line != '}') {
          temporary_buffer[tb_len++] = *line++;
        }
        temporary_buffer[tb_len++] = '\0';
        // Add to p_set if non-local
        if (!local_variable) {
          char *type = param_type(fmt_string);
          char *name = strdup(temporary_buffer);

          if (!type) {
            // invalid format string
            fprintf(
                stderr,
                "error: %10s:%-5ld invalid format specifier \"%s\" for %s\n",
                file, line_number, fmt_string, name);
            free(function_buffer);
            return NULL;
          }

          parameter *possible_collision = NULL;
          if ((possible_collision =
                   ps_insert(p_set, (parameter){.type = type, .name = name}))) {
            // A type collision was found
            fprintf(stderr,
                    "error: %10s:%-5ld duplicate param (%s) w/ mismatched "
                    "type \"%s\" and \"%s\"\n",
                    file, line_number, name, type, possible_collision->type);
            free(function_buffer);
            return NULL;
          }
        }
        // Generate code
        fb_ptr += sprintf(fb_ptr,
                          "\toutput_ptr += sprintf(output_ptr, \"%s\", %s);\n",
                          fmt_string, temporary_buffer);
        temporary_buffer[0] = '\0';
        tb_len = 0;
        inside_variable = false;
        line++;
      } else {
        if (*line == '"') {
          temporary_buffer[tb_len++] = '\\';
        }
        temporary_buffer[tb_len++] = *line;
        temporary_buffer[tb_len] = '\0';
        line++;
      }
    }
    temporary_buffer[tb_len] = '\0';
    fb_ptr += sprintf(
        fb_ptr, "\toutput_ptr += sprintf(output_ptr, \"%%s\\n\", \"%s\");\n",
        temporary_buffer);
  }

  free(input_line);

  if (!include) {
    fb_ptr += sprintf(fb_ptr, "\treturn output_buffer;\n");
  }
  fclose(fp);
  return function_buffer;
}

// generate_function generates a function from the template provided in
// `filename`, it's contents are written to `output_fd`
void generate_function(const char *const filename, FILE *output_fd) {
  parameter_set p_set = {.size = 0, .params = (parameter[32]){0}};

  char *function_name = convert_filename(filename);
  char *template_function = template_to_function(filename, &p_set, 0, false);
  if (!template_function) {
    return;
  }

  fprintf(output_fd, "char *%s(", function_name);
  for (int i = 0; i < p_set.size; i++) {
    fprintf(output_fd, "%s %s", p_set.params[i].type, p_set.params[i].name);
    free(p_set.params[i].type);
    free(p_set.params[i].name);
    if (i != p_set.size - 1) {
      fprintf(output_fd, ", ");
    }
  }
  fprintf(output_fd, ") {\n");
  fprintf(output_fd, "%s", template_function);
  fprintf(output_fd, "}");

  free(template_function);
  free(function_name);
}

int main(int argc, char **argv) {
  FILE *output_header = fopen("gen_templates.h", "w");
  if (!output_header) {
    perror("fopen");
    fprintf(stderr, "Unable to open header file for writing\n");
    exit(1);
  }

  fprintf(output_header, "%s\n", WARNING);
  fprintf(output_header, "#pragma once\n");
  fprintf(output_header, "#include <stdio.h>\n");
  fprintf(output_header, "#include <stdlib.h>\n\n");

  for (int i = 1; i < argc; i++) {
    generate_function(argv[i], output_header);
  }

  fclose(output_header);
}
