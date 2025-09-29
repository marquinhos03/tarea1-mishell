NAME = mishell

SRCDIR = src
INCLUDEDIR = include/..

SOURCES := $(wildcard $(SRCDIR)/*.c)
INCLUDES = -I$(INCLUDEDIR)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: $(NAME)

$(NAME): $(SOURCES)
	@echo "Compilando ..."
	$(CC) $(SOURCES) $(INCLUDES) -o $(NAME)

clean:
	@echo "Limpiando ejecutable ..."
	rm -f $(NAME)