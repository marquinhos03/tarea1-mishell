#include "include/shell.h"

char *miprof_messages[] = {
    "\nTiempo de ejecución Usuario: %.5f segundos\n",
    "Tiempo de ejecución Sistema: %.5f segundos\n",
    "Tiempo de ejecución Real: %.5f segundos\n",
    "Peak de memoria máxima residente: %ld KB\n"
};

int count_args(char **args) {
    int count = 0;
    while (args[count] != NULL) {
        count++;
    }

    return count;
}

miprof_info get_miprof_info(struct rusage usage, struct timespec start_time, struct timespec end_time) {
    miprof_info command_info;
    
    command_info.tiempo_usuario = (double)(usage.ru_utime.tv_sec) + (double)(usage.ru_utime.tv_usec) / 1000000.0;
    command_info.tiempo_sistema = (double)(usage.ru_stime.tv_sec) + (double)(usage.ru_stime.tv_usec) / 1000000.0;
    command_info.tiempo_real = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
    command_info.maximum_resident_set = usage.ru_maxrss;

    return command_info;
}

miprof_info miprof_ejec(char **args) {
    pid_t pid;
    miprof_info command_info;
    int status;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);  // Tiempo inicio

    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */
        reset_child_signals();

        execvp(args[0], args);

        perror("error en execvp");
        _exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        /* Error en fork() */
        perror("error en miprof_ejec: fork");
    }
    else {
        /* Proceso padre */
        struct rusage usage;

        do {
            wait4(pid, &status, WUNTRACED, &usage);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
        clock_gettime(CLOCK_REALTIME, &end_time);  // Tiempo final

        /* Si el proceso hijo falló */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            command_info.status = -1;
            return command_info;
        }

        /* Capturar información respecto al tiempo de ejecución */
        command_info = get_miprof_info(usage, start_time, end_time);
        command_info.status = 0;
    }

    return command_info;
}

int miprof_ejecsave(char *file_name, char *command_name, miprof_info command_info) {
    struct stat st;
    FILE *file;

    if (stat(file_name, &st) == 0) {
        // Si el archivo ya existe
        file = fopen(file_name, "a");
    }
    else {
        file = fopen(file_name, "w");
    }

    if (!file) {
        perror("miprof_ejecsave: error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    // Pasar información al archivo
    fprintf(file, "Comando: %s\n", command_name);
    fprintf(file, miprof_messages[0], command_info.tiempo_usuario);
    fprintf(file, miprof_messages[1], command_info.tiempo_sistema);
    fprintf(file, miprof_messages[2], command_info.tiempo_real);
    fprintf(file, miprof_messages[3], command_info.maximum_resident_set);
    fprintf(file, "\n");

    fclose(file);
    printf("\nResultados guardados en: %s\n", file_name);
    return CONTINUE;
}

int execute_miprof(char **args) {
    char **comando;
    miprof_info command_info;
    
    if (strcmp(args[1], MIPROF_EJEC) == 0) {
        // Si se ejecuto miprof ejec
        if (count_args(args) < 3) {
            // Si falto un comando
            printf("Uso: miprof ejec <comando> args\n");
            return CONTINUE;
        }
        
        comando = &args[2];
        command_info = miprof_ejec(comando);

        if (command_info.status < 0) {
            // Si no se ingreso un comando válido (formato inválido)
            printf("Comando 'miprof %s %s' no válido\n", args[1], args[2]);
            return CONTINUE;
        }

        printf(miprof_messages[0], command_info.tiempo_usuario);
        printf(miprof_messages[1], command_info.tiempo_sistema);
        printf(miprof_messages[2], command_info.tiempo_real);
        printf(miprof_messages[3], command_info.maximum_resident_set);
    }
    else if (strcmp(args[1], MIPROF_EJECSAVE) == 0) {
        // Si se ejecuto miprof ejecsave
        if (count_args(args) < 4) {
            // Si falto un comando
            printf("Uso: miprof ejecsave [archivo] <comando> args\n");
            return CONTINUE;
        }

        comando = &args[3];
        char *file_name = args[2];
        command_info = miprof_ejec(comando);

        if (command_info.status < 0) {
            // Si no se ingreso archivo para guardar (formato inválido)
            printf("Comando 'miprof %s %s' no válido\n", args[1], args[2]);
            return CONTINUE;
        }

        return miprof_ejecsave(file_name, *comando, command_info);
    }
    return CONTINUE;
}