#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Thing {
    uint16_t target;
    uint16_t ops[16];
    size_t num_ops;
    uint16_t joltages[16];
    size_t num_joltages;
} Thing;

int main() {
    Thing things[1024] = {0};
    size_t num_things = 0;

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin)) {
        char *ptr = strtok(buf, " ");

        do {
            if (ptr[0] == '[') {
                printf("ptr is \"%s\", in '[' loop\n", ptr);
                ptr++;
                int where = 15;
                while (ptr[0] && ptr[0] != ']') {
                    things[num_things].target |= (ptr[0] == '#') << (where--);
                    ptr++;
                };
            } else if (ptr[0] == '(') {
                ptr++;
                while (ptr[0] && ptr[0] != ')') {
                    if (ptr[0] == ',') {
                        ptr++;
                        continue;
                    }

                    // At a number
                    things[num_things].ops[things[num_things].num_ops] |=
                        1 << (15 - (ptr[0] - '0'));
                    ptr++;
                }

                // done
                things[num_things].num_ops++;
            } else if (ptr[0] == '{') {
                ptr++;

                while (1) {
                    int a;
                    sscanf(ptr, "%d", &a);
                    things[num_things]
                        .joltages[things[num_things].num_joltages++] = a;

                    while (ptr[0] != ',' && ptr[0] != '}') {
                        ptr++;
                    }

                    if (ptr[0] == ',') {
                        ptr++;
                    }

                    if (ptr[0] == '}') {
                        break;
                    }
                }

                ptr++;
                num_things++;
                break;
            }
        } while ((ptr = strtok(NULL, " ")));
    }

    printf("We have %zu things\n", num_things);
    for (size_t i = 0; i < num_things; i++) {
        printf("%8hu ", things[i].target);
        printf("[");
        for (size_t j = 0; j < 16; j++) {
            printf("%c", (things[i].target >> (16 - j - 1) & 1) ? '#' : '.');
        }
        printf("]");
        for (size_t j = 0; j < things[i].num_ops; j++) {
            printf("%hu ", things[i].ops[j]);
        }
        printf(" {");
        for (size_t j = 0; j < things[i].num_joltages; j++) {
            printf("%hu ", things[i].joltages[j]);
        }
        printf(" }");
        printf("\n");
    }

    system("rm answer.txt");
    for (size_t i = 0; i < num_things; i++) {
        FILE *fp = fopen("math.txt", "w");

        fprintf(fp, "pkg load xlinprog;\n");

        fprintf(fp, "A=[");
        for (size_t j = 0; j < things[i].num_ops; j++) {
            for (size_t k = 0; k < things[i].num_joltages; k++) {
                fprintf(fp, "%hu", (things[i].ops[j] >> (15 - k)) & 1);
                if (k != things[i].num_joltages - 1) {
                    fprintf(fp, ",");
                } else {
                    fprintf(fp, ";");
                }
            }
        }
        fprintf(fp, "]';\n");

        fprintf(fp, "b=[");
        for (size_t j = 0; j < things[i].num_joltages; j++) {
            fprintf(fp, "%hu;", things[i].joltages[j]);
        }
        fprintf(fp, "];\n");

        fprintf(fp, "cols = size(A, 2);\n");
        fprintf(fp, "f = ones(1,cols);\n");
        fprintf(fp, "lb = 0 * ones(1,cols);\n");
        fprintf(fp, "intcon=1:cols;\n");

        fprintf(fp, "[x,fval,exitflag,output]=intlinprog(f,intcon,[],[],A,b,lb)"
                    ";\ndisp(x)\n");

        fclose(fp);

        system("octave <math.txt 2>/dev/null | awk '{sum+=$1}END{print sum}' "
               ">>answer.txt");
    }

    char b[256];
    FILE *fp = fopen("answer.txt", "r");
    long long total = 0;
    while (fgets(b, sizeof(b), fp)) {
        long long tmp;
        sscanf(b, "%lld", &tmp);
        total += tmp;
    }
    fclose(fp);

    printf("Answer: %lld\n", total);
}
