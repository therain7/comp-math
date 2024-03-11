#include "approx.h"
#include <assert.h>
#include <math.h>
#include <omp.h>

double calculate_mean_error(Net *net, Fun f) {
    double sum = 0;

    for (uint32_t i = 0; i < net->size; i++) {
        for (uint32_t j = 0; j < net->size; j++) {
            Point point = net->points[i][j];
            sum += fabs(net->u[i][j] - f(point.x, point.y));
        }
    }

    return sum / (net->size * net->size);
}

void assert_net_eq(Net *net1, Net *net2) {
    assert(net1->size == net2->size);
    assert(net1->h == net2->h);

    for (uint32_t i = 0; i < net1->size; i++) {
        for (uint32_t j = 0; j < net1->size; j++) {
            Point p1 = net1->points[i][j];
            Point p2 = net2->points[i][j];

            assert(p1.x == p2.x && p1.y == p2.y);
            assert(net1->u[i][j] == net2->u[i][j]);
            assert(net1->f[i][j] == net2->f[i][j]);
        }
    }
}

Fun fun;
double only_borders_fun(double x, double y) {
    if (x == 0 || y == 0 || x == 1 || y == 1) {
        return fun(x, y);
    }
    return 0;
}
