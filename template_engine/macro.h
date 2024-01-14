//DO NOT EDIT, FILE IS AUTOMATICALLY GENERATED
//EDIT TEMPLATE FILES INSTEAD
// #include "templates.h" in your .c file to use

#include <stdio.h>
#include <stdlib.h>


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

#define print_value(buf, value) ({ \
    int result = sprintf(buf, print_generic(value), value);\
    result; \
})

#define index_html() ({\
	char *buf = malloc(4096); \
	do{ \
		char *buf_ptr = buf; \
		buf_ptr += sprintf(buf_ptr, "%s\n", "<!doctype html>"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "<html class=\"no-js\" lang=\"\">"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "    <head>"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "        <meta charset=\"utf-8\">"); \
		buf_ptr += sprintf(buf_ptr, "%s", "        "); \
		buf_ptr += sprintf(buf_ptr, "%s", testing); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "    </head>"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "    <body>"); \
		buf_ptr += sprintf(buf_ptr, "%s", "        <div>Hi Mom! My name is "); \
		buf_ptr += sprintf(buf_ptr, "%s", name); \
		buf_ptr += sprintf(buf_ptr, "%s", " "); \
		buf_ptr += sprintf(buf_ptr, "%s", last_name); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "</div>"); \
		for(int i = 0; i < 10; i++) { \
		buf_ptr += sprintf(buf_ptr, "%s", "            <div>"); \
		buf_ptr += sprintf(buf_ptr, "%d", i); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "</div>"); \
		} \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "---"); \
		buf_ptr += sprintf(buf_ptr, "%s", "title: "); \
		buf_ptr += sprintf(buf_ptr, "%s", title); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s", "author: "); \
		buf_ptr += sprintf(buf_ptr, "%s", author); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "---"); \
		buf_ptr += sprintf(buf_ptr, "%s", "# "); \
		buf_ptr += sprintf(buf_ptr, "%s", title); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s", "[Author]: # ("); \
		buf_ptr += sprintf(buf_ptr, "%s", author); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ")"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "Content of article"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s", "Written by ["); \
		buf_ptr += sprintf(buf_ptr, "%s", author); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "]."); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%d", hi); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s", "        "); \
		buf_ptr += sprintf(buf_ptr, "%f", sqrt(32)); \
		buf_ptr += sprintf(buf_ptr, "%s\n", ""); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "    </body>"); \
		buf_ptr += sprintf(buf_ptr, "%s\n", "</html>"); \
	} while (0);\
	buf;\
})

