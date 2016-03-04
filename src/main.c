#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <mpi.h>
#include <sys/time.h>

#include "utilities.h"
#include "errors.h"
#include "worker.h"
#include "field.h"

int main(int argc, char *argv[]) {
    if (argc == 3) {
        puts("Error: can't read from file currently!");
        return -1;
        if (parse_int(argv[1], &cnt_workers) != SUCCESS ||
                parse_int(argv[2], &steps) != SUCCESS || 
                cnt_workers >= MAX_WORKERS) {
            puts("Invalid argument list: must be 'cnt_workers cnt_steps' or 'n m cnt_workers cnt_steps'");
            return INVALID_ARGUMENTS_LIST;
        }
        if (get_field_from_file(&field) != SUCCESS) {
            puts("Can't read from this file");
            return FILE_READING_FAIL;
        }
    } else if (argc == 5) {
        int n, m;
        if (parse_int(argv[1], &n) != SUCCESS ||
                parse_int(argv[2], &m) != SUCCESS || 
                parse_int(argv[3], &cnt_workers) != SUCCESS ||
                parse_int(argv[4], &steps) != SUCCESS || 
                cnt_workers >= MAX_WORKERS) {
            puts("Invalid argument list: must be 'filename cnt_workers cnt_steps' or 'n m cnt_workers cnt_steps'");
            return INVALID_ARGUMENTS_LIST;
        }
        if (get_random_field(&field, n, m) != SUCCESS) {
            puts("Can't create field");
            return MALLOC_FAIL;
        }
    } else {
        puts("Invalid argument list: must be 'filename cnt_workers cnt_steps' or 'n m cnt_workers cnt_steps'");
        return INVALID_ARGUMENTS_LIST;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &node_number);

    if (node_number == 0) {
        int i;
        struct timeval buf_time;
        long long start_time, end_time;
        int cnt_turns = 0;

        gettimeofday(&buf_time, NULL);
        start_time = buf_time.tv_usec + buf_time.tv_sec * 1000000;

        while (cnt_turns < steps) {
            init_working_intervals();

            for (i = 0; i <= cnt_workers; i++) {
                send_array(working_interval_l, cnt_workers, i);
                send_array(working_interval_r, cnt_workers, i);
            }

            for (i = 1; i <= cnt_workers; i++) {
                int first_line, last_line;
                get_working_interval(i, &first_line, &last_line);
                send_field(&field, first_line, last_line, i);
            }

            for (i = 1; i <= cnt_workers; i++) {
                int first_line, last_line;
                get_working_interval(i, &first_line, &last_line);
                recv_field(&field, first_line, last_line, i);
            }
            cnt_turns += TURN_LENGTH;
        }
        gettimeofday(&buf_time, NULL);
        end_time = buf_time.tv_usec + buf_time.tv_sec * 1000000;
        printf("Done in %lld.%lld sec\n", 
                (end_time - start_time) / 1000000, (end_time - start_time) % 1000000);
        /*print_field(&field);*/
    } else if (node_number <= cnt_workers) {
        run();
    }
    free_field(&field);
    MPI_Finalize();
    return SUCCESS;
}
