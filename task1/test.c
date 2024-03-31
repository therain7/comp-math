#include "approx.h"
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>

#define array_len(arr) (sizeof(arr) / sizeof(arr[0]))

Fun fun;
static double only_borders_fun(double x, double y) {
    if (x == 0 || y == 0 || x == 1 || y == 1) {
        return fun(x, y);
    }
    return 0;
}

static double fun1(double x, double y) { return 1000 * pow(x, 3) + 2000 * pow(y, 3); }
static double fun1_d(double x, double y) { return 6000 * x + 12000 * y; }

static void test_threads_num(double eps, uint32_t block_size, const uint32_t *threads,
                             uint32_t threads_len, const uint32_t *net_sizes,
                             uint32_t net_sizes_len, const char *results_filename) {
    FILE *out = fopen(results_filename, "w+");
    fprintf(out, "threads,net size,iterations,time\n");

    for (uint32_t i = 0; i < threads_len; i++) {
        for (uint32_t j = 0; j < net_sizes_len; j++) {
            uint32_t threads_num = threads[i];
            uint32_t net_size = net_sizes[j];

            fun = fun1;
            Net *net = init_net(net_size, only_borders_fun, fun1_d);

            uint32_t iters;
            double start_time, end_time;
            if (threads_num == 1) {
                start_time = omp_get_wtime();
                iters = approximate_seq(net, eps);
                end_time = omp_get_wtime();
            } else {
                start_time = omp_get_wtime();
                iters = approximate_prl(net, threads_num, block_size, eps);
                end_time = omp_get_wtime();
            }
            free_net(net);

            fprintf(out, "%u,%u,%u,%f\n", threads_num, net_size, iters, end_time - start_time);
            printf("%u,%u,%u,%f\n", threads_num, net_size, iters, end_time - start_time);
        }
    }

    fclose(out);
}

static void test_block_size(double eps, const uint32_t *block_sizes, uint32_t block_sizes_len,
                            const uint32_t *net_sizes, uint32_t net_sizes_len,
                            const char *results_filename) {
    FILE *out = fopen(results_filename, "w+");
    fprintf(out, "block size,net size,time");

    for (uint32_t i = 0; i < block_sizes_len; i++) {
        for (uint32_t j = 0; j < net_sizes_len; j++) {
            uint32_t block_size = block_sizes[i];
            uint32_t net_size = net_sizes[j];

            fun = fun1;
            Net *net = init_net(net_size, only_borders_fun, fun1_d);

            double start_time = omp_get_wtime();
            approximate_prl(net, 4, block_size, eps);
            double end_time = omp_get_wtime();
            free_net(net);

            fprintf(out, "%u,%u,%f\n", block_size, net_size, end_time - start_time);
            printf("%u,%u,%f\n", block_size, net_size, end_time - start_time);
        }
    }
    fclose(out);
}
