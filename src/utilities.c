#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <mpi.h>

#include "errors.h"
#include "utilities.h"

int new_matrix_char(char ***matrix, int n, int m) {
    int i;
    *matrix = (char**)malloc(n * sizeof(char*));
    if (*matrix == NULL) {
        return MALLOC_FAIL;
    }
    for (i = 0; i < n; i++) {
        (*matrix)[i] = (char*)malloc(m * sizeof(char));     
        if ((*matrix)[i] == NULL) {
            int j;
            for (j = 0; j < i; j++) {
                free((*matrix)[j]);
            }
            free(*matrix);
            return MALLOC_FAIL;
        }
    }
    return SUCCESS;
}

int new_matrix_int(int ***matrix, int n, int m) {
    int i;
    *matrix = (int**)malloc(n * sizeof(int*));
    if (*matrix == NULL) {
        return MALLOC_FAIL;
    }
    for (i = 0; i < n; i++) {
        (*matrix)[i] = (int*)malloc(m * sizeof(int));     
        if ((*matrix)[i] == NULL) {
            int j;
            for (j = 0; j < i; j++) {
                free((*matrix)[j]);
            }
            free(*matrix);
            return MALLOC_FAIL;
        }
    }
    return SUCCESS;
}

void free_matrix_char(char **matrix, int n) {
    int i;
    for (i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void free_matrix_int(int **matrix, int n) {
    int i;
    for (i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int add_line(char ***matrix, int n, char *line) {
    char **buf = NULL;
    buf = (char**)realloc(*matrix, (n + 1) * sizeof(char*));
    if (buf == NULL) {
        return MALLOC_FAIL;
    }
    *matrix = buf;
    (*matrix)[n] = line;
    return SUCCESS;
}

void shrink(char **s) {
    int len = strlen(*s);
    char *buf = (char*)realloc(*s, (len + 1) * sizeof(char));
    if (buf != NULL) {
        (*s) = buf;
    }
}

int add_symbol(char **dest, char c, int *len, int *cur_size) {
    int page_size = sysconf(_SC_PAGE_SIZE);
    char *buf = NULL;

    if (dest == NULL || len == NULL || cur_size == NULL) {
        return FILE_READING_FAIL;
    }

    if ((*len) + 1 >= (*cur_size)) {
        (*cur_size) += page_size;
        buf = (char*)realloc(*dest, (*cur_size) * sizeof(char));
        if (buf == NULL) {
            return FILE_READING_FAIL;
        } else {
            (*dest) = buf;
        }
    }
    (*dest)[*len] = c;
    (*len)++;
    (*dest)[*len] = 0;

    return SUCCESS;
}

int safe_gets(char **str, FILE *in) {
    int len = 0;
    char *buf = NULL;
    int page_size = sysconf(_SC_PAGE_SIZE);
    int cur_size = page_size;

    if (in == NULL || str == NULL) {
        return FILE_READING_FAIL;
    }

    buf = (char*)realloc(*str, cur_size * sizeof(char));
    if (buf == NULL) {
        return FILE_READING_FAIL;
    } else {
        (*str) = buf;
    }
    (*str[0]) = 0;

    while (1) {
        int c = fgetc(in);
        if (ferror(in)) {
            return FILE_READING_FAIL;
        }
        if (c == EOF && len == 0) {
            return EMPTY_FILE;
        }
        if (c == EOF || c == '\n') {
            break;
        }
        if (len + 1 >= cur_size) {
            cur_size += page_size;
            buf = (char*)realloc(*str, cur_size * sizeof(char));
            if (buf == NULL) {
                return FILE_READING_FAIL;
            } else {
                (*str) = buf;
            }
        }
        (*str)[len] = (char)c;
        (*str)[len + 1] = 0;
        len++;
    }
    
    buf = (char*)realloc(*str, (len + 1) * sizeof(char));
    if (buf != NULL) {
        (*str) = buf;
    }
    return SUCCESS;
}

int concat_strings(char **dest, char *from) {
    char *buf = 
        (char*)realloc(*dest, (strlen(*dest) + strlen(from) + 1) * sizeof(char));
    if (buf == NULL) {
        return MALLOC_FAIL;
    }
    *dest = buf;
    strcat(*dest, from);
    return SUCCESS;
}

void copy(char *dest, char *from, int l, int r) {
    int i;
    for (i = l; i <= r; i++) {
        dest[i - l] = from[i];
    }
    dest[r - l + 1] = 0;
}

int get_arg(char ***args, int cur, char *command, int l, int r) {
    (*args)[cur] = (char*)malloc((r - l + 2) * sizeof(char));
    if ((*args)[cur] == NULL) {
        free_matrix_char(*args, cur - 1);
        free(command);
        return MALLOC_FAIL;
    }
    copy((*args)[cur], command, l, r);
    return SUCCESS;
}

int read_command(char ***args, int *cnt_args) {
    char *command = NULL;
    int i;
    int len, last, cur;
    int start = 0;

    if (safe_gets(&command, stdin) != SUCCESS) {
        return FILE_READING_FAIL;
    }
    len = strlen(command);
    *cnt_args = 0;
    start = len;
    for (i = 0; i < len; i++) {
        if (command[i] != ' ') {
            start = i;
            break;
        }
    }

    last = start - 1;
    for (i = start; i < len; i++) {
        if (command[i] == ' ') {
            if (last == start - 1 || last != i - 1) {
                (*cnt_args)++;
            }
            last = i;
        }
    }
    if (last != len - 1) {
        (*cnt_args)++;
    }

    *args = (char**)malloc((*cnt_args) * sizeof(char*));
    if (*args == NULL) {
        free(command);
        return MALLOC_FAIL;
    }
    last = start - 1;
    cur = 0;
    for (i = start; i < len; i++) {
        if (command[i] == ' ') {
            if (last == start - 1 || last != i - 1) {
                if (get_arg(args, cur, command, last + 1, i - 1) != SUCCESS) {
                    return MALLOC_FAIL;
                }
                cur++;
            }
            last = i;
        }
    }
    if (last != len - 1) {
        if (get_arg(args, cur, command, last + 1, len - 1) != SUCCESS) {
            return MALLOC_FAIL;
        }
    }
    free(command);
    return SUCCESS;
}

int parse_int(char *s, int *to) {
    int len = strlen(s);
    int i;

    if (len > 10) {
        return INVALID_ARGUMENT;
    }
    for (i = 0; i < len; i++) {
        if (s[i] < '0' || s[i] > '9') {
            return INVALID_ARGUMENT;
        }
    }
    long long res = atoll(s);
    *to = res;
    if (*to != res) {
        return INVALID_ARGUMENT;
    }
    return SUCCESS;
}

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

void send_array(int *arr, int m, int to) {
    MPI_Bsend(arr, m, MPI_INT, 
              to, 0, MPI_COMM_WORLD);
}

void recv_array(int *arr, int m, int from) {
    MPI_Recv(arr, m, MPI_INT, 
              from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void send_matrix_char(char **matrix, int m, int first_line, int last_line, int to) {
    int i;
    for (i = first_line; i < last_line; i++) {
        MPI_Bsend(matrix[i], m, MPI_CHAR, 
                  to, 0, MPI_COMM_WORLD);
    }
}

void send_matrix_int(int **matrix, int m, int first_line, int last_line, int to) {
    int i;
    for (i = first_line; i < last_line; i++) {
        MPI_Bsend(matrix[i], m, MPI_INT, 
                  to, 0, MPI_COMM_WORLD);
    }
}

void recv_matrix_char(char **matrix, int m, int first_line, int last_line, int from) {
    int i;
    for (i = first_line; i < last_line; i++) {
        MPI_Recv(matrix[i], m, MPI_CHAR, 
                  from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void recv_matrix_int(int **matrix, int m, int first_line, int last_line, int from) {
    int i;
    for (i = first_line; i < last_line; i++) {
        MPI_Recv(matrix[i], m, MPI_INT, 
                  from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}
