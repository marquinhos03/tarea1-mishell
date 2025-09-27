#include "include/shell.h"

// Handler para SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    printf("\n"); // Nueva línea después de Ctrl+C
    printf(PROMPT);
    fflush(stdout);
}

int command_exit(char **args) {
    if (args[1]) {
        return atoi(args[1]);
    }
    else {
        return 0;
    }
}

int command_cd(char **args) {
    if (!args[1]) {
        fprintf(stderr, "cd: se esperaba un argumento\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
    return CONTINUE;
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
        "exit",
        "cd"
    };
    
    int (*integrated_func_list[])(char **) = {
        &command_exit,
        &command_cd
    };

    if (args[0] == NULL) {
        return CONTINUE;
    }

    size_t num_commands = sizeof(integrated_command_list) / sizeof(char *);
    for(size_t i = 0; i < num_commands; i++) {
        if (strcmp(args[0], integrated_command_list[i]) == 0) {
            return (*integrated_func_list[i])(args);
        }
    }

    /* Si se ejecuto el comando miprof */
    if (strcmp(args[0], MIPROF) == 0)  {
        // Si se ingreso solo "miprof"
        if (!args[1]) {
            printf("Uso: miprof [ejec | ejecsave archivo] <comando> args\n");
            return CONTINUE;
        }

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

        info = get_redirection_info(args);
        if (info.type != REDIR_NULL) {
            redirect_stdout_to_file(info.type, info.file_name);
        }

        execvp(args[0], args);
        perror("execvp error en simple_command");
        _exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
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