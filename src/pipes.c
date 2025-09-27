#include "include/shell.h"

int contar_comandos_pipeline(char **args) {
    int n_comandos = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            n_comandos++;
        }
    }
    n_comandos++;   // +1 para el primer comando
    return n_comandos;
}

char ***parse_pipeline(char **args, int n_comandos) {
    char ***comandos = malloc((n_comandos + 1) * sizeof(char **));
    if (!comandos) {
        perror("error al asignar memoria en split_line: comandos\n");
        exit(EXIT_FAILURE);
    }

    int c_index = 0;
    int token_index = 0;

    while (c_index < n_comandos) {
        comandos[c_index] = &args[token_index];

        /* Encuentra el final del comando si "|" o el fin de los argumentos si NULL */
        while (args[token_index] != NULL && strcmp(args[token_index], "|") != 0) {
            token_index++;
        }

        /* Reemplazar "|" por NULL para indicar el final del argumento del comando encontrado */
        if (args[token_index] != NULL) {
            args[token_index] = NULL;
            token_index++;
        }
        c_index++;
    }
    /* NULL para indicar el final de los argumentos del array de comandos */
    comandos[c_index] = NULL;

    return comandos;
}

int *crear_pipes(int n_comandos) {
    int n_pipes = n_comandos - 1;

    int *pipes_arr = malloc((n_pipes * 2) * sizeof(int));
    if (!pipes_arr) {
        perror("malloc error en crear_pipes: pipes_arr\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_pipes; i++) {
        if (pipe(pipes_arr + 2 * i) == -1) {
            perror("error en crear_pipes: pipe\n");
            free(pipes_arr);
            exit(EXIT_FAILURE);    
        }
    }

    return pipes_arr;
}

void ejecutar_pipeline(int n_comandos, int *pipes_arr, char ***comandos) {
    pid_t pids[n_comandos]; /* n_comandos <=> n procesos hijos */
    int n_fd_pipes = (n_comandos - 1) * 2;  /* n pipes * 2 <=> n descriptores de archivo */

    for (int i = 0; i < n_comandos; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("error en ejecutar_pipeline: fork");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {
            /* Proceso hijo */
            reset_child_signals();

            /* Caso 1: Primer comando*/
            if (i == 0) {
                dup2(pipes_arr[1], STDOUT_FILENO);
            }
            /* Caso 2: Ultimo comando */
            else if (i == n_comandos - 1) {                
                dup2(pipes_arr[(i - 1) * 2], STDIN_FILENO);

                /* Caso: Hay operador de redirección */
                redirection_info info;
                info = get_redirection_info(comandos[i]);

                if (info.type != REDIR_NULL) {
                    redirect_stdout_to_file(info.type, info.file_name);
                }
            }
            /* Caso 3: Comandos intermedios */
            else {
                dup2(pipes_arr[(i - 1) * 2], STDIN_FILENO);
                dup2(pipes_arr[(i * 2) + 1], STDOUT_FILENO);
            }

            /* Proceso hijo cierra pipes que no va a usar */
            for (int j = 0; j < n_fd_pipes; j++) {
                close(pipes_arr[j]);
            }

            execvp(comandos[i][0], comandos[i]);
            
            /* Solo se ejecuta si execvp da error */
            perror("execvp error en ejecutar_pipeline");
            _exit(EXIT_FAILURE);
        }
    }

    /* Proceso padre cierra pipes para no bloquear los hijos */
    for (int j = 0; j < n_fd_pipes; j++) {
        close(pipes_arr[j]);
    }

    /* Proceso padre espera a todos sus hijos */
    for (int i = 0; i < n_comandos; i++) {
        waitpid(pids[i], NULL, 0);
    }
}