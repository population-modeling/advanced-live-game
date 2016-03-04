#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include <mpi.h>

#include "worker.h"
#include "field.h"
#include "errors.h"
#include "utilities.h"
#include "storage.h"

const int CNT_THREADS = 4;

struct Field field;
struct Field working_field[2];
const int dx[4] = { 1, -1, 0, 0 };
const int dy[4] = { 0, 0, 1, -1 };

int cnt_workers;
int world_size;
int node_number;
int steps;

int working_interval_l[MAX_WORKERS];
int working_interval_r[MAX_WORKERS];

void recalc_cell(int step, int i, int j) {
    int from = step % 2;
    int to = 1 - from;

    /* copy */
    working_field[to].env[i][j] = working_field[from].env[i][j];
    working_field[to].subst1[i][j] = working_field[from].subst1[i][j];
    working_field[to].subst2[i][j] = working_field[from].subst2[i][j];
    working_field[to].bact1[i][j] = working_field[from].bact1[i][j];
    working_field[to].bact2[i][j] = working_field[from].bact2[i][j];
    
    /* bact1 */
    if (working_field[from].bact1[i][j] == '1') {
        /* life */
        if (working_field[from].env[i][j] >= working_field[from].need_env &&
                working_field[from].subst2[i][j] >= working_field[from].need_subst2) {
            working_field[to].env[i][j] -= working_field[from].need_env;
            working_field[to].subst2[i][j] -= working_field[from].need_subst2;
            working_field[to].subst1[i][j] += working_field[from].prod_subst1;
        } else {
            working_field[to].hunger[i][j]++;
            if (working_field[to].hunger[i][j] > working_field[to].max_hunger) {
                working_field[to].hunger[i][j] = 0;
                working_field[to].bact1[i][j] = '0';
            }
        }
        /* reproduction */
        if (working_field[to].subst1[i][j] >= working_field[to].reprod_subst1) {
            int dir, cand;
            int cnt_ok = 0;
            int ok[4] = { 0, 0, 0, 0 };
            for (dir = 0; dir < 4; dir++) {
                int x = (i + dx[dir]) % field.n, y = (j + dy[dir]) % field.m; 
                if (working_field[to].bact1[x][y] == '0') {
                    ok[dir] = 1;
                    cnt_ok++;
                }
            }
            cand = rand() % cnt_ok; 
            for (dir = 0; dir < 4; dir++) {
                if (ok[dir]) {
                    if (cand == 0) {
                        int x = (i + dx[dir]) % field.n, y = (j + dy[dir]) % field.m; 
                        working_field[to].bact1[x][y] = '1';
                    } else {
                        cand--;
                    }
                }
            }
        }
        /* death */
        if (working_field[to].subst1[i][j] >= working_field[to].death_subst1) {
            working_field[to].bact1[i][j] = '0';
        }
    }

    /* bact2 */
    if (working_field[from].bact2[i][j] == '1') {
        /* life */
        if (working_field[from].subst1[i][j] >= working_field[from].need_subst1) {
            working_field[to].subst1[i][j] -= working_field[from].need_subst1;
            working_field[to].subst2[i][j] += working_field[from].prod_subst2;
        } else {
            working_field[to].bact1[i][j] = '0';
        }
        /* reproduction */
        if (working_field[to].subst2[i][i] >= working_field[to].reprod_subst2) {
            int dir, cand;
            int cnt_ok = 0;
            int ok[4] = { 0, 0, 0, 0 };
            for (dir = 0; dir < 4; dir++) {
                int x = (i + dx[dir]) % field.n, y = (j + dy[dir]) % field.m; 
                if (working_field[to].bact2[x][y] == '0') {
                    ok[dir] = 1;
                    cnt_ok++;
                }
            }
            cand = rand() % cnt_ok; 
            for (dir = 0; dir < 4; dir++) {
                if (ok[dir]) {
                    if (cand == 0) {
                        int x = (i + dx[dir]) % field.n, y = (j + dy[dir]) % field.m; 
                        working_field[to].bact2[x][y] = '1';
                    } else {
                        cand--;
                    }
                }
            }
        }
    }
}

void free_workers() {
    free_field(&working_field[0]);
    free_field(&working_field[1]);
}

int init_working_intervals() {
    int cnt_alive = 0;
    int *alive_in_line = (int*)malloc(field.n * sizeof(int));
    int i, j;
    int per_line;
    int cur = 0;
    int sum = 0;
    if (alive_in_line == NULL) {
        return MALLOC_FAIL;
    }
    
    for (i = 0; i < field.n; i++) {
        for (j = 0; j < field.m; j++) {
            if (field.bact1[i][j] || field.bact2[i][j]) {
                cnt_alive++;
                alive_in_line[i]++;
            }
        }
    }
    per_line = cnt_alive / cnt_workers;
    for (i = 0; i < field.n; i++) {
        sum += alive_in_line[i];
        if (sum >= per_line) {
            working_interval_r[cur] = i + 1;
            cur++;
            working_interval_l[cur] = i + 1;
            sum = 0;
        }
    }
    working_interval_r[cur] = field.n;
    for (i = cur + 1; i < cnt_workers; i++) {
        working_interval_l[i] = working_interval_r[i] = field.n;
    }

    free(alive_in_line);
    return SUCCESS;
}

void get_working_interval(int node_number, int *first_line, int *last_line) {
    *first_line = working_interval_l[node_number];
    *last_line = working_interval_r[node_number];
}

void prepare_storage(struct Storage *st, struct Field *field, int first_line, int last_line, int step) {
    int i, j;
    for (i = first_line; i < last_line; i++) {
        for (j = 0; j < field->m; j++) {
            if (field->bact1[i][j] != '0' || field->bact2[i][j] != '0') {
                add(st, i, j, step + 1);
            }
        }
    }
}

struct Storage st[2];
int first_line, last_line;

void* thread_worker(void *arg) {
    int step = *((int*)arg);
    int from = step % 2;
    int to = 1 - step;
    int dx[4] = { -1, 1, 0, 0 };
    int dy[4] = { 0, 0, 1, -1 };
    while (true) {
        int i;
        int x, y;
        get(&st[from], &x, &y);        
        if (x == -1) {
            break;
        }
        recalc_cell(step, x, y);
        for (i = 0; i < 4; i++) {
            int xx = (x + dx[i]) % field.n, yy = (y + dy[i]) % field.m;
            if (first_line <= xx && xx < last_line) {
                add(&st[to], xx, yy, step);
            }
        }
    }

    return NULL;
}

void run() {
    int field_num;
    int step = 0;
    int i;
    init_storage(&st[0], field.n, field.m);
    init_storage(&st[1], field.n, field.m);
    
    while (step < steps) {
        recv_array(working_interval_l, cnt_workers, 0);
        recv_array(working_interval_r, cnt_workers, 0);

        get_working_interval(node_number, &first_line, &last_line);

        copy_info(&working_field[0], &field);
        copy_info(&working_field[1], &field);
        get_empty_field(&working_field[0], field.n, field.m);
        get_empty_field(&working_field[1], field.n, field.m);
        recv_field(&working_field[0], first_line, last_line, 0);

        prepare_storage(&st[0], &working_field[0], first_line, last_line, step);

        for (; step < steps; step++) {
            if (step % TURN_LENGTH == 0 && step != 0) {
                break;
            }
            int next_line = last_line % field.n;
            int next_node = node_number + 1;
            if (next_node > cnt_workers)
                next_node = 1;
            int prev_line = (first_line - 1 + field.n) % field.n;
            int prev_node = node_number - 1;
            if (prev_node < 1)
                prev_node = cnt_workers;
            send_field(&working_field[step % 2], first_line, first_line + 1, prev_node);
            send_field(&working_field[step % 2], last_line - 1, last_line, next_node);
            recv_field(&working_field[step % 2], prev_line, prev_line + 1, prev_node);
            recv_field(&working_field[step % 2], next_line, next_line + 1, next_node);
            
            pthread_t threads[CNT_THREADS - 1];
            for (i = 0; i < CNT_THREADS - 1; i++) {
                pthread_create(&threads[i], NULL, thread_worker, &step);
            }
            thread_worker(&step);
            for (i = 0; i < CNT_THREADS - 1; i++) {
                pthread_join(threads[i], NULL);
            }
        }

        field_num = steps % 2;
        send_field(&working_field[field_num], first_line, last_line, 0);
    }

    free_storage(&st[0]);
    free_storage(&st[1]);
    free_workers();
}

