#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include "input.h"

int SEQUENTIAL_CUTOFF = 100;

int compare(const void *a, const void *b)
{   
    return (* (int *)a) - (* (int *)b);
}

void merge(int *a, long begin, long mid, long end, int *b)
{
    long i = begin, j = mid;
    
    for(long k = begin; k < end; k++)
    {
        if(i < mid && (j >= end || a[i] <= a[j]))
        {
            b[k] = a[i];
            i++;
        }
        else
        {
            b[k] = a[j];
            j++;
        }
    }
}

inline void split(int *b, long begin, long end, int *a) 
{
    if(end - begin < SEQUENTIAL_CUTOFF) 
    {
        qsort(a + begin, end - begin, sizeof(int), compare);
        return;
    }
    
    long mid = (begin + end) / 2;
    
    #pragma omp task shared(a, b) firstprivate(begin, mid)
    split(a, begin, mid, b);

    #pragma omp task shared(a, b) firstprivate(mid, end)
    split(a, mid, end, b);

    #pragma omp taskwait

    merge(b, begin, mid, end, a);
}

void print_v(int *v, long l) {
    printf("\n");
    for(long i = 0; i < l; i++) 
    {
        if(i != 0 && (i % 10 == 0)) printf("\n");
        printf("%d ", v[i]);
    }
    printf("\n");
}

/* Sort vector v of l elements using mergesort */
void msort(int *v, long l) {
    int *v_copy = (int*)malloc(l*sizeof(int));
    memcpy(v_copy, v, l*sizeof(int));
    
    split(v_copy, 0, l, v);
    print_v(v, l);
}

int main(int argc, char **argv) 
{
    params cli_params = { 0 };
    if (parse_arguments(argc, argv, &cli_params) != 0)
    {
        return -1;
    }

    struct timespec before, after;

    /* Seed such that we can always reproduce the same random vector */
    srand(cli_params.seed);

    /* Allocate vector. */
    int *vector = (int *)calloc(cli_params.length, sizeof(int));

    if(vector == NULL) 
    {
        fprintf(stderr, "Malloc failed...\n");
        return -1;
    }

    /* Fill vector. */
    switch(cli_params.order)
    {
        case ASCENDING:
            for(long i = 0; i < cli_params.length; i++) vector[i] = (int)i;
            break;
        case DESCENDING:
            for(long i = 0; i < cli_params.length; i++) vector[i] = (int)(cli_params.length - i);
            break;
        case RANDOM:
            for(long i = 0; i < cli_params.length; i++) vector[i] = rand();
            break;
    }

    if(cli_params.debug) 
    {
        print_v(vector, cli_params.length);
    }

    clock_gettime(CLOCK_MONOTONIC, &before);

    #pragma omp parallel
    {
        #pragma omp single
        msort(vector, cli_params.length);
    }

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("parallel mergesort took: % .6e seconds \n", time);

    if(cli_params.debug) 
    {
        print_v(vector, cli_params.length);
    }

    return 0;
}

