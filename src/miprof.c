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
    miprof_info cmd_info;
    redirection_info redir_info;
    int status;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);  // Tiempo inicio

    pid = fork();
    if (pid == 0) {
        /* Proceso hijo */
        reset_child_signals();

        redir_info = get_redirection_info(args);
        if (redir_info.type != REDIR_NULL) {
            redirect_stdout_to_file(redir_info.type, redir_info.file_name);
        }

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
            cmd_info.status = STATUS_EXEC_FAIL;
            return cmd_info;
        }

        /* Capturar información respecto al tiempo de ejecución */
        cmd_info = get_miprof_info(usage, start_time, end_time);
        cmd_info.status = 0;
    }

    return cmd_info;
}



int miprof_ejecsave(char *file_name, char *command_name, miprof_info info) {
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
    fprintf(file, miprof_messages[0], info.tiempo_usuario);
    fprintf(file, miprof_messages[1], info.tiempo_sistema);
    fprintf(file, miprof_messages[2], info.tiempo_real);
    fprintf(file, miprof_messages[3], info.maximum_resident_set);
    fprintf(file, "\n");

    fclose(file);
    fprintf(stderr, "\nResultados guardados en: %s\n", file_name);
    return CONTINUE;
}



miprof_info miprof_ejecutar_maxtiempo(char **args, int maxtiempo) {
    pid_t pid;
    miprof_info info;
    int status;
    struct timespec start_time, end_time;

    clock_gettime(CLOCK_REALTIME, &start_time);

    pid = fork();
    if (pid == 0) {
        /* --- Proceso hijo --- */
        reset_child_signals();

        execvp(args[0], args);

        perror("execvp error en miprof_ejecutar_maxtiempo");
        _exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("fork error en miprof_ejecutar_maxtiempo");
    }
    else {
        /* --- Proceso padre --- */
        struct rusage usage;
        alarm(maxtiempo);   // Activar alarma

        do {
            if (wait4(pid, &status, 0, &usage) == -1) {
                /**wait4 falló
                 * Se verifica si fue por alarma
                 */
                if (errno == EINTR) {
                    /* SIGALARM interrumpe */
                    kill(pid, SIGKILL); // Matar proceso hijo

                    wait4(pid, &status, 0, &usage);
                    alarm(0);   // Desactivar alarma

                    clock_gettime(CLOCK_REALTIME, &end_time);
                    info = get_miprof_info(usage, start_time, end_time);
                    info.status = STATUS_TIMEOUT;
                    return info;
                }
                /* Si fue por otra razon */
                perror("miprof_ejecutar_maxtiempo: wait4 error fatal");
                info.status = STATUS_EXEC_FAIL;
                return info;
            }
        } while(!(WIFEXITED(status)) && !(WIFSIGNALED(status)));

        alarm(0);   // Desactivar alarma
        
        clock_gettime(CLOCK_REALTIME, &end_time);

        /* Si el proceso hijo falló */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            info.status = STATUS_EXEC_FAIL;
            return info;
        }

        info = get_miprof_info(usage, start_time, end_time);
        info.status = 0;
    }

    return info;
}



int execute_miprof(char **args) {
    char **comando;
    miprof_info command_info;
    
    if (strcmp(args[1], MIPROF_EJEC) == 0) {
        // Si se ejecuto miprof ejec
        if (count_args(args) < 3) {
            // Si falto un comando
            fprintf(stderr, "Uso: miprof ejec <comando> args\n");
            return CONTINUE;
        }
        
        comando = &args[2];
        command_info = miprof_ejec(comando);

        if (command_info.status == STATUS_EXEC_FAIL) {
            // Si no se ingreso un comando válido (formato inválido)
            fprintf(stderr, "Comando 'miprof %s %s' no válido\n", args[1], args[2]);
            return CONTINUE;
        }
        // Continuar para imprimir
    }
    else if (strcmp(args[1], MIPROF_EJECSAVE) == 0) {
        // Si se ejecuto miprof ejecsave
        if (count_args(args) < 4) {
            // Si falto un comando
            fprintf(stderr, "Uso: miprof ejecsave [archivo] <comando> args\n");
            return CONTINUE;
        }

        comando = &args[3];
        char *file_name = args[2];
        command_info = miprof_ejec(comando);

        if (command_info.status == STATUS_EXEC_FAIL) {
            // Si no se ingreso archivo para guardar (formato inválido)
            fprintf(stderr, "Comando 'miprof %s %s' no válido\n", args[1], args[2]);
            return CONTINUE;
        }

        return miprof_ejecsave(file_name, *comando, command_info);
    }
    else if (strcmp(args[1], MIPROF_EJECUTAR) == 0 && strcmp(args[2], MIPROF_MAXTIEMPO) == 0) {
        // Si se ejecuto "miprof ejecutar"
        if (count_args(args) < 5) {
            // Si falto un comando
            fprintf(stderr, "Uso: miprof ejecutar maxtiempo [maxtiempo] <comando> args\n");
            return CONTINUE;
        }

        int maxtiempo = atoi(args[3]);
        comando = &args[4];

        if (maxtiempo <= 0) {
            fprintf(stderr, "Error: El tiempo máximo debe ser un entero positivo (segundos > 0).\n");
            return CONTINUE;
        }

        command_info = miprof_ejecutar_maxtiempo(comando, maxtiempo);
        if (command_info.status == STATUS_EXEC_FAIL) {
            // Si no se ingreso un comando válido (formato inválido)
            fprintf(stderr, "Comando 'miprof %s %s' no válido\n", args[1], args[2]);
            return CONTINUE;
        }
        else if (command_info.status == STATUS_TIMEOUT) {
            // Si no se cumplio el maxtiempo
            fprintf(stderr, "\n--- EJECUCIÓN INTERRUMPIDA ---\n");
            fprintf(stderr, "Proceso <%s> terminado por TIMEOUT (Tiempo límite: %d segundos).\n", *comando, maxtiempo);
        }
        // Continuar para imprimir
    }
    else {
        // Formato inválido de miprof
        fprintf(stderr, "Uso: miprof [ejec | ejecsave archivo] <comando> args\n");
        return CONTINUE;
    }

    if (command_info.status != STATUS_EXEC_FAIL) {
        // Si el comando no falló en la ejecución, imprimimos el perfilamiento.
        fprintf(stderr, miprof_messages[0], command_info.tiempo_usuario);
        fprintf(stderr, miprof_messages[1], command_info.tiempo_sistema);
        fprintf(stderr, miprof_messages[2], command_info.tiempo_real);
        fprintf(stderr, miprof_messages[3], command_info.maximum_resident_set);
    }

    return CONTINUE;
}