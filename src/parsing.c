#include "include/shell.h"
#include <fcntl.h>

typedef enum e_redirection_type {
    REDIR_TRUNC,    // >
    REDIR_APPEND,   // >>
    REDIR_NULL
} t_redirection_type;

char *redirection_string[] = {
    ">",
    ">>",
    NULL
};

typedef struct s_redirection_info {
    t_redirection_type type;
    char *file_name;
} t_redirection_info;

int fd_out(char **args, t_redirection_type type, char *file_name) {
    int file_fd;
    int flags;

    if (type == REDIR_TRUNC) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (type == REDIR_APPEND) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }

    file_fd = open(file_name, flags, 0644);

    if (file_fd == -1) {
        perror("parsing: open");
        exit(EXIT_FAILURE);
    }
    if (dup2(file_fd, STDOUT_FILENO) == -1) {
        perror("parsing: dup2");
        exit(EXIT_FAILURE);
    }

    close(file_fd);
    execvp(args[0], args);

    return -1;
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

t_redirection_info get_redirection_info (char **args) {
    t_redirection_info info = {REDIR_NULL, NULL};
    char **operador;

    int i = 0;
    while(redirection_string[i] != NULL) {
        operador = buscar_token(args, redirection_string[i]);

        if (operador) {
            info.type = (t_redirection_type)i;
            info.file_name = *(operador + 1);

            *operador = NULL;

            return info;
        }

        i++;
    }

    return info;
}

int main() {
    int status;
    char *line;
    char **args;
    
    line = read_line();
    args = split_line(line);

    t_redirection_info redir_info = get_redirection_info(args);

    /*
    t_redirection_type operador = operador_token1(args);

    t_redirection_type operador = operador_token(args);
    printf("%d\n", operador);

    char **operador_redir = buscar_token(args, ">");
    char **operador_redir1 = buscar_token(args, ">>");
    char *file_name;

    if (operador_redir) {
        file_name = *(operador_redir + 1);
        printf("%s\n", *operador_redir);
        *operador_redir = NULL;
    } else if (operador_redir1) {
        file_name = *(operador_redir1 + 1);
        printf("%s\n", *operador_redir1);
        *operador_redir1 = NULL;
    }
    */
    
    pid_t pid;
    pid = fork();

    if (pid == 0) {
        printf("proceso hijo");
        fd_out(args, redir_info.type, redir_info.file_name);
        // fd_out(args, operador, file_name);
    }
    else {
        printf("proceso padre");
        waitpid(pid, &status, 0);
    }

    free(line);
    free(args);

    return 0;
}