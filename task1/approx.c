#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "approx.h"

static inline double min(double a, double b) { return (a < b) ? a : b; }
static inline double max(double a, double b) { return (a > b) ? a : b; }

static void print_2d_double_arr(double **arr, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        for (uint32_t j = 0; j < size; j++) {
            printf("%.2f\t", arr[i][j]);
        }
        printf("\n");
    }
}

static void print_2d_points_arr(Point **arr, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        for (uint32_t j = 0; j < size; j++) {
            printf("%.2f %.2f\t", arr[i][j].x, arr[i][j].y);
        }
        printf("\n");
    }
}

void print_net(const Net *net) {
    printf("size: %u\n", net->size);

    printf("\npoints:\n");
    print_2d_points_arr(net->points, net->size);

    printf("\nu(x, y):\n");
    print_2d_double_arr(net->u, net->size);

    printf("\nf(x, y):\n");
    print_2d_double_arr(net->f, net->size);
}

static void free_2d_arr(void **arr, uint32_t arr_size) {
    for (uint32_t i = 0; i < arr_size; i++) {
        free(arr[i]);
    }
    free(arr);
}

Net *init_net(uint32_t size, Fun u, Fun f) {
    double h = 1.0 / (size - 1);

    Point **points = malloc(sizeof(Point) * size);
    double **u_arr = malloc(sizeof(double) * size);
    double **f_arr = malloc(sizeof(double) * size);
    if (points == NULL || u_arr == NULL || f_arr == NULL) {
        free(points);
        free(u_arr);
        free(f_arr);
        return NULL;
    }

    for (uint32_t i = 0; i < size; i++) {
        points[i] = malloc(sizeof(Point) * size);
        u_arr[i] = malloc(sizeof(double) * size);
        f_arr[i] = malloc(sizeof(double) * size);
        if (points[i] == NULL || u_arr[i] == NULL || f_arr[i] == NULL) {
            free_2d_arr((void **)points, i + 1);
            free_2d_arr((void **)u_arr, i + 1);
            free_2d_arr((void **)f_arr, i + 1);
            return NULL;
        }

        for (uint32_t j = 0; j < size; j++) {
            Point point = {.x = i * h, .y = j * h};
            points[i][j] = point;
            u_arr[i][j] = u(point.x, point.y);
            f_arr[i][j] = f(point.x, point.y);
        }
    }

    Net *net = malloc(sizeof(Net));
    if (net == NULL) {
        free_2d_arr((void **)points, size);
        free_2d_arr((void **)u_arr, size);
        free_2d_arr((void **)f_arr, size);
        return NULL;
    }

    net->size = size;
    net->h = h;
    net->points = points;
    net->u = u_arr;
    net->f = f_arr;
    return net;
}

void free_net(Net *net) {
    free_2d_arr((void **)net->points, net->size);
    free_2d_arr((void **)net->u, net->size);
    free_2d_arr((void **)net->f, net->size);
    free(net);
}

static inline double approx_iter(Net *net, uint32_t i, uint32_t j) {
    return 0.25 * (net->u[i - 1][j] + net->u[i + 1][j] + net->u[i][j - 1] + net->u[i][j + 1] -
                   net->h * net->h * net->f[i][j]);
}

uint32_t approximate_seq(Net *net, double eps) {
    uint32_t iters = 0;
    double dmax = 0;

    do {
        iters++;
        dmax = 0;

        for (uint32_t i = 1; i < net->size - 1; i++) {
            for (uint32_t j = 1; j < net->size - 1; j++) {
                double temp = net->u[i][j];
                net->u[i][j] = approx_iter(net, i, j);

                double dm = fabs(temp - net->u[i][j]);
                dmax = max(dmax, dm);
            }
        }
    } while (dmax > eps);

    return iters;
}

static double approximate_block(Net *net, uint32_t block_size, uint32_t i, uint32_t j) {
    uint32_t i0 = 1 + i * block_size;
    uint32_t j0 = 1 + j * block_size;

    uint32_t i_max = min(i0 + block_size, net->size - 1);
    uint32_t j_max = min(j0 + block_size, net->size - 1);

    double dmax = 0;
    for (i = i0; i < i_max; i++) {
        for (j = j0; j < j_max; j++) {
            double temp = net->u[i][j];
            net->u[i][j] = approx_iter(net, i, j);

            double dm = fabs(temp - net->u[i][j]);
            dmax = max(dmax, dm);
        }
    }
    return dmax;
}

int32_t approximate_prl(Net *net, uint32_t block_size, double eps) {
    uint32_t iters = 0;

    uint32_t size_wo_border = net->size - 2;
    uint32_t blocks_amount = size_wo_border / block_size;

    if (blocks_amount * block_size != size_wo_border) {
        blocks_amount++;
    }

    double *dm = calloc(blocks_amount, sizeof(double));
    if (dm == NULL) {
        return -1;
    }

    double dmax = 0;
    do {
        iters++;
        dmax = 0;

        uint32_t i, j;
        double d;

        for (uint32_t nx = 0; nx < blocks_amount; nx++) {
            dm[nx] = 0;

#pragma omp parallel for shared(net, nx, dm) private(i, j, d)
            for (i = 0; i < nx + 1; i++) {
                j = nx - i;

                d = approximate_block(net, block_size, i, j);
                dm[i] = max(dm[i], d);
            }
        }

        for (int32_t nx = blocks_amount - 2; nx >= 0; nx--) {
#pragma omp parallel for shared(net, nx, dm) private(i, j, d)
            for (i = blocks_amount - nx - 1; i < blocks_amount; i++) {
                j = 2 * blocks_amount - nx - i - 2;

                d = approximate_block(net, block_size, i, j);
                dm[i] = max(dm[i], d);
            }
        }

        for (i = 0; i < blocks_amount; i++) {
            dmax = max(dmax, dm[i]);
        }
    } while (dmax > eps);

    free(dm);
    return iters;
}
