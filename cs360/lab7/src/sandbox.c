#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct node {
    pid_t pid;
    char* command;
    struct node* next;
} node;

static node* head = NULL;
static size_t num_jobs = 0;

static void add_pid(pid_t new_pid, char* command) {
    if (!head) {
        head = calloc(1, sizeof(node));
        if (!head) {
            perror("calloc");
            exit(1);
        }
        head->pid = new_pid;
        head->command = command;
        num_jobs++;
        return;
    }

    node* new_node = malloc(sizeof(node));
    if (!new_node) {
        perror("malloc");
        exit(1);
    }
    new_node->pid = new_pid;
    head->command = command;
    new_node->next = head;
    head = new_node;
    num_jobs++;
}

static void remove_pid(pid_t pid) {
    if (!head) {
        return;
    }

    if (head->pid == pid) {
        node* tmp = head;
        head = head->next;

        free(tmp->command);
        free(tmp);
        num_jobs--;
        return;
    }

    node* trailing = head;
    node* cursor = head->next;
    while (cursor) {
        if (cursor->pid == pid) {
            trailing->next = cursor->next;
            free(cursor->command);
            free(cursor);
            num_jobs--;
            return;
        }

        trailing = cursor;
        cursor = cursor->next;
    }
}

static void reap_jobs(void) {
    node* cursor = head;

    while (cursor) {
        if (!waitpid(cursor->pid, NULL, WNOHANG)) {
            cursor = cursor->next;
            continue;
        }

        pid_t pid = cursor->pid;
        cursor = cursor->next;

        remove_pid(pid);
    }
}

#define shell_exit() return 0;

bool shell_cd(const char* new_dir) {
    if (!new_dir || (strlen(new_dir) == 1 && new_dir[0] == '~')) {
        new_dir = getenv("HOME");
    }

    int ret = chdir(new_dir);
    if (ret == -1) {
        perror(new_dir);
        return false;
    }

    return true;
}

void shell_jobs(void) {
    node* cursor = head;

    printf("%zu jobs.\n", num_jobs);
    while (cursor) {
        printf("%8d  - %s\n", cursor->pid, cursor->command);
        cursor = cursor->next;
    }
}

const char* builtins[] = {"exit", "cd", "jobs"};
const size_t num_builtins = 3;

static int is_builtin(const char* token) {
    for (size_t i = 0; i < num_builtins && token; i++) {
        if (!strcmp(builtins[i], token)) {
            return i;
        }
    }

    return -1;
}

static void replace_home(char* path) {
    const char* home = getenv("HOME");
    if (!home) {
        return;
    }

    char* home_pos = strstr(path, home);
    if (!home_pos) {
        return;
    }

    *home_pos = '~';
    memmove(home_pos + 1, home_pos + strlen(home),
            strlen(home_pos + strlen(home)));

    *(home_pos + strlen(home_pos + strlen(home)) + 1) = '\0';

    return;
}

