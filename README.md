# Tarea 1: Mishell

### Objetivos

> Introducir a los estudiantes en el manejo de procesos concurrentes en Unix, creación, ejecución y
terminación usando llamadas a sistemas **fork()**, **exec()** y **wait()**. Además el uso de otras llamadas a sistema como **signals** y comunicación entre procesos usando **pipes**.


## 📁 Files

| Nombre | Descripción |
| :--- | :--- |
| shell.h | Archivo de cabecera. |
| main.c | Arranque de la shell. |
| builtin.c | Implementación de comandos integrados como cd, exit, env y help. |
| input.c | Funciones encargadas de la lectura de comandos desde el teclado. |
| miprof.c | Implementación de miprof [ejec / ejecsave] y miprof ejecutar maxtiempo. |
| pipes.c | Manejo de comandos comunicados por pipes. |
| redirection.c | Manejo de los operadores de redirección de salida (>, >>). |
| shell_utils.c | Funcionamiento de la shell para comandos simples, pipelines, etc. |
| signals.c | Gestion de señales en procesos (padre, hijo, timeout). |

## 💻 Instalación, Compilación y Ejecución

Pasos para clonar, compilar y ejecutar mishell en su sistema.

### 1. Clonar el Repositorio

```
 git clone https://github.com/marquinhos03/tarea1-mishell.git
 cd tarea1-mishell
```

### 2. Compilación

El proyecto utiliza un Makefile para la compilación. Ejecute el comando make para crear el ejecutable mishell

```
 make
```
### 3. Ejecución

```
 ./mishell
```

## 📝 Ejemplos

* **Ejemplos de uso principales de mishell**

```
 mishell$ ls -l src
 total 44
 -rw-r--r-- 1 root root 1619 Sep 27 20:35 builtin.c
 -rw-r--r-- 1 root root 1658 Sep 26 22:11 input.c
 -rw-r--r-- 1 root root  153 Sep 27 20:40 main.c
 -rw-r--r-- 1 root root 8315 Sep 28 19:05 miprof.c
 -rw-r--r-- 1 root root 4150 Sep 28 19:08 pipes.c
 -rw-r--r-- 1 root root 1562 Sep 26 22:12 redirection.c
 -rw-r--r-- 1 root root 2356 Sep 28 18:25 shell_utils.c
 -rw-r--r-- 1 root root 1051 Sep 28 17:51 signals.c
```

* **Comando miprof**

```
 mishell$ miprof ejec cat src/miprof.c
 Contenido del archivo
 ...
 Tiempo de ejecución Usuario: 0.00097 segundos
 Tiempo de ejecución Sistema: 0.00000 segundos
 Tiempo de ejecución Real: 0.00273 segundos
 Peak de memoria máxima residente: 1792 KB
```
```
 mishell$ miprof ejecsave resultados.txt ls -l
 total 48
 -rw-r--r-- 1 root root   314 Sep 26 22:00 Makefile
 -rw-r--r-- 1 root root  1870 Sep 28 20:09 README.md
 drwxr-xr-x 2 root root  4096 Sep 26 22:00 include
 -rwxr-xr-x 1 root root 31576 Sep 28 19:16 shell
 drwxr-xr-x 2 root root  4096 Sep 27 20:39 src

 Resultados guardados en: resultados.txt
```
```
 mishell$ miprof ejecutar maxtiempo 1 sleep 5
 
 --- EJECUCIÓN INTERRUMPIDA ---
 Proceso <sleep> terminado por TIMEOUT (Tiempo límite: 1 segundos). 
 
 Tiempo de ejecución Usuario: 0.00120 segundos
 Tiempo de ejecución Sistema: 0.00000 segundos
 Tiempo de ejecución Real: 1.00096 segundos
 Peak de memoria máxima residente: 1664 KB
```

* **Pipelines**

```
 mishell$ ls | miprof ejec cat | wc

 Tiempo de ejecución Usuario: 0.00000 segundos
 Tiempo de ejecución Sistema: 0.00151 segundos
 Tiempo de ejecución Real: 0.00194 segundos
 Peak de memoria máxima residente: 1664 KB
      5       5      37
```
* **Redirección de Salida (>, >>)**

```
 mishell$ cat src/shell_utils.c | grep "PROMPT" | wc > test.txt
 mishell$ cat include/shell.h | grep "PROMPT" | wc >> test.txt
```
