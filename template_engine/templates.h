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

