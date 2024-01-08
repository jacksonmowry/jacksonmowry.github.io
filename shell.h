#ifndef SHELL_H_
#define SHELL_H_

int sh_cd(char ***args);
int sh_help(char ***args);
int sh_exit(char ***args);
int sh_time(char ***args);

int sh_num_builtins();

int sh_run(char***, int);

char *read_line();
char **split_pipes(char*);
char ***split_args(char**, int*);

void execute_command(char**, int, int);
int execute_pipeline(char***, int);

#endif // SHELL_H_
