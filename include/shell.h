#ifndef SHELL_H
#define SHELL_H

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
#include <errno.h>

/* --- DEFINES --- */
#define TOKEN_DELIM " \t\r\n\a\""
#define CONTINUE -1
#define NOT_BUILTIN 100
#define MIPROF "miprof"
#define MIPROF_EJEC "ejec"
#define MIPROF_EJECSAVE "ejecsave"
#define MIPROF_EJECUTAR "ejecutar"
#define MIPROF_MAXTIEMPO "maxtiempo"
#define STATUS_TIMEOUT -2
#define STATUS_EXEC_FAIL -1

#define LGREEN "\x1B[38;2;17;245;120m"
#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define PROMPT BOLD LGREEN "mishell" RESET "$ "

/* --- Definiciones de tipos --- */
typedef enum e_redirection_type {
    REDIR_TRUNC,    // >
    REDIR_APPEND,   // >>
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
extern char *miprof_messages[];
extern char **environ;
extern char *integrated_command_list[];
extern int (*integrated_func_list[])(char **);

/* --- Prototipos de función --- */

// shell_utils.c
void shell_interactive();
int execute_args(char **args);
int simple_command(char **args);

// builtin.c
int command_help(char **args);
int command_exit(char **args);
int command_cd(char **args);
int command_env(char **args);
int execute_builtin(char **args);

// input.c
char *read_line();
char **split_line(char *line);

// pipes.c
int contar_comandos_pipeline(char **args);
char ***parse_pipeline(char **args, int n_comandos);
int *crear_pipes(int n_comandos);
void ejecutar_pipeline(int n_comandos, int *pipes_arr, char ***comandos);

// miprof.c
int count_args(char **args);
int execute_miprof(char **args);
miprof_info miprof_ejec(char **args);
int miprof_ejecsave(char *file_name, char *command_name, miprof_info command_info);
miprof_info get_miprof_info(struct rusage usage, struct timespec start_time, struct timespec end_time);
miprof_info miprof_ejecutar_maxtiempo(char **args, int maxtiempo);

// redirection.c
void redirect_stdout_to_file(redirection_type type, char *file_name);
char **buscar_token(char **args, char *token);
redirection_info get_redirection_info(char **args);

// signals.c
void sigalrm_handler(int sig);
void sigint_handler(int sig);
void parent_signals();
void reset_child_signals();

#endif