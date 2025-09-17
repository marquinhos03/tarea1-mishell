#ifndef SHELL_H
#define SHELL_H

/*Librerias*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>

#define TOKEN_DELIM " \t\r\n\a\""

/*Prototipos de función*/
char *read_line();
char **parse_line(char *line);
int new_process(char **args);
int execute_args(char **args);

int own_exit(char **);

#endif