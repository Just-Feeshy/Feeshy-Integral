#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <benchmark/benchmark.h>

const int throws = 1000000;

double func(double x) {
    return 1 / sqrt(log(x) + 1);
}

void feeshy() {
    int sigma = !(throws & 1);
    double f = 0.0;
    double h = 5.0 / throws;
    double x_sigma = 1.0;
    double y_sigma = 6.0;

    for(int i=0; i<=throws>>1; i++) {
        f += 3 * (func(x_sigma) + func(y_sigma));
        x_sigma += h;
        y_sigma -= h;
    }

    double sum = (h / 3.0) * f;
    printf("%lf\n", sum);
}

void monte_carlo() {
    srand(0);

    double x_i = 0.0;
    double f = 0.0;

    for (int i=0; i<throws; i++) {
        x_i = (rand() / (double)RAND_MAX * 5) + 1;
        f += func(x_i);
    }

    double sum = (5.0 / throws) * f;
    printf("%lf\n", sum);
}

void feeshy_test(benchmark::State& state) {
    for (auto _ : state) {
        feeshy();
    }
}
BENCHMARK(feeshy_test);

void monte_carlo_test(benchmark::State& state) {
    for (auto _ : state) {
        monte_carlo();
    }
}
BENCHMARK(monte_carlo_test);

BENCHMARK_MAIN();
