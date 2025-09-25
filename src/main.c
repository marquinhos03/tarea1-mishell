#include "include/shell.h"

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
    int shell_status = CONTINUE;

    do {
        printf(PROMPT);
        line = read_line();
        args = split_line(line);
        shell_status = execute_args(args);

        free(line);
        free(args);

        if (shell_status >= 0) {
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

    /* Si se pulso Enter */
    if (args[0] == NULL) {
        return CONTINUE;
    }

    for(int i = 0; i < sizeof(integrated_command_list) / sizeof(char *); i++) {
        if (strcmp(args[0], integrated_command_list[i]) == 0) {
            return (*integrated_func_list[i])(args);
        }
    }

    /* Si se ejecuto el comando miprof */
    if (strcmp(args[0], "miprof") == 0) {
        int current_status;
        
        return execute_miprof(args);
    }

    int n_comandos = contar_comandos_pipeline(args);
    /* Si se ingreso una pipeline */
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
    redirection_info info;
    int status;
    
    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */

        /* Caso: Hay operador de redirección */
        info = get_redirection_info(args);
        if (info.type != REDIR_NULL) {
            redirect_stdout_to_file(info.type, info.file_name);
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