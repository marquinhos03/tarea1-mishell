#include "include/shell.h"
#include "include/defines.h"

char *redirection_string[] = {
    ">",    // REDIR_TRUNC
    ">>",   // REDIR_APPEND
    NULL    // REDIR_NULL
};

void fd_out(t_redirection_type type, char *file_name) {
    int file_fd;
    int flags;

    if (type == REDIR_TRUNC) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (type == REDIR_APPEND) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }

    file_fd = open(file_name, flags, 0644);

    if (file_fd == -1) {
        perror("error en parsing: open");
        exit(EXIT_FAILURE);
    }
    if (dup2(file_fd, STDOUT_FILENO) == -1) {
        perror("error en parsing: dup2");
        exit(EXIT_FAILURE);
    }

    close(file_fd);
}

char **buscar_token(char **args, char *token) {
    int i = 0;
    while (args[i]) {
        if (strcmp(args[i], token) == 0) {
            return &args[i];
        }
        i++;
    }
    return NULL;
}

t_redirection_info get_redirection_info(char **args) {
    t_redirection_info current_info = {REDIR_NULL, NULL};
    char **operador;

    int i = 0;
    while (redirection_string[i] != NULL) {
        /* Buscar operador y retornar la dirección del operador en args */
        operador = buscar_token(args, redirection_string[i]);

        if (operador) {
            current_info.type = (t_redirection_type)i;
            current_info.file_name = *(operador + 1);

            /* Reemplazar operador por NULL en el array args */
            *operador = NULL;

            return current_info;
        }

        i++;
    }

    return current_info;
}

/*
int main() {
    int status;
    char *line;
    char **args;
    
    line = read_line();
    args = split_line(line);

    t_redirection_info redir_info = get_redirection_info(args);
    
    pid_t pid;
    pid = fork();

    if (pid == 0) {
        printf("proceso hijo");
        fd_out(args, redir_info.type, redir_info.file_name);
    }
    else {
        printf("proceso padre");
        waitpid(pid, &status, 0);
    }

    free(line);
    free(args);

    return 0;
}
*/
