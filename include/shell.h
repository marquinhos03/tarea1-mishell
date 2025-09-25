#ifndef SHELL_H
#define SHELL_H

/* --- Librerias --- */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "defines.h"

/* --- DEFINES --- */

#define TOKEN_DELIM " \t\r\n\a\""
#define CONTINUE -1
#define MIPROF_EJEC "ejec"
#define MIPROF_EJECSAVE "ejecsave"


/* --- Prototipos de función --- */

// input.c
char *read_line();
char **split_line(char *line);

// pipes.c
int contar_comandos_pipeline(char **args);
char ***parse_pipeline(char **args, int n_comandos);
int *crear_pipes(int n_comandos);
void ejecutar_pipeline(int n_comandos, int *pipes_arr, char ***comandos);

int simple_command(char **args);
int execute_args(char **args);

// miprof.c
typedef struct s_miprof_info {
    double tiempo_usuario;
    double tiempo_sistema;
    double tiempo_real;
    long maximum_resident_set;
} miprof_info;

int execute_miprof(char **args);
int miprof_ejec(char **args, char *file_name);
int miprof_ejecsave(char *file_name, char *command_name, miprof_info command_info);
miprof_info get_miprof_info(struct rusage usage, struct timespec start_time, struct timespec end_time);

int command_exit(char **);

void fd_out(t_redirection_type type, char *file_name);
char **buscar_token(char **args, char *token);
t_redirection_info get_redirection_info (char **args);

#endif