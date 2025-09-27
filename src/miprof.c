#include "../include/shell.h"
#include <sys/time.h>  // Para gettimeofday

// Versión portable de obtener tiempo
void get_current_time(struct timespec *tp) {
    #if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
    clock_gettime(CLOCK_REALTIME, tp);
    #else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tp->tv_sec = tv.tv_sec;
    tp->tv_nsec = tv.tv_usec * 1000;
    #endif
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
    get_current_time(&start_time);  // Tiempo inicio

    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */
        execvp(args[0], args);

        perror("error en execvp");
        _exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        /* Error en fork() */
        perror("error en miprof_ejec: fork");
        command_info.status = -1;
        return command_info;
    }
    else {
        /* Proceso padre */
        struct rusage usage;
        
        // Usar wait3 que es más portable que wait4
        #if defined(HAVE_WAIT3)
        wait3(&status, WUNTRACED, &usage);
        #else
        // Alternativa con waitpid + getrusage
        waitpid(pid, &status, WUNTRACED);
        getrusage(RUSAGE_CHILDREN, &usage);
        #endif

        while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
            #if defined(HAVE_WAIT3)
            wait3(&status, WUNTRACED, &usage);
            #else
            waitpid(pid, &status, WUNTRACED);
            getrusage(RUSAGE_CHILDREN, &usage);
            #endif
        }

        get_current_time(&end_time);  // Tiempo final

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
    FILE *file = fopen(file_name, "a");
    if (!file) {
        perror("miprof_ejecsave: error al abrir el archivo");
        return -1;
    }

    /* Pasar información al archivo */
    fprintf(file, "Comando: %s\n", command_name);
    fprintf(file, "Tiempo de usuario: %.5f segundos\n", command_info.tiempo_usuario);
    fprintf(file, "Tiempo de sistema: %.5f segundos\n", command_info.tiempo_sistema);
    fprintf(file, "Tiempo real: %.5f segundos\n", command_info.tiempo_real);
    fprintf(file, "Peak de memoria máxima residente: %ld KB\n", command_info.maximum_resident_set);
    fprintf(file, "\n");

    fclose(file);
    return 0;
}

int execute_miprof(char **args) {
    char **comando;
    miprof_info command_info;

    if (args[1] == NULL) {
        printf("Uso: miprof [ejec | ejecsave archivo] comando args\n");
        return CONTINUE;
    }

    if (strcmp(args[1], MIPROF_EJEC) == 0) {
        if (args[2] == NULL) {
            printf("Uso: miprof ejec <comando> [args]\n");
            return CONTINUE;
        }
        
        comando = &args[2];
        command_info = miprof_ejec(comando);

        if (command_info.status < 0) {
            printf("Error al ejecutar el comando\n");
            return CONTINUE;
        }

        printf("\nTiempo de ejecución Usuario: %.5f segundos\n", command_info.tiempo_usuario);
        printf("Tiempo de ejecución Sistema: %.5f segundos\n", command_info.tiempo_sistema);
        printf("Tiempo de ejecución Real: %.5f segundos\n", command_info.tiempo_real);
        printf("Peak de memoria máxima residente: %ld KB\n", command_info.maximum_resident_set);
    }
    else if (strcmp(args[1], MIPROF_EJECSAVE) == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            printf("Uso: miprof ejecsave <archivo> <comando> [args]\n");
            return CONTINUE;
        }
        
        comando = &args[3];
        char *file_name = args[2];
        command_info = miprof_ejec(comando);

        if (command_info.status < 0) {
            printf("Error al ejecutar el comando\n");
            return CONTINUE;
        }

        if (miprof_ejecsave(file_name, args[3], command_info) == 0) {
            printf("Resultados guardados en: %s\n", file_name);
        }
    }
    else {
        printf("Comando 'miprof %s' no válido\n", args[1]);
        printf("Uso: miprof [ejec | ejecsave archivo] comando args\n");
    }

    return CONTINUE;
}