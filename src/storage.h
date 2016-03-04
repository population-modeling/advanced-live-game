#ifndef STORAGE_H

#define STORAGE_H

#include <pthread.h>
#include <stdbool.h>

struct Storage {
    int *x;
    int *y;
    int **added;

    int sz;
    int n;

    pthread_mutex_t mutex;
};

int init_storage(struct Storage *st, int n, int m);
void free_storage(struct Storage *st); 

void add(struct Storage *st, int x, int y, int step);
void get(struct Storage *st, int *x, int *y);

#endif
