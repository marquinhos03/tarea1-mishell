#include "include/shell.h"

char *integrated_command_list[] = {
    "exit",
    "cd",
    "env",
    "help"
};

int (*integrated_func_list[])(char **) = {
    &command_exit,
    &command_cd,
    &command_env,
    &command_help
};

int command_help(char **args) {
    (void)(**args);
    size_t num_commands = sizeof(integrated_command_list) / sizeof(char *);

    printf("\n--- help mishell ---\n");
    printf("Comandos integrados:\n");
    for(size_t i = 0; i < num_commands; i++) {
        printf("  -> %s\n", integrated_command_list[i]);
    }

    printf("Utilice en comando 'man' para obtener información sobre otros programas.\n\n");
    return CONTINUE;
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

int command_env(char **args) {
    (void)(**args);
    int i = 0;
    while (environ[i]) {
        write(STDOUT_FILENO, environ[i], strlen(environ[i]));
        write(STDOUT_FILENO, "\n", 1);
        i++;
    }
    return CONTINUE;
}

int execute_builtin(char **args) {
    size_t num_commands = sizeof(integrated_command_list) / sizeof(char *);

    for(size_t i = 0; i < num_commands; i++) {
        if (strcmp(args[0], integrated_command_list[i]) == 0) {
            return (*integrated_func_list[i])(args);
        }
    }

    return NOT_BUILTIN;     // Solo si no se encontró un comando integrado
}