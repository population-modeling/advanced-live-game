#include <stdlib.h>
#include <pthread.h>

#include "storage.h"
#include "errors.h"
#include "utilities.h"

int init_storage(struct Storage *st, int n, int m) {
    int i, j;
    st->x = (int*)malloc(n * m * sizeof(int));
    if (st->x == NULL) {
        return MALLOC_FAIL;
    }
    st->y = (int*)malloc(n * m * sizeof(int));
    if (st->y == NULL) {
        free(st->x);
        return MALLOC_FAIL;
    }

    new_matrix_int(&st->added, n, m);
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++)
            st->added = 0;
    }
    st->n = n;
    st->sz = 0;
    pthread_mutex_init(&st->mutex, NULL);
    return SUCCESS;
}

void free_storage(struct Storage *st) {
    if (st == NULL) {
        return;
    }
    if (st->x != NULL) {
        free(st->x);
    }
    if (st->y != NULL) {
        free(st->y);
    }
    free_matrix_int(st->added, st->n);
    pthread_mutex_destroy(&st->mutex);    
}

void add(struct Storage *st, int x, int y, int step) {
    if (st == NULL || st->x == NULL || st->y == NULL) {
        return;
    }
    pthread_mutex_lock(&st->mutex);
    if (st->added[x][y] == step) {
        pthread_mutex_unlock(&st->mutex);
        return; 
    }
    st->x[st->sz] = x;
    st->y[st->sz] = y;
    st->sz++;
    pthread_mutex_unlock(&st->mutex);
}

void get(struct Storage *st, int *x, int *y) {
    if (st == NULL || st->x == NULL || st->y == NULL) {
        return;
    }
    pthread_mutex_lock(&st->mutex);
    if (st->sz == 0) {
        *x = *y = -1;
    } else {
        st->sz--;
        *x = st->x[st->sz];
        *y = st->y[st->sz];
    }
    pthread_mutex_unlock(&st->mutex);
}
