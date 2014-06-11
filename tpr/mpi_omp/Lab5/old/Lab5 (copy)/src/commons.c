#include "commons.h"

#include <stdlib.h>

inline double random_double() {
    //return ((double) abs(rand())) / RAND_MAX;
    return rand()%2+1;
}
