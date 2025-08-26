#include "tokenizier.h"
#include "tokens.h"

#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define log_fatal(fmt, ...)                                                    \
    do {                                                                       \
        fprintf(stderr, "scripting: " fmt __VA_OPT__(, ) __VA_ARGS__);         \
        exit(1);                                                               \
    } while (false)

int main(int argc, char* argv[]) {
    char* filename;

    int c;
    int digit_optind = 0;

    while (1) {
        auto this_option_optind = optind ? optind : 1;
        auto option_index = 0;
        static struct option long_options[] = {{0, 0, 0, 0}};

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            printf("option %s", long_options[option_index].name);
            if (optarg) {
                printf(" with arg%s", optarg);
            }
            printf("\n");
            break;
        case '?':
            break;
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        const auto remaining_arguments = argc - optind - 1;
        if (remaining_arguments > 1) {
            log_fatal("Too many file arguments, expected 1 source file\n", );
        }

        filename = argv[optind];
    }

    auto input_file = fopen(filename, "r");
    if (!input_file) {
        log_fatal("Unable to open file for reading %s\n", filename);
    }

    Tokenizer t;

    return 0;
}
