#ifndef SHELL_H
#define SHELL_H

/* Librerias */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include "defines.h"

#define TOKEN_DELIM " \t\r\n\a\""

/*Prototipos de función*/

// input.c
char *read_line();
char **split_line(char *line);

// pipes.c
char ***parse_pipeline(char **args, int n_comandos);
int contar_comandos_pipeline(char **args);

int simple_command(char **args, t_redirection_info redir_info);
int execute_args(char **args, t_redirection_info redir_info);


int command_exit(char **);

void fd_out(t_redirection_type type, char *file_name);
char **buscar_token(char **args, char *token);
t_redirection_info get_redirection_info (char **args);

#endif