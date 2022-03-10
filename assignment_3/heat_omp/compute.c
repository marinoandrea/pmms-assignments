#include <time.h>
#include <math.h>
#include <stdlib.h>

#include <pthread.h>

#include "compute.h"

#include "ref1.c"

void do_compute(const struct parameters* p, struct results *r)
{
    #include "ref2.c"
}
