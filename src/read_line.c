#include "include/shell.h"

char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;

    /** 
     * getline() para leer la linea completa 
     * Si getline() retorno -1 se maneja el error
    */
    if (getline(&line, &bufsize, stdin) == -1) {
        /* Si llega al final de un archivo (!= 0)*/
        if (feof(stdin)) {
            free(line);
            exit(EXIT_SUCCESS);
        }
        else {
            free(line);
            perror("Error al leer la linea");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}