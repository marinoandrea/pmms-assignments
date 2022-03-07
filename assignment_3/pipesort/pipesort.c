#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

/* Output from rand() is >= 0, so guaranteed to be different from END. */
#define END -1

/* Function arguments for the comparator threads */
typedef struct thread_args_t {
    int *values;
    int index;
} thread_args_t;

/* Pthread attributes are shared between all threads */
static pthread_attr_t attr;

void *compare(void *args) {
    thread_args_t *thread_args = (thread_args_t *) args;
    int *values = thread_args->values;
    int max = thread_args->index;

    pthread_t child_thread;
    thread_args_t child_args;

    int current = max + 1;

    /* Compare incoming to stored number, store maximum */
    if (values[current] > values[max]) {
        child_args.index = max;
        max = current;
    } else {
        child_args.index = current;
    }

    child_args.values = values;

    /* Create a child thread to receive remaining numbers */
    pthread_create(&child_thread, &attr, &compare, &child_args);

    /* Compare remaining numbers and keep track of max value */
    while(values[current] != END) {

    }

    /* Forward internally stored number (int max) */

    /* Forward sorted values from parent thread until second END is received */
    while(values[current] != END) {

    }

    /* Wait for child thread to complete */
    pthread_join(child_thread, NULL);

    /* Terminate thread */
    return 0;
}

int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    long length = 1e4;
    int num_threads = 2;

    struct timespec before;
    struct timespec  after;

    thread_args_t thread_args;
    pthread_t first_comparator;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "l:s:p:")) != -1) {
        switch(c) {
            case 'l':
                length = atol(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'p':
                num_threads = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    clock_gettime(CLOCK_MONOTONIC, &before);

    pthread_attr_init(&attr);

    /* Use only kernel threads (the default and only option on Linux) */    
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* Set a small stack size to support a larger total number of threads,
    it should be a multiple of _SC_PAGESIZE and at least PTHREAD_STACK_MIN */
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + sysconf(_SC_PAGESIZE));

    /* Generate n random integers between 0 and RAND_MAX */
    int values[length + 2];
    for (int i = 0; i < length; ++i) {
        values[i] = rand();
        printf("%d\n", values[i]);
    }

    /* Terminate the values sequence by two END symbols */
    values[length] = END;
    values[length + 1] = END;

    /* Initialize comparator thread arguments */
    thread_args.values = values;
    thread_args.index = 0;

    /* The master thread is the generator, all others are comparators */
    pthread_create(&first_comparator, &attr, &compare, &thread_args);

    /* Wait for comparator threads to complete */
    pthread_join(first_comparator, NULL);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);
}
