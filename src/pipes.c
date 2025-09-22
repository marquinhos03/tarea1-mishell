#include "include/shell.h"
#include "include/defines.h"

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

void ejecutar_pipeline(int n_comandos, int *pipes, char ***comandos) {
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

            /* Caso 1: Primer comando*/
            if (i == 0) {
                dup2(pipes[1], STDOUT_FILENO);  /* 1: pipe extremo lectura */
            }
            /* Caso 2: Ultimo comando */
            else if (i == n_comandos - 1) {
                dup2(pipes[(i - 1) * 2], STDIN_FILENO); /* (i - 1) * 2: pipe extremo escritura */
            }
            /* Caso 3: Comandos intermedios */
            else {
                dup2(pipes[(i - 1) * 2], STDIN_FILENO); /* pipe extremo escritura */
                dup2(pipes[(i * 2) + 1], STDOUT_FILENO);    /* pipe extremo lectura*/
            }

            /* Proceso hijo cierra pipes que no va a usar */
            for (int j = 0; j < n_fd_pipes; j++) {
                close(pipes[j]);
            }

            execvp(comandos[i][0], comandos[i]);
            
            /* Solo se ejecuta si hay error */
            perror("execvp error en ejecutar_pipeline");
            _exit(EXIT_FAILURE);
        }
    }

    /* Proceso padre cierra pipes para no bloquear los hijos */
    for (int j = 0; j < n_fd_pipes; j++) {
        close(pipes[j]);
    }

    /* Proceso padre espera a todos sus hijos */
    for (int i = 0; i < n_comandos; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

int main() {
    char *line;
    char **args;

    // cat txt1.txt | grep "hola" | wc
    printf("Comando: ");
    line = read_line();
    args = split_line(line);

    int n_comandos = contar_comandos_pipeline(args);
    char ***comandos = parse_pipeline(args, n_comandos);
    int *pipes = crear_pipes(n_comandos);

    for (int i = 0; comandos[i] != NULL; i++) {
        for (int j = 0; comandos[i][j] != NULL; j++) {
            printf("%s ", comandos[i][j]);
        }
        printf("\n");
    }

    ejecutar_pipeline(n_comandos, pipes, comandos);

    free(line);
    free(args);
    free(pipes);
    free(comandos);

    return 0;
}