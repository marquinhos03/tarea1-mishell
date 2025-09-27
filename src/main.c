#include "include/shell.h"

int main() {
    // Configurar señales
    parent_signals();

    // Iniciar shell
    shell_interactive();

    return 0;
}