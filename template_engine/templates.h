#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#define print_generic(value) _Generic((value), \
   char: "%c", \
   char*: "%s", \
   const char*: "%s", \
   float: "%.2f", \
   double: "%.2f", \
   signed char: "%d", \
   unsigned char: "%u", \
   signed short: "%d", \
   unsigned short: "%u", \
   signed int: "%d", \
   unsigned int: "%u", \
   signed long: "%ld", \
   unsigned long: "%lu", \
   signed long long: "%lld", \
   unsigned long long: "%llu" \
)

#define print_value_generic(buf, value) ({ \
    int length = snprintf(NULL, 0, print_generic(value), value); \
    buffer_resize(&buf, length); \
    buf.len += sprintf(&buf.buffer[buf.len], print_generic(value), value); \
    buf.buffer[buf.len] = '\0'; \
})
#define print_value(buf, fmt, value) ({ \
    int length = snprintf(NULL, 0, fmt, value); \
    buffer_resize(&buf, length); \
    buf.len += sprintf(&buf.buffer[buf.len], fmt, value); \
    buf.buffer[buf.len] = '\0'; \
})

// GENERATED FUNCTION from file index_html
// Memory returned by %s is heap allocated, and must be freed
#define index_html()({\
	buffer output_buffer = (buffer){.buffer = malloc(4096), .len = 0, .cap = 4096};\
	do {\
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "<!doctype html>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "<html class=\"no-js\" lang=\"\">"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "    <head>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "        <meta charset=\"utf-8\">"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s", "        <link>"); \
		print_value_generic(output_buffer, testing); \
		buffer_write(&output_buffer, "%s\n", "</link>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "        <style>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "         body {"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "             background-color: #1a1a1a; /* Dark background color */"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "             color: #ffffff; /* Light text color */"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "         }"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "        </style>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "    </head>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "    <body>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s", "        <div>Hi Mom! My name is "); \
		print_value_generic(output_buffer, name); \
		buffer_write(&output_buffer, "%s", " "); \
		print_value_generic(output_buffer, last_name); \
		buffer_write(&output_buffer, "%s\n", "</div>"); \
		for(int i = 0; i < 10; i++) { \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s", "            <div>"); \
		print_value_generic(output_buffer, i); \
		buffer_write(&output_buffer, "%s\n", "</div>"); \
		} \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "        <pre>"); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s\n", "---"); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s", "title: "); \
		print_value(output_buffer, "%s", title); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s", "author: "); \
		print_value(output_buffer, "%s", author); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s\n", "---"); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s", "# "); \
		print_value(output_buffer, "%s", title); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s", "[Author]: # ("); \
		print_value(output_buffer, "%s", author); \
		buffer_write(&output_buffer, "%s\n", ")"); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s\n", "Content of article"); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s", "Written by ["); \
		print_value(output_buffer, "%s", author); \
		buffer_write(&output_buffer, "%s\n", "]."); \
		buffer_write(&output_buffer, "%s", "			"); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", "			"); \
		print_value(output_buffer, "%d", hi); \
		buffer_write(&output_buffer, "%s\n", ""); \
		char j = 'c'; \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "        </pre>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s", "        "); \
		print_value_generic(output_buffer, sqrt(32)); \
		buffer_write(&output_buffer, "%s\n", ""); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "    </body>"); \
		buffer_write(&output_buffer, "%s", ""); \
		buffer_write(&output_buffer, "%s\n", "</html>"); \
	} while (0); \
	output_buffer.buffer;\
})
