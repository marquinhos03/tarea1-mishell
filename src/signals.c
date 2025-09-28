#include "include/shell.h"

/* Handler para SIGINT (CTRL+C) */
void sigint_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
}

/* Handler para SIGALRM */
void sigalrm_handler(int sig) {
	(void)sig;
}

void parent_signals() {
    struct sigaction sa_int, sa_quit, sa_alrm;

    /* Configuración de SIGINT (CTRL+C) */
	sa_int.sa_handler = &sigint_handler;
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = SA_RESTART; // Reiniciar llamadas de sistema interrumpidas
	sigaction(SIGINT, &sa_int, NULL);

	/* Configuración de SIGQUIT (CTRL+\) para ser ignorado */
	sa_quit.sa_handler = SIG_IGN; // Ignorar
	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_flags = 0;
	sigaction(SIGQUIT, &sa_quit, NULL);

	/* Configuración de SIGALARM */
	sa_alrm.sa_handler = &sigalrm_handler;
	sigemptyset(&sa_alrm.sa_mask);
	sa_alrm.sa_flags = 0;
	sigaction(SIGALRM, &sa_alrm, NULL);

}

void reset_child_signals() {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
}