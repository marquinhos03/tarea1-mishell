#ifndef DEFINES_H
#define DEFINES_H

#define BLUE "\x1b[34m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"

typedef enum e_redirection_type {
    REDIR_TRUNC,    // >
    REDIR_APPEND,   // >>
    REDIR_NULL
} t_redirection_type;

typedef struct s_redirection_info {
    t_redirection_type type;
    char *file_name;
} t_redirection_info;

extern char *redirection_string[];

#endif