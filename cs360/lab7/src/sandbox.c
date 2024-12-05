// Jackson Mowry
// Tue Nov 12 19:23:50 2024
// sandbox shell lab with resource limits, and no piping :(

#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct node {
    pid_t pid;
    char* command;
    struct node* next;
} Node_t;

// A single list to rule them all
static Node_t* head = NULL;
static size_t num_jobs = 0;

// Adds a job node to the list
static void add_pid(pid_t new_pid, char* command) {
    if (!head) {
        head = calloc(1, sizeof(Node_t));
        if (!head) {
            perror("calloc");
            exit(1);
        }
        head->pid = new_pid;
        head->command = command;
        num_jobs++;
        return;
    }

    Node_t* new_node = malloc(sizeof(Node_t));
    if (!new_node) {
        perror("malloc");
        exit(1);
    }
    new_node->pid = new_pid;
    new_node->command = command;
    new_node->next = head;
    head = new_node;
    num_jobs++;
}

// Removes a job node from the list
static void remove_pid(pid_t pid) {
    if (!head) {
        return;
    }

    if (head->pid == pid) {
        Node_t* tmp = head;
        head = head->next;

        free(tmp->command);
        free(tmp);
        num_jobs--;
        return;
    }

    Node_t* trailing = head;
    Node_t* cursor = head->next;
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

// Calls waitpid with WNOHANG on each pid, removing it if it has died
static void reap_jobs(void) {
    Node_t* cursor = head;

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

// I just wanted this to look like a function and Marz said no exit(0) call
#define shell_exit() return 0;

// Changes the current directory, it handles it's own errors
static bool shell_cd(const char* new_dir) {
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

// Prints all current jobs
static void shell_jobs(void) {
    Node_t* cursor = head;

    printf("%zu jobs.\n", num_jobs);
    while (cursor) {
        printf("%8d  - %s\n", cursor->pid, cursor->command);
        cursor = cursor->next;
    }
}

static const char* builtins[] = {"exit", "cd", "jobs"};
static const size_t num_builtins = 3;

// Does a simple string comparison on the token
static int is_builtin(const char* token) {
    for (size_t i = 0; i < num_builtins && token; i++) {
        if (!strcmp(builtins[i], token)) {
            return i;
        }
    }

    return -1;
}

// Specific function to replace `~` in a token
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

// Replaces all env vars in a token
static char* replace_env(const char* token) {
    if (!token) {
        return NULL;
    }

    // Starts with $ and a-zA-Z
    // Allowed chars a-z, A-Z, 0-9, _
    char scratch[1024] = {0};
    size_t scratch_idx = 0;

    for (size_t i = 0; i < strlen(token); i++) {
        if (token[i] == '$' && ((token[i + 1] >= 'a' && token[i + 1] <= 'z') ||
                                (token[i + 1] >= 'A' && token[i + 1] <= 'Z'))) {
            // parse env
            char env[1024] = {token[i + 1]};
            size_t env_idx = 1;
            while (token[i + 1 + env_idx] == '_' ||
                   (token[i + 1 + env_idx] >= 'a' &&
                    token[i + 1 + env_idx] <= 'z') ||
                   (token[i + 1 + env_idx] >= 'A' &&
                    token[i + 1 + env_idx] <= 'Z') ||
                   (token[i + 1 + env_idx] >= '0' &&
                    token[i + 1 + env_idx] <= '9')) {
                env[env_idx] = token[i + 1 + env_idx];
                env_idx++;
            }

            char* env_var = getenv(env);
            if (env_var) {
                char* ret = stpcpy(scratch + scratch_idx, env_var);
                scratch_idx = ret - scratch;
            }

            i += env_idx;
        } else {
            scratch[scratch_idx++] = token[i];
        }
    }

    char* ret_buf = malloc(strlen(scratch) + 1);
    if (!ret_buf) {
        perror("malloc");
        return NULL;
    }

    strcpy(ret_buf, scratch);

    return ret_buf;
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

    // Parse all provided options
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

    // Main loop!
    while (true) {
        // Check for completed jobs at the start of each iteration
        reap_jobs();

        // Build prompt
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

        // Ensure it ends with a null byte
        line[strcspn(line, "\n")] = '\0';

        // They just hit enter
        if (strlen(line) == 0) {
            continue;
        }

        // Command string for possible async job
        char* command = malloc(strlen(line) + 1);
        if (!command) {
            perror("malloc");
            return 1;
        }
        strcpy(command, line);
        command[strlen(command) - 1] = '\0';

        // Parse all tokens on whitespace
        char** tokens = calloc(10, sizeof(char*));
        if (!tokens) {
            perror("calloc");
            exit(1);
        }
        size_t max_tokens = 10;
        size_t num_tokens = 0;

        // Check each token for env vars
        tokens[num_tokens++] = strtok(line, " ");
        tokens[num_tokens - 1] = replace_env(tokens[num_tokens - 1]);

        while ((tokens[num_tokens++] = strtok(NULL, " "))) {
            tokens[num_tokens - 1] = replace_env(tokens[num_tokens - 1]);
            if (num_tokens == max_tokens) {
                max_tokens *= 2;
                tokens = realloc(tokens, sizeof(char*) * max_tokens);
            }
        }

        num_tokens--;

        // Handle builtin functions
        switch (is_builtin(tokens[0])) {
        case 0:
            for (size_t i = 0; i < num_tokens; i++) {
                free(tokens[i]);
            }
            free(tokens);
            free(command);
            shell_exit() break;
        case 1:
            shell_cd(tokens[1]);
            free(command);
            goto cleanup;
        case 2:
            // Check for completed jobs just before calling `shell_jobs()`
            reap_jobs();
            shell_jobs();
            free(command);
            goto cleanup;
        default:
            break;
        }

        // Check if the task needs to be performed in the background
        bool background = false;
        if (tokens[num_tokens - 1] && tokens[num_tokens - 1][0] == '&') {
            background = true;
            free(tokens[num_tokens - 1]);
            tokens[num_tokens - 1] = NULL;

            num_tokens--;
        }

        // Somehow we were empty and made it all the way down here
        if (num_tokens <= 0) {
            free(command);
            goto cleanup;
        }

        // Forking!
        int pid;
        if ((pid = fork()) == 0) {
            // child
            // Find redirection symbols
            // Final pass to create the command vector
            for (size_t i = 0; i < num_tokens; i++) {
                if (!tokens[i]) {
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

            // Set resource limits
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

            // Only execute if we have tokens
            if (tokens[0] && num_tokens > 0) {
                execvp(tokens[0], tokens);
                // If we make it here that means execvp failed to exec
                perror(tokens[0]);
                free(tokens);
                free(command);
                // Return 1 from child to "kill" it
                return 1;
            }
        } else if (pid == -1) {
            // error, we don't ever want to be here, this means forking/clone
            // failed
            perror("fork");
            return 1;
        } else {
            // parent, must wait for child or continue if async
            if (!background) {
                waitpid(pid, NULL, 0);
                free(command);
            } else {
                add_pid(pid, command);
            }
        }

    cleanup:
        // Standard cleanup section for most pathways
        for (size_t i = 0; i < num_tokens; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
}
