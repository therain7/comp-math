# Parallelizing the Gauss-Seidel method

## Introduction

Gauss-Seidel method can be used to numerically solve partial differential equations.
This work explores the effectiveness of the parallel implementation of the method presented in the _Гергель, В. П. (2010). Высокопроизводительные вычисления для многопроцессорных многоядерных систем_ in Chapter 11.

Authors claim that the parallel version is much more effective than sequential one producing results up to 5 times faster.
The goal of this work is to verify these claims as well as find the optimal parameters to run parallel version of algorithm with.

## Method implementation

The signatures of functions used to run algorithm are presented as follows:

```C
Net *init_net(uint32_t size, Fun u, Fun f);

uint32_t approximate_seq(Net *net, double eps);

int32_t approximate_prl(Net *net, uint32_t threads_num, uint32_t block_size, double eps);
```

`init_net` creates the square matrix of function values of size `size` filled with zeros except for the borders. The matrix (net) is then used to approximate function values in all points using the presented method.

`approximate_seq` runs the algorithm on given `net`. `eps` sets the acceptable relative error i.e. directly influences the amount of iterations algorithm should go through.

`approximate_prl` runs the parallel version of the algorithm. Computation is performed in several threads each computing its own block that's initial matrix (net) is divided into. The function accepts the size of such blocks as well as the amount of threads to run the algorithm on.

Both functions return the amount of iterations that were performed during the approximation using Gauss-Seidel method.

## Theses & goals

-   It's expected that parallel version runs faster than sequential one at large enough nets.
    The goal is to find the net size starting at which it makes sense to use the parallel version.

-   It's expected that the amount of threads directly influences the running time of the algorithm.
    The goal is to find the optimal number of threads to run algorithm on.

-   It's expected that block size directly influences the amount of parallelization algorithm allows for which in turn affects the overall running time.
    The goal is to find the optimal block size and study it's relation to net size.

## Environment & parameters

All tests were run on `Macbook Pro 14-inch M3 Pro 11-Core, 18GB memory` running `macOS Sonoma 14.4, clang 17.0.6`.
Compile options can be found in [CMakeLists.txt](CMakeLists.txt).

It's argued that the choice of functions to approximate is insignificant in the presented testing.
Input functions mainly affect the amount of iterations algorithm goes through.
However, amount of iterations doesn't change with the change of amount of threads and therefore doesn't effect speedup.

Each test was run 10 times. All results are presented as a mean of 10 runs and are given in a 95% confidence interval.

## Testing 1

First let's try to determine the depedendecy of running time on net size and amount of threads.
The test is implemented in [test.c](test.c) as `test_threads_num` function.
Block size is fixed at _32_ and eps is fixed at _0.05_.
Results of the test can be found in [test_threads_num.csv](results/test_threads_num.csv).

Following the results it's clear that on net size of _200_ parallel version is ineffective compared to sequential one.
However, starting at net size _400_ speedup can already be seen.
At net size _2000_ speedup can be as high as _250%_.
Such poor performance at net size _200_ can be explained by little running time overall.
It's possible that the overhead of parallelization is quite high compared to running time on net size of _200_.

To sum up, it doesn't make sense to use parallel version on small nets (e.g. _200_) but at _400_ and higher parallel algorithm shows significant speedup.

As to the amount of threads, testing shows that in the presented environment running algorithm on _4_ threads shows the best results.
Using _8+_ threads results in degraded performance compared to running on _4_ threads. This can be explained by small net sizes overall.
It's likely that _4_ threads can handle net sizes up to _2000_ quite well while engaging more threads results in high overhead.

## Testing 2

Second let's try to find optimal block size to use on different net sizes.
The test is implemented in [test.c](test.c) as `test_block_size` function.
Eps is fixed at _0.05_ and amount of threads is fixed at _4_.
Results of the test can be found in [test_block_size.csv](results/test_block_size.csv).

Following the results it's clear that block size cannot be too big relative to net size (bigger than _1/2_ of net)
as in such case there's not much room for parallelization and performance is poor.
Overall it can be observed that taking block size = _~1/8_ of net size is a good rule of thumb here.

## Summary

-   It makes sense to use parallel version of the algorithm starting at net size _400_.
-   In the presented environment and on given parameters it's optimal to run algorithm on _4_ threads.
-   A good rule of thumb is to take block size = _~1/8_ net size.
