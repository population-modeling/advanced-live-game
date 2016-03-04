#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "errors.h"
#include "field.h"
#include "utilities.h"

int print_field(struct Field *field) {
    int i, j;
    puts("First bact");
    for (i = 0; i < field->n; i++) {
        for (j = 0; j < field->m; j++) {
            if (printf("%c", field->bact1[i][j]) < 0) {
                return IO_FAIL;
            }
        }
        printf("\n");
    }
    puts("Second bact");
    for (i = 0; i < field->n; i++) {
        for (j = 0; j < field->m; j++) {
            if (printf("%c", field->bact2[i][j]) < 0) {
                return IO_FAIL;
            }
        }
        printf("\n");
    }
    return SUCCESS;
}

int get_empty_field(struct Field *field, int n, int m) {
    int i, j;
    field->n = n;
    field->m = m;
    if (new_matrix_char(&field->bact1, n, m) != SUCCESS) {
        return MALLOC_FAIL;
    }
    if (new_matrix_int(&field->hunger, n, m) != SUCCESS) {
        return MALLOC_FAIL;
    }

    if (new_matrix_char(&field->bact2, n, m) != SUCCESS) {
        return MALLOC_FAIL;
    }

    if (new_matrix_int(&field->env, n, m) != SUCCESS) {
        return MALLOC_FAIL;
    }
    if (new_matrix_int(&field->subst1, n, m) != SUCCESS) {
        return MALLOC_FAIL;
    }
    if (new_matrix_int(&field->subst2, n, m) != SUCCESS) {
        return MALLOC_FAIL;
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            field->bact1[i][j] = field->bact2[i][j] = '0';
            field->hunger[i][j] = field->env[i][j] = 
                field->subst1[i][j] = field->subst2[i][j] = 0;
        }
    }

    return SUCCESS;
}

int get_random_field(struct Field *field, int n, int m) {
    int i, j;
    srand(43);
    get_empty_field(field, n, m);

    field->max_hunger = rand() % 50;
    field->need_env = rand() % 100;
    field->need_subst2 = rand() % 100;
    field->prod_subst1 = rand() % 100;
    field->death_subst1 = rand() % 10000 + 
        max(field->reprod_subst1, field->need_subst1);

    field->need_subst1 = rand() % 100;
    field->prod_subst2 = rand() % 100;
    field->reprod_subst2 = rand() % 100;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            (field->bact1)[i][j] = '0' + rand() % 2;
            (field->hunger)[i][j] = 0;

            (field->bact2)[i][j] = '0' + rand() % 2;

            (field->env)[i][j] = rand() % 10000;
            (field->subst1)[i][j] = rand() % 10000;
            (field->subst2)[i][j] = rand() % 10000;
        }
    }
    return SUCCESS;
}

int get_field_from_file(struct Field *field) {
    FILE *fin = NULL;
    int x, y;
    int env, subst1, subst2, hunger;
    char bact1, bact2;
    fin = fopen("state.txt", "r");
    if (fin == NULL) {
        return FILE_OPENING_FAIL;
    }

    assert(fscanf(fin, "%d %d", &field->n, &field->m));
    get_empty_field(field, field->n, field->m);
    while (fscanf(fin, "%d %d %d %d %d %c %d %c", 
                &x, &y, &env, &subst1, &subst2, &bact1, &hunger, &bact2) != 0) {
        field->bact1[x][y] = bact1;
        field->bact2[x][y] = bact2;
        field->env[x][y] = env;
        field->subst1[x][y] = subst1;
        field->subst2[x][y] = subst2;
        field->hunger[x][y] = hunger;
    }
    fclose(fin);

    fin = fopen("creatures.conf", "r");
    if (fin == NULL) {
        return FILE_OPENING_FAIL;
    }
    assert(fscanf(fin, "%d%d%d%d%d%d%d%d%d", 
            &field->need_env,
            &field->need_subst2,
            &field->prod_subst1,
            &field->reprod_subst1,
            &field->max_hunger,
            &field->death_subst1,
            &field->need_subst1,
            &field->prod_subst2,
            &field->reprod_subst2));
    fclose(fin);
    return SUCCESS;
}

void free_field(struct Field *field) {
    if (field->bact1 != NULL) {
        free_matrix_char(field->bact1, field->n);
        free_matrix_int(field->hunger, field->n);

        free_matrix_char(field->bact2, field->n);

        free_matrix_int(field->env, field->n);
        free_matrix_int(field->subst1, field->n);
        free_matrix_int(field->subst2, field->n);
    }
}

void copy_field(struct Field *to, struct Field *from) {
    int i, j;
    to->n = from->n;
    to->m = from->m;

    to->need_env = from->need_env;
    to->need_subst2 = from->need_subst2;
    to->prod_subst1 = from->prod_subst1;
    to->reprod_subst1 = from->reprod_subst1;
    to->death_subst1 = from->death_subst1;
    to->max_hunger = from->max_hunger;

    to->need_subst1 = from->need_subst1;
    to->prod_subst2 = from->prod_subst2;
    to->reprod_subst2 = from->reprod_subst2;
    for (i = 0; i < to->n; i++) {
        for (j = 0; j < to->m; j++) {
            to->bact1[i][j] = from->bact1[i][j];
            to->hunger[i][j] = from->hunger[i][j];

            to->bact2[i][j] = from->bact2[i][j];

            to->env[i][j] = from->env[i][j];
            to->subst1[i][j] = from->subst1[i][j];
            to->subst2[i][j] = from->subst2[i][j];
        }
    }
}

void send_field(struct Field *field, int first_line, int last_line, int to) {
    send_matrix_char(field->bact1, field->m, first_line, last_line, to);
    send_matrix_int(field->hunger, field->m, first_line, last_line, to);

    send_matrix_char(field->bact2, field->m, first_line, last_line, to);

    send_matrix_int(field->env, field->m, first_line, last_line, to);
    send_matrix_int(field->subst1, field->m, first_line, last_line, to);
    send_matrix_int(field->subst2, field->m, first_line, last_line, to);
}

void recv_field(struct Field *field, int first_line, int last_line, int from) {
    recv_matrix_char(field->bact1, field->m, first_line, last_line, from);
    recv_matrix_int(field->hunger, field->m, first_line, last_line, from);

    recv_matrix_char(field->bact2, field->m, first_line, last_line, from);

    recv_matrix_int(field->env, field->m, first_line, last_line, from);
    recv_matrix_int(field->subst1, field->m, first_line, last_line, from);
    recv_matrix_int(field->subst2, field->m, first_line, last_line, from);
}

void copy_info(struct Field *to, struct Field *from) {
    to->n = from->n;
    to->m = from->m;
    
    to->need_env = from->need_env;
    to->need_subst2 = from->need_subst2;
    to->prod_subst1 = from->prod_subst1;
    to->reprod_subst1 = from->reprod_subst1;
    to->max_hunger = from->max_hunger;
    to->death_subst1 = from->death_subst1;

    to->need_subst1 = from->need_subst1;
    to->prod_subst2 = from->prod_subst2;
    to->reprod_subst2 = from->reprod_subst2;
}
