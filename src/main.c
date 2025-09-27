#include "../include/shell.h"
#include <signal.h>

// Variable global para controlar la shell
static volatile sig_atomic_t shell_running = 1;

// Handler para SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    printf("\n"); // Nueva línea después de Ctrl+C
    printf(PROMPT);
    fflush(stdout);
}

int command_exit(char **args) {
    shell_running = 0;
    if (args[1]) {
        return atoi(args[1]);
    }
    else {
        return 0;
    }
}

int command_cd(char **args) {
    if (args[1] == NULL) {
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
    struct sigaction sa;

    // Configurar handler para SIGINT
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    shell_running = 1;
    
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

    } while (shell_running);
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

    size_t num_commands = sizeof(integrated_command_list) / sizeof(char *);

    if (args[0] == NULL) {
        return CONTINUE;
    }

    for(size_t i = 0; i < num_commands; i++) {
        if (strcmp(args[0], integrated_command_list[i]) == 0) {
            return (*integrated_func_list[i])(args);
        }
    }

    if (strcmp(args[0], "miprof") == 0) {
        if (args[1] == NULL) {
            printf("Uso: miprof [ejec | ejecsave archivo] comando args\n");
            return CONTINUE;
        }
        return execute_miprof(args);
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
    redirection_info info;
    int status;
    struct sigaction sa_ignore, sa_default;
    
    pid = fork();
    if (pid == 0) {
        // Proceso hijo - restaurar handler por defecto para SIGINT
        sa_default.sa_handler = SIG_DFL;
        sigemptyset(&sa_default.sa_mask);
        sa_default.sa_flags = 0;
        sigaction(SIGINT, &sa_default, NULL);

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
        // Proceso padre - ignorar SIGINT mientras espera al hijo
        sa_ignore.sa_handler = SIG_IGN;
        sigemptyset(&sa_ignore.sa_mask);
        sa_ignore.sa_flags = 0;
        sigaction(SIGINT, &sa_ignore, NULL);
        
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
        // Restaurar el handler original después de esperar
        sa_default.sa_handler = sigint_handler;
        sigemptyset(&sa_default.sa_mask);
        sa_default.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa_default, NULL);
    }

    return CONTINUE;
}

int main() {
    shell_interactive();
    return 0;
}