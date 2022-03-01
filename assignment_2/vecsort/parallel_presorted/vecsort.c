#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <omp.h>
#include <assert.h>
#include <string.h>

/* Ordering of the vector */
typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;

typedef struct
{
    int *array;
    int length;
} array_t;

int debug = 0;

void Merge(int *a, long begin, long mid, long end, int *b)
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

void Split(int *b, long begin, long end, int *a)
{
    if(end - begin < 2)
        return;
    
    long mid = (begin + end) / 2;
    Split(a, begin, mid, b);
    Split(a, mid, end, b);

    Merge(b, begin, mid, end, a);
}


/* Sort vector v of l elements using mergesort */
void msort(int *v, long l) {
    int *v_copy = (int*)malloc(l*sizeof(int));
    memcpy(v_copy, v, l*sizeof(int));
    
    Split(v_copy, 0, l, v);
}

//TODO: Just Do It. Don't let your dreams be dreams.
void vecsort(array_t *zipped_vectors, long length_outer, int num_threads) {
    printf("start vecsort\n");
    long chunk_size = 1; // length_outer / (long) num_threads;
    #pragma omp parallel for schedule(dynamic, chunk_size) num_threads(num_threads) if(length_outer > 400)
    for (long i = 0; i < length_outer; ++i) {
        msort(zipped_vectors[i].array, zipped_vectors[i].length);
    }
}

void print_v(int **vector_vectors, int *vector_lengths, long length_outer) {
    printf("\n");
    for(long i = 0; i < length_outer; i++) {
        for (int j = 0; j < vector_lengths[i]; j++){
            if(j != 0 && (j % 10 == 0)) {
                printf("\n");
            }
            printf("%d ", vector_vectors[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int cmp (const void * a, const void * b) {
    return ((array_t *) a)->length - ((array_t *) b)->length;
}

int main(int argc, char **argv) {

    int c;
    int seed = 42;
    long length_outer = 1e4;
    int num_threads = 1;
    Order order = ASCENDING;
    int length_inner_min = 100;
    int length_inner_max = 1000;

    int **vector_vectors;
    int *vector_lengths;

    struct timespec before, after;


    /* Read command-line options. */
    while ((c = getopt(argc, argv, "adrgn:x:l:p:s:")) != -1) {
        switch (c) {
            case 'a':
                order = ASCENDING;
                break;
            case 'd':
                order = DESCENDING;
                break;
            case 'r':
                order = RANDOM;
                break;
            case 'l':
                length_outer = atol(optarg);
                break;
            case 'n':
                length_inner_min = atoi(optarg);
                break;
            case 'x':
                length_inner_max = atoi(optarg);
                break;
            case 'g':
                debug = 1;
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'p':
                num_threads = atoi(optarg);
                break;
            case '?':
                if (optopt == 'l' || optopt == 's') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                }
                return -1;
            default:
                return -1;
        }
    }

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    /* Allocate vector. */
    vector_vectors = (int **) malloc(length_outer * sizeof(int*));
    vector_lengths = (int *) malloc(length_outer * sizeof(int));
    if (vector_vectors == NULL || vector_lengths == NULL) {
        fprintf(stderr, "Malloc failed...\n");
        return -1;
    }

    assert(length_inner_min < length_inner_max);

    array_t *vectors_tuples = (array_t *)malloc(length_outer * sizeof(array_t));

    printf("after tuples init\n");

    /* Determine length of inner vectors and fill them. */
    for (long i = 0; i < length_outer; i++) {
        int length_inner = (rand() % (length_inner_max + 1 - length_inner_min)) + length_inner_min ; //random number inclusive between min and max
        array_t al = { 0 };
        al.array  = (int *) malloc(length_inner * sizeof(int));
        al.length = length_inner;
        vectors_tuples[i] = al;

        /* Allocate and fill inner vector. */
        switch (order) {
            case ASCENDING:
                for (long j = 0; j < length_inner; j++) {
                    vectors_tuples[i].array[j] = (int) j;
                }
                break;
            case DESCENDING:
                for (long j = 0; j < length_inner; j++) {
                    vectors_tuples[i].array[j] = (int) (length_inner - j);
                }
                break;
            case RANDOM:
                for (long j = 0; j < length_inner; j++) {
                    vectors_tuples[i].array[j] = rand();
                }
                break;
        }
    }

    if(debug) {
        print_v(vector_vectors, vector_lengths, length_outer);
    }

    clock_gettime(CLOCK_MONOTONIC, &before);

    printf("before qsort\n");
    /* Sort */
    qsort(vectors_tuples, length_outer, sizeof(array_t), cmp);

    vecsort(vectors_tuples, length_outer, num_threads);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Vecsort took: % .6e \n", time);

    if(debug) {
        print_v(vector_vectors, vector_lengths, length_outer);
    }

    free(vector_vectors);
    free(vector_lengths);
    free(vectors_tuples);

    return 0;
}

