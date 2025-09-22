#include "include/shell.h"

/**
 * Leer la linea de entrada por teclado
 */
char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;

    /** 
     * getline() para leer la linea completa 
     * Si getline() retorno -1 se maneja el error
    */
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            /* Caso EOF (Ctrl+D), salida con éxito de la shell. */ 
            printf("\n");
            free(line);
            exit(EXIT_SUCCESS);
        }
        else {
            free(line);
            perror("error al leer la linea");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

/**
 * Dividir el string de entrada en multiples strings (substrings)
 */
char **split_line(char *line) {
    int bufsize = 64;
    int c = 0;
    char **tokens = malloc(bufsize * sizeof(char *));   /* sizeof(char *) -> 8 bytes */
    char *token;

    if (!tokens) {
        perror("error al asignar memoria en split_line: tokens\n");
        exit(EXIT_FAILURE);
    }

    /* Busca en line hasta el primer delimitador */
    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
        tokens[c] = token;
        c++;

        if (c >= bufsize) {
            bufsize += bufsize;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens) {
                perror("error al reasignar memoria en split_line: tokens");
                exit(EXIT_FAILURE);
            }
        }
        /* NULL para que continue desde la ultima posición */
        token = strtok(NULL, TOKEN_DELIM);
    }
    /* NULL para indicar el final de los argumentos de la linea */
    tokens[c] = NULL;
    
    return tokens;
}

int contar_comandos_pipeline(char **args) {
    int n_comandos = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            n_comandos++;
        }
    }
    n_comandos++;   /* +1 para el primer comando */
    return n_comandos;
}

char ***parse_pipeline(char **args, int n_comandos) {
    char ***comandos = malloc((n_comandos + 1) * sizeof(char **));   /* sizeof(char *) -> 8 bytes */
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