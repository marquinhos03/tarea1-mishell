#include "include/shell.h"

#define TOK_DELIM " \t\r\n\a\""

void shell_interactive() {
    char *line;
    char **args;
    int status = -1;

    while (1) {
        printf("mishell$  ");

        status++;
        if (status >= 0) {
            exit(status);
        }
    }
}

/**
 * Dividir un string en multiples strings (substrings)
 */
char **split_line(char *line) {
    int bufsize = 64;
    int i = 0;
    char **tokens = malloc(bufsize * sizeof(char *));   /* sizeof(char *) -> 8 bytes */
    char *token;

    if (!tokens) {
        perror("Error al asignar memoria en split_line: tokens\n");
        exit(EXIT_FAILURE);
    }

    /* Busca en line hasta el primer delimitador */
    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[i] = token;
        i++;
        
        if (i >= bufsize) {
            bufsize += bufsize;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens) {
                fprintf(stderr, "Error al reasignar memoria en split_line: tokens");
                exit(EXIT_FAILURE);
            }
        }
        /* NULL para que continue desde la ultima posición */
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[i] = NULL;
    
    return tokens;
}


int main() {
    
    char *line;
    char **args;

    line = read_line();
    args = split_line(line);

    for (int i = 0; args[i] != NULL; i++) {
        printf("Token %d: %s\n", i, args[i]);
    }

    free(line);
    free(args);

    return 0;
}
