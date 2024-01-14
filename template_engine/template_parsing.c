#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *header_warnings =
    "//DO NOT EDIT, FILE IS AUTOMATICALLY GENERATED\n"
    "//EDIT TEMPLATE FILES INSTEAD\n"
    "// #include \"templates.h\" in your .c file to use\n"
    "\n"
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "\n";

const char *generic_print_macro =
    "#define print_generic(value) _Generic((value), \\\n"
    "    char: \"%c\", \\\n"
    "    char*: \"%s\", \\\n"
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

#define RED_BOLD "\033[1;31m"
#define RESET "\033[0m"

typedef struct parameter {
  char *type;
  char *name;
} parameter;

typedef struct parameter_set {
  size_t param_count;
  parameter *parameters;
} parameter_set;

// Attemps to insert new parameter in parameter_set
// Returns NULL on duplicate insert to indicate success
// Returns parameter* on type mismatch to show mismatch
parameter *ps_insert(parameter_set *ps, parameter p) {
  for (int i = 0; i < ps->param_count; i++) {
    if (strcmp(ps->parameters[i].name, p.name) == 0) {
      // Duplicate name, check for mismatched type
      if (strcmp(ps->parameters[i].type, p.type) != 0) {
        return &ps->parameters[i];
      } else {
        free(p.name);
        free(p.type);
        return NULL;
      }
    }
  }
  // No duplicate
  ps->parameters[ps->param_count++] = p;
  return NULL;
}

