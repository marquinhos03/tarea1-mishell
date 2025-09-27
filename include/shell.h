#ifndef SHELL_H
#define SHELL_H

#define _POSIX_C_SOURCE 200809L  // Para características POSIX modernas

/* --- Librerias --- */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

/* --- DEFINES --- */
#define TOKEN_DELIM " \t\r\n\a\""
#define CONTINUE -1
#define MIPROF_EJEC "ejec"
#define MIPROF_EJECSAVE "ejecsave"
#define MIPROF_EJECUTAR "ejecutar"
#define MIPROF_MAXTIEMPO "maxtiempo"

#define LGREEN "\x1B[38;2;17;245;120m"
#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define PROMPT BOLD LGREEN "mishell" RESET "$ "

// Definir constantes para portabilidad
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#ifndef SA_RESTART
#define SA_RESTART 0x10000000
#endif

/* --- Definiciones de tipos --- */
typedef enum e_redirection_type {
    REDIR_TRUNC,
    REDIR_APPEND,
    REDIR_NULL
} redirection_type;

typedef struct s_redirection_info {
    redirection_type type;
    char *file_name;
} redirection_info;

typedef struct s_miprof_info {
    int status;
    double tiempo_usuario;
    double tiempo_sistema;
    double tiempo_real;
    long maximum_resident_set;
} miprof_info;

/* --- Variables globales --- */
extern char *redirection_string[];

/* --- Prototipos de función --- */

// input.c
char *read_line();
char **split_line(char *line);

// pipes.c
int contar_comandos_pipeline(char **args);
char ***parse_pipeline(char **args, int n_comandos);
int *crear_pipes(int n_comandos);
void ejecutar_pipeline(int n_comandos, int *pipes_arr, char ***comandos);

// main.c
int simple_command(char **args);
int execute_args(char **args);
int command_exit(char **args);
int command_cd(char **args);

// miprof.c
int execute_miprof(char **args);
miprof_info miprof_ejec(char **args);
int miprof_ejecsave(char *file_name, char *command_name, miprof_info command_info);
miprof_info get_miprof_info(struct rusage usage, struct timespec start_time, struct timespec end_time);

// redirection.c
void redirect_stdout_to_file(redirection_type type, char *file_name);
char **buscar_token(char **args, char *token);
redirection_info get_redirection_info(char **args);

#endif