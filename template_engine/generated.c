#include <stdio.h>
#include <math.h>

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

#define print_value(value) printf(print_generic(value), value)

#define index_html() \
	do{ \
		printf("%s", "<!doctype html>"); \
		printf("%s", "<html class=\"no-js\" lang=\"\">"); \
		printf("%s", "    <head>"); \
		printf("%s", "        <meta charset=\"utf-8\">"); \
		printf("%s", "        "); \
		print_value(testing); \
		printf("%s", ""); \
		printf("%s", "    </head>"); \
		printf("%s", "    <body>"); \
		printf("%s", "        <div>Hi Mom! My name is "); \
		printf("%s", name); \
		printf("%s", " "); \
		printf("%s", last_name); \
		printf("%s", "</div>"); \
		for(int i = 0; i < 10; i++) { \
		printf("%s", "            <div>"); \
		printf("%d", i); \
		printf("%s", "</div>"); \
		} \
		printf("%s", ""); \
		printf("%s", "        "); \
		print_value(sqrt(32)); \
		printf("%s", ""); \
		printf("%s", "    </body>"); \
		printf("%s", "</html>"); \
	} while (0)


int main() {
	char *name = "Jackson";
	char *last_name = "Mowry";
	float testing = 6.9;
	index_html();
}