char *convert_filename(const char *filename) {
  char *converted = strdup(filename);
  for (int i = 0; converted[i]; i++) {
    if (!((converted[i] >= 'a' && converted[i] <= 'z') ||
          (converted[i] >= 'A' && converted[i] <= 'Z'))) {
      converted[i] = '_';
    }
  }

  return converted;
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

void trim_newline(char *line, int len) { line[len - 1] = '\0'; }

int code_line_p(char *line) {
  int pos = 0;
  while (*line == ' ' || *line == '\t') {
    ++line;
    ++pos;
  }
  if (!*line) {
    return -1;
  } else if (*line == '@') {
    return pos + 1;
  } else {
    return -1;
  }
}

bool include_p(char *line) { return strncmp(line, "include", 7) == 0; }

char *template_to_function(const char *const file, parameter_set *params,
                           bool include) {
  FILE *fp = fopen(file, "r");
  if (!fp) {
    if (errno == ENOENT) {
      fprintf(stderr, "%serror%s: File \"%s\" does not exist.\n", RED_BOLD,
              RESET, file);
    } else if (errno == EMFILE) {
      fprintf(
          stderr,
          "%serror%s: Too many open files, do you have a circular import?\n",
          RED_BOLD, RESET);
    } else {
      perror("fopen");
    }
    return NULL;
  }

  size_t cur_size = 4096;
  char *new_func = calloc(1, cur_size);
  char *buf_ptr = new_func;
  if (!new_func) {
    perror("calloc");
    return NULL;
  }

  if (!include) {
    buf_ptr += sprintf(buf_ptr, "\tchar *buf = (char*)malloc(4096);\n");
    buf_ptr += sprintf(buf_ptr, "\tchar *buf_ptr = buf;\n");
  }

  char *line_input = NULL;
  size_t len = 0;
  ssize_t read;
  int pos;
  size_t line_number = 0;

  while ((read = getline(&line_input, &len, fp)) != -1) {
    char *line = line_input;
    line_number++;
    trim_newline(line, read);
    if ((pos = code_line_p(line)) != -1) {
      if (include_p(&line[pos])) {
        // Include other file
        line += pos + 9;    // skip past @inc..
        strtok(line, "\""); // null terminate filename
        // line now contains filename
        char *parsed_file = template_to_function(line, params, true);
        if (!parsed_file) {
          fprintf(stderr,
                  "%serror%s: %10s:%-5ld failed while importing \"%s\"\n",
                  RED_BOLD, RESET, file, line_number, line);
          return NULL;
        }
        buf_ptr = stpcpy(buf_ptr, parsed_file);
        free(parsed_file);
        continue;
      }
      buf_ptr += sprintf(buf_ptr, "\t%s\n", line + pos);
      continue;
    }
    // buf_ptr += \tsprintf(buf_ptr, \"%%s\", \"%s\");\n
    char tmp[500] = {0};
    int tmp_pos = 0;
    bool inside_var = false;
    while (*line) {
      if (*line == '$' && *(line + 1) == '{') {
        // Write out everything before the var decl
        if (tmp_pos > 0) {
          buf_ptr +=
              sprintf(buf_ptr,
                      "\tbuf_ptr += sprintf(buf_ptr, \"%%s\", \"%s\");\n", tmp);
          tmp[0] = '\0';
          tmp_pos = 0;
        }
        inside_var = true;
        line += 2;
      } else if (inside_var) {
        bool local_var = false;
        if (*line == '$') {
          local_var = true;
          line++;
        }
        while (*line != ':') {
          if (*line == '}') {
            tmp[tmp_pos] = '\0';
            fprintf(stderr,
                    "%serror%s: %10s:%-5ld, fmt specifer required "
                    "for \"%s\"\n",
                    RED_BOLD, RESET, file, line_number, tmp);
            return NULL;
          }
          tmp[tmp_pos++] = *line++;
        }
        tmp[tmp_pos++] = '\0';
        line++; // skip past `:`
        char *fmt = &tmp[tmp_pos];
        while (*line != '}') {
          tmp[tmp_pos++] = *line++;
        }
        tmp[tmp_pos++] = '\0';
        // Add var to params if non-local
        if (!local_var) {
          char *type = param_type(fmt);
          if (!type) {
            fprintf(stderr,
                    "%serror%s: %10s:%-5ld invalid format specifier \"%s\"\n",
                    RED_BOLD, RESET, file, line_number, fmt);
            return NULL;
          }
          parameter p = {.type = type, .name = strdup(tmp)};
          parameter *ret;
          if ((ret = ps_insert(params, p))) {
            fprintf(stderr,
                    "%serror%s: %10s:%-5ld, duplicate param (%s) w/ mismatched "
                    "type "
                    "\"%s\" and \"%s\"",
                    RED_BOLD, RESET, file, line_number, tmp, p.type, ret->type);
            return NULL;
          }
        }
        // write out var
        buf_ptr += sprintf(
            buf_ptr, "\tbuf_ptr += sprintf(buf_ptr, \"%s\", %s);\n", fmt, tmp);
        tmp[0] = '\0';
        tmp_pos = 0;
        inside_var = false;
        line++;
      } else {
        if (*line == '"') {
          tmp[tmp_pos++] = '\\';
        }
        tmp[tmp_pos++] = *line;
        tmp[tmp_pos] = '\0';
        line++;
      }
    }
    tmp[tmp_pos] = '\0';
    buf_ptr += sprintf(
        buf_ptr, "\tbuf_ptr += sprintf(buf_ptr, \"%%s\\n\", \"%s\");\n", tmp);
  }
  if (!include) {
    buf_ptr += sprintf(buf_ptr, "\treturn buf;\n");
  }
  free(line_input);
  return new_func;
}

char *template_to_macro(const char *const file, bool include) {
  FILE *fp = fopen(file, "r");
  if (!fp) {
    if (errno == ENOENT) {
      fprintf(stderr, "%serror%s: File \"%s\" does not exist.\n", RED_BOLD,
              RESET, file);
    } else if (errno == EMFILE) {
      fprintf(
          stderr,
          "%serror%s: Too many open files, do you have a circular import?\n",
          RED_BOLD, RESET);
    } else {
      perror("fopen");
    }
    return NULL;
  }

  char *filename = convert_filename(file);

  size_t cur_size = 4096;
  char *new_macro = calloc(1, cur_size);
  char *buf_ptr = new_macro;
  if (!new_macro) {
    perror("calloc");
    return NULL;
  }

  if (!include) {
    buf_ptr += sprintf(buf_ptr, "#define %s() ({\\\n", filename);
    buf_ptr += sprintf(buf_ptr, "\tchar *buf = malloc(4096); \\\n");
    buf_ptr += sprintf(buf_ptr, "\tdo{ \\\n");
    buf_ptr += sprintf(buf_ptr, "\t\tchar *buf_ptr = buf; \\\n");
  }

  free(filename);

  char *line_input = NULL;
  size_t len = 0;
  size_t read;
  int pos;
  size_t line_number = 0;

  while ((read = getline(&line_input, &len, fp)) != -1) {
    line_number++;
    char *line = line_input;
    trim_newline(line, read);
    if ((pos = code_line_p(line)) != -1) {
      if (include_p(&line[pos])) {
        // Include other file
        line += pos + 9;    // skip past @include{"
        strtok(line, "\""); // null terminate filename
        // line now contains filename
        char *parsed_file = template_to_macro(line, true);
        if (!parsed_file) {
          fprintf(stderr,
                  "%serror%s: %10s:%-5ld failed while importing \"%s\"\n",
                  RED_BOLD, RESET, file, line_number, line);
          return NULL;
        }
        buf_ptr = stpcpy(buf_ptr, parsed_file);
        free(parsed_file);
        continue;
      }
      buf_ptr += sprintf(buf_ptr, "\t\t%s \\\n", line + pos);
      continue;
    }
    char tmp[500] = {0};
    int tmp_pos = 0;
    bool inside_var = false;
    while (*line) {
      if (*line == '$' && *(line + 1) == '{') {
        // write out everything before var decl
        if (tmp_pos > 0) {
          buf_ptr += sprintf(
              buf_ptr, "\t\tbuf_ptr += sprintf(buf_ptr, \"%%s\", \"%s\"); \\\n",
              tmp);
          tmp[0] = '\0';
          tmp_pos = 0;
        }
        inside_var = true;
        line += 2;
      } else if (inside_var) {
        bool no_fmt = false;
        while (*line != ':' && *line != '}') {
          if (*line == '$') {
            line++;
            continue;
          }
          tmp[tmp_pos++] = *line++;
        }
        tmp[tmp_pos++] = '\0';
        if (*line == '}') {
          no_fmt = true;
        }
        if (no_fmt) {
          buf_ptr += sprintf(
              buf_ptr, "\t\tbuf_ptr += print_value(buf_ptr, %s); \\\n", tmp);
        } else {
          ++line;
          char *fmt = &tmp[tmp_pos];
          while (*line != '}') {
            tmp[tmp_pos++] = *line++;
          }
          tmp[tmp_pos++] = '\0';
          // Write out var print
          buf_ptr += sprintf(
              buf_ptr, "\t\tbuf_ptr += sprintf(buf_ptr, \"%s\", %s); \\\n", fmt,
              tmp);
        }
        tmp[0] = '\0';
        tmp_pos = 0;
        inside_var = false;
        line++;
      } else {
        if (*line == '"') {
          tmp[tmp_pos++] = '\\';
        }
        tmp[tmp_pos++] = *line;
        tmp[tmp_pos] = '\0';
        line++;
      }
    }
    tmp[tmp_pos] = '\0';
    buf_ptr += sprintf(
        buf_ptr, "\t\tbuf_ptr += sprintf(buf_ptr, \"%%s\\n\", \"%s\"); \\\n",
        tmp);
  }

  if (!include) {
    buf_ptr += sprintf(buf_ptr, "\t} while (0);\\\n");
    buf_ptr += sprintf(buf_ptr, "\tbuf;\\\n");
    buf_ptr += sprintf(buf_ptr, "})\n");
  }
  free(line_input);
  return new_macro;
}

int main(int argc, char **argv) {
  bool function_mode = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--function") == 0 || strcmp(argv[i], "-f") == 0) {
      function_mode = true;
      break;
    }
  }
  FILE *header = fopen("templates.h", "w");
  if (!header) {
    perror("fopen");
    exit(69);
  }

  fprintf(header, "%s\n", header_warnings);
  if (!function_mode) {
    fprintf(header, "%s\n", generic_print_macro);
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-f") == 0) {
      continue;
    }
    if (function_mode) {
      char *filename = convert_filename(argv[i]);
      parameter params[32];
      parameter_set ps = {.param_count = 0, .parameters = params};
      char *func = template_to_function(argv[i], &ps, false);
      if (!func) {
        exit(EXIT_FAILURE);
      }
      fprintf(header, "char *%s(", filename);
      for (int i = 0; i < ps.param_count; i++) {
        fprintf(header, "%s %s", ps.parameters[i].type, ps.parameters[i].name);
        free(ps.parameters[i].type);
        free(ps.parameters[i].name);
        if (i != ps.param_count - 1) {
          fprintf(header, ", ");
        }
      }
      fprintf(header, ") {\n");

      fprintf(header, "%s\n", func);

      fprintf(header, "}\n");
      free(filename);
      free(func);
    } else {
      char *macro = template_to_macro(argv[i], false);
      fprintf(header, "%s\n", macro);
      free(macro);
    }
  }
}
