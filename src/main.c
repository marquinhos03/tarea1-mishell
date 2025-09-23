#include "include/shell.h"
#include "include/defines.h"

#define CONTINUE -1

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
    int shell_status = CONTINUE;

    do {
        printf(BOLD BLUE "mishell" RESET "$ ");
        line = read_line();
        args = split_line(line);
        /* fixear esto */
        // redir_info = get_redirection_info(args);
        /**/
        shell_status = execute_args(args);

        free(line);
        free(args);

        if (shell_status >= 0) {
            printf("Saliendo... valor shell_status: %d\n", shell_status);
            exit(shell_status);
        }

    } while (shell_status == CONTINUE);
}

int execute_args(char **args) {
    char *integrated_command_list[] = {
        "exit"
    };
    int (*integrated_func_list[])(char **) = {
        &command_exit
    };

    if (args[0] == NULL) {
        return CONTINUE;
    }

    for(int i = 0; i < sizeof(integrated_command_list) / sizeof(char *); i++) {
        if (strcmp(args[0], integrated_command_list[i]) == 0) {
            return (*integrated_func_list[i])(args);
        }
    }

    int n_comandos = contar_comandos_pipeline(args);
    if (n_comandos > 1) {
        char ***comandos = parse_pipeline(args, n_comandos);
        int *pipes_arr = crear_pipes(n_comandos);
        ejecutar_pipeline(n_comandos, pipes_arr, comandos);
        
        free(pipes_arr);
        free(comandos);

        return CONTINUE;
    }

    return simple_command(args);
}

int simple_command(char **args) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */

        /* Caso: Hay operador de redirección */
        t_redirection_info info;
        info = get_redirection_info(args);

        if (info.type != REDIR_NULL) {
            fd_out(info.type, info.file_name);
        }

        execvp(args[0], args);

        /* Solo se ejecuta si hay error */
        perror("execvp error en simple_command");
        _exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        /* Error en fork() */
        perror("error en simple_command: fork");
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

    return CONTINUE;
}

int main() {

    shell_interactive();

    return 0;
}