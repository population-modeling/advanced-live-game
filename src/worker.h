#ifndef WORKER_H

#define WORKER_H

#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>

#include "field.h"

#define MAX_WORKERS 1000
#define TURN_LENGTH 200

extern int cnt_workers;
extern struct Field field;
extern struct Field working_field[2];
extern int world_size;
extern int node_number;
extern int steps;
extern int working_interval_l[MAX_WORKERS];
extern int working_interval_r[MAX_WORKERS];

int init_working_intervals();

void get_working_interval(int node_number, int *first_line, int *last_line);

void run();

#endif
