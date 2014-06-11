#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

double random_double() {
    return ((double)abs(rand()) / RAND_MAX);
}

int compute(int tries) {
    int hits = 0;
    double x, y;
    for(int i = 0; i < tries; ++i) {
        x = random_double();
        y = random_double();
        if (x*x + y*y <= 1) {
            ++hits;
        }
    }
    return hits;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Provide number of tries");
        return 0;
    }
    srand(time(0));

    int tries = atoi(argv[1]);
    int hits = compute(tries);
    double pi = 4.0 * hits / tries;
    printf("PI = %f\n", (float)pi);

    return 0;
}

