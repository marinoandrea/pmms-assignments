#include <time.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "compute.h"
#include "cuda_compute.h"

void do_compute(const struct parameters *p, struct results *r)
{
   cuda_do_compute(p, r);
}
