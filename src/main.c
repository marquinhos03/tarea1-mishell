#include "include/shell.h"
#include "include/defines.h"

int command_exit(char **args) {
    if (args[1]) {
        return atoi(args[1]);
    }
    else {
        return 0;
    }
}

void shell_interactive() {
    char *line;
    char **args;
    t_redirection_info redir_info;
    int shell_status = -1;

    do {
        printf(BOLD BLUE "mishell" RESET "$ ");
        line = read_line();
        args = split_line(line);
        redir_info = get_redirection_info(args);

        shell_status = execute_args(args, redir_info);

        free(line);
        free(args);

        if (shell_status >= 0) {
            printf("Saliendo... valor shell_status: %d\n", shell_status);
            exit(shell_status);
        }

    } while (shell_status == -1);
}

int execute_args(char **args, t_redirection_info redir_info) {
    char *integrated_command_list[] = {
        "exit"
    };
    int (*integrated_func_list[])(char **) = {
        &command_exit
    };

    if (args[0] == NULL) {
        return -1;
    }

    for(int i = 0; i < sizeof(integrated_command_list) / sizeof(char *); i++) {
        if (strcmp(args[0], integrated_command_list[i]) == 0) {
            return (*integrated_func_list[i])(args);
        }
    }

    return new_process(args, redir_info);
}

int new_process(char **args, t_redirection_info redir_info) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */

    
        if (redir_info.type != REDIR_NULL) {
            fd_out(args, redir_info.type, redir_info.file_name);
        }

        if (execvp(args[0], args) < 1) {
            perror("error en new_process: proceso hijo");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        /* Error en fork() */
        perror("error en new_process: fork");
    }
    else {
        /* Proceso padre */
        do {
            waitpid(pid, &status, WUNTRACED);

            if (WIFEXITED(status)) {
                printf("Hijo %d termina con estado = %d\n", pid, WEXITSTATUS(status));
            }
            else {
                printf("Hijo %d termina mal\n", pid);
            }

            /**
             * WIFEXITED() y WIFSIGNALED() distintos de cero (true) si el proceso hijo termino 
             * de forma normal o fue terminado por una señal
            */
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return -1;
}

int main() {

    shell_interactive();

    return 0;
}


/**
 * Casos encontrados para solucionar:
 * 
 * Ejecutar el comando en mishell y terminal de ubuntu para visualizarlo mejor:
 * cat include/shell.h
 */