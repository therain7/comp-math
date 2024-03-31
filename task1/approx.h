#ifndef APPROX_H
#define APPROX_H

#include <stdint.h>

typedef struct {
    uint32_t size;
    double h;

    double **u;
    double **f;
} Net;

typedef double (*Fun)(double, double);

Net *init_net(uint32_t size, Fun u, Fun f);

void free_net(Net *net);

uint32_t approximate_seq(Net *net, double eps);

int32_t approximate_prl(Net *net, uint32_t threads_num, uint32_t block_size, double eps);

#endif // !APPROX_H