int main(int argc, char* argv[]) {
    size_t max_processes = 256;
    size_t max_data_size = 1 << 30;
    size_t max_stack_size = 1 << 30;
    size_t max_file_descriptors = 256;
    size_t max_file_size = 1 << 30;
    size_t max_cpu_time = 1 << 30;

    opterr = 0;
    int c = 0;

    while ((c = getopt(argc, argv, "p:d:s:n:f:t:")) != -1) {
        switch (c) {
        case 'p':
            sscanf(optarg, "%lu", &max_processes);
            break;
        case 'd':
            sscanf(optarg, "%lu", &max_data_size);
            break;
        case 's':
            sscanf(optarg, "%lu", &max_stack_size);
            break;
        case 'n':
            sscanf(optarg, "%lu", &max_file_descriptors);
            break;
        case 'f':
            sscanf(optarg, "%lu", &max_file_size);
            break;
        case 't':
            sscanf(optarg, "%lu", &max_cpu_time);
            break;
        case '?':
            if (optopt == 'p' || optopt == 'd' || optopt == 's' ||
                optopt == 'n' || optopt == 'f' || optopt == 't') {
                fprintf(stderr, "Option -%c requires and argument.\n", optopt);
            } else if (isprint(optopt)) {
                fprintf(stderr, "Unknown switch -%c.\n", optopt);
            } else {
                fprintf(stderr, "Uknown option character \\x%x.\n", optopt);
            }
            break;
        default:
            fprintf(stderr, "Error while parsing arguments. %d\n", c);
            return 1;
        }
    }

    while (true) {
        char cwd[512] = {0};
        if (!getcwd(cwd, sizeof(cwd))) {
            perror("getcwd");
            exit(1);
        }

        if (feof(stdin)) {
            shell_exit();
        }

        replace_home(cwd);
        printf("%s@%s:%s> ", getenv("USER"), "sandbox", cwd);

        char line[1024] = {0};
        if (!fgets(line, sizeof(line), stdin)) {
            shell_exit();
        }

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        char* command = malloc(strlen(line) + 1);
        if (!command) {
            perror("malloc");
            return 1;
        }
        strcpy(command, line);
        command[strlen(command) - 1] = '\0';

        char** tokens = calloc(10, sizeof(char*));
        if (!tokens) {
            perror("calloc");
            exit(1);
        }
        size_t max_tokens = 10;
        size_t num_tokens = 0;

        tokens[num_tokens++] = strtok(line, " ");

        while ((tokens[num_tokens++] = strtok(NULL, " "))) {
            if (num_tokens == max_tokens) {
                max_tokens *= 2;
                tokens = realloc(tokens, sizeof(char*) * max_tokens);
            }
        }

        num_tokens--;

        switch (is_builtin(tokens[0])) {
        case 0:
            free(tokens);
            free(command);
            shell_exit() break;
        case 1:
            if (!shell_cd(tokens[1])) {
                perror(tokens[1]);
                free(tokens);
                free(command);
                return 1;
            }
            free(tokens);
            free(command);
            continue;
        case 2:
            reap_jobs();
            shell_jobs();
            free(tokens);
            free(command);
            continue;
        default:
            break;
        }

        bool background = false;
        if (tokens[num_tokens - 1] && tokens[num_tokens - 1][0] == '&') {
            background = true;
            tokens[num_tokens - 1] = NULL;

            num_tokens--;
        }

        if (num_tokens <= 0) {
            free(tokens);
            free(command);
            continue;
        }

        int pid;
        if ((pid = fork()) == 0) {
            // child
            // Find redirection symbols
            for (size_t i = 0; i < num_tokens; i++) {
                if (!tokens[i]) {
                    continue;
                }

                if (tokens[i] && tokens[i][0] == '$') {
                    tokens[i] = getenv(tokens[i] + 1);
                    continue;
                }

                if (!strncmp(">>", tokens[i], 2) && strlen(tokens[i]) > 2) {
                    // append redirection
                    int fd = open(tokens[i] + 2, O_WRONLY | O_APPEND | O_CREAT,
                                  0666);
                    if (fd == -1) {
                        perror(tokens[i] + 2);
                        return 1;
                    }
                    tokens[i] += 2;
                    dup2(fd, STDOUT_FILENO);
                } else if (!strncmp(">", tokens[i], 1) &&
                           strlen(tokens[i]) > 1) {
                    // overwrite redirection
                    int fd = open(tokens[i] + 1, O_WRONLY | O_CREAT, 0666);
                    if (fd == -1) {
                        perror(tokens[i] + 1);
                        return 1;
                    }
                    tokens[i] += 1;
                    dup2(fd, STDOUT_FILENO);
                } else if (!strncmp("<", tokens[i], 1) &&
                           strlen(tokens[i]) > 1) {
                    // input redirection
                    int fd = open(tokens[i] + 1, O_RDONLY);
                    if (fd == -1) {
                        perror(tokens[i] + 1);
                        return 1;
                    }
                    tokens[i] += 1;
                    dup2(fd, STDIN_FILENO);
                } else {
                    continue;
                }

                memmove(tokens + i, tokens + i + 1,
                        (num_tokens - i) * sizeof(char*));
                num_tokens--;
                tokens[num_tokens] = NULL;
                i--;
            }
            setrlimit(RLIMIT_NPROC,
                      &(struct rlimit){.rlim_cur = max_processes,
                                       .rlim_max = max_processes});
            setrlimit(RLIMIT_DATA, &(struct rlimit){.rlim_cur = max_data_size,
                                                    .rlim_max = max_data_size});
            setrlimit(RLIMIT_STACK,
                      &(struct rlimit){.rlim_cur = max_stack_size,
                                       .rlim_max = max_stack_size});
            setrlimit(RLIMIT_NOFILE,
                      &(struct rlimit){.rlim_cur = max_file_descriptors,
                                       .rlim_max = max_file_descriptors});
            setrlimit(RLIMIT_FSIZE,
                      &(struct rlimit){.rlim_cur = max_file_size,
                                       .rlim_max = max_file_size});
            setrlimit(RLIMIT_CPU, &(struct rlimit){.rlim_cur = max_cpu_time,
                                                   .rlim_max = max_cpu_time});

            if (tokens[0] && num_tokens > 0) {
                execvp(tokens[0], tokens);
                perror(tokens[0]);
                free(tokens);
                free(command);
                return 1;
            }
        } else if (pid == -1) {
            // error
            exit(1);
            perror("fork");
        } else {
            // parent
            if (!background) {
                waitpid(pid, NULL, 0);
                free(command);
            } else {
                add_pid(pid, command);
            }
        }

        free(tokens);
    }
}
