#include "input.h"
#include "output.h"

#ifndef CUDA_COMPUTE_H
#define CUDA_COMPUTE_H

#ifdef __cplusplus
extern "C" 
#endif 

void cuda_do_compute(const struct parameters *p, struct results *r);

#endif
