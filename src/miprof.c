#include "include/shell.h"
#include "include/defines.h"

int execute_miprof(char **args) {
    char **comando;

    if (strcmp(args[1], MIPROF_EJEC) == 0) {
        comando = &args[2];
        return miprof_ejec(comando, NULL);
    }
    else if (strcmp(args[1], MIPROF_EJECSAVE) == 0) {
        comando = &args[3];
        char *file_name = args[2];
        return miprof_ejec(comando, file_name);
    }
    else {
        printf("Comando 'miprof %s' no válido\n", args[1]);
        printf("Uso: miprof [ejec | ejecsave archivo] comando args\n");
    }

    return CONTINUE;
}

int miprof_ejec(char **args, char *file_name) {
    pid_t pid;
    int status;

    miprof_info command_info;
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */
        execvp(args[0], args);

        perror("execvp error en miprof_ejec");  // Solo se ejecuta si hay error
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
            /**
             * WIFEXITED() y WIFSIGNALED() distintos de cero (true) si el proceso hijo termino 
             * de forma normal o fue terminado por una señal
            */
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        clock_gettime(CLOCK_REALTIME, &end_time);
    
        command_info = get_miprof_info(usage, start_time, end_time);

        /* Si */
        if (file_name != NULL) {
            return miprof_ejecsave(file_name, args[0], command_info);
        }

        printf("\nTiempo de ejecución Usuario: %.5f segundos\n", command_info.tiempo_usuario);
        printf("Tiempo de ejecución Sistema: %.5f segundos\n", command_info.tiempo_sistema);
        printf("Tiempo de ejecución Real: %.5f segundos\n", command_info.tiempo_real);
        printf("Peak de memoria máxima residente: %ld KB\n", command_info.maximum_resident_set);
    }

    return CONTINUE;
}

int miprof_ejecsave(char *file_name, char *command_name, miprof_info command_info) {
    struct stat st;
    FILE *file;

    if (stat(file_name, &st) == 0) {
        /* Si el archivo ya existe*/
        file = fopen(file_name, "a");
    }
    else {
        file = fopen(file_name, "w");
    }

    if (!file) {
        perror("miprof_ejecsave: error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    /* Pasar información al archivo */
    fprintf(file, "Comando: %s\n", command_name);
    fprintf(file, "Tiempo de usuario: %.5f segundos\n", command_info.tiempo_usuario);
    fprintf(file, "Tiempo de sistema: %.5f segundos\n", command_info.tiempo_sistema);
    fprintf(file, "Tiempo real: %.5f segundos\n", command_info.tiempo_real);
    fprintf(file, "Peak de memoria máxima residente: %ld KB\n", command_info.maximum_resident_set);
    fprintf(file, "\n");

    fclose(file);

    return CONTINUE;
}

miprof_info get_miprof_info(struct rusage usage, struct timespec start_time, struct timespec end_time) {
    miprof_info command_info;
    
    command_info.tiempo_usuario = (double)(usage.ru_utime.tv_sec) + (double)(usage.ru_utime.tv_usec) / 1000000.0;
    command_info.tiempo_sistema = (double)(usage.ru_stime.tv_sec) + (double)(usage.ru_stime.tv_usec) / 1000000.0;
    command_info.tiempo_real = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
    command_info.maximum_resident_set = usage.ru_maxrss;

    return command_info;
}