#ifndef UTILITIES_H

#define UTILITIES_H

#include <stdio.h>

int new_matrix_char(char ***matrix, int n, int m);
int new_matrix_int(int ***matrix, int n, int m);

void free_matrix_char(char **matrix, int n);
void free_matrix_int(int **matrix, int n);

int add_line(char ***matrix, int n, char *line);

int safe_gets(char **str, FILE *in);

int concat_strings(char **dest, char *from);

int read_command(char ***args, int *cnt_args);

int parse_int(char *s, int *to);

int max(int a, int b);

int min(int a, int b);

void send_array(int *arr, int m, int to);
void recv_array(int *arr, int m, int from);

void send_matrix_char(char **matrix, int m, int first_line, int last_line, int to);
void send_matrix_int(int **matrix, int m, int first_line, int last_line, int to);

void recv_matrix_char(char **matrix, int m, int first_line, int last_line, int from);
void recv_matrix_int(int **matrix, int m, int first_line, int last_line, int from);

#endif
