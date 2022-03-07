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
    int length;
} thread_args_t;

static pthread_attr_t attr;

void *compare(void *thread_args) {
    /* Receive parameters struct from parent thread */
    thread_args_t *args = (thread_args_t *) thread_args;
    int *values = args->values;
    int length = args->length;
    int index = args->index;

    pthread_t child_thread;
    int current_max = 0;

    /* How do we get a next value from the parent? */

    // while(next != END) {
    //     /* Compare incoming to stored number, store maximum */
    //     current_max = max(current_max, next);
        
    //     /* Forward other value to next thread somehow */

    //     /* Create a child thread to receive lower numbers */
    //     pthread_create(&child_thread, &attr, &compare, NULL);
    // }

    // /* Forward internally stored number (int max) */

    // /* Forward sorted values from parent thread until second END is received */
    // while(next != END) {
    //     /* Write incoming number from parent thread */
    // }

    // /* Wait for child thread to complete */
    // pthread_join(child_thread, NULL);

    /* Terminate thread */
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    long length = 1e4;
    int num_threads = 2;

    struct timespec before;
    struct timespec  after;

    thread_args_t thread_args;
    pthread_t child_thread;

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

    /* Initialize semaphore for access to values sequence */
    // if (sem_init(&sem, 0, 1) == -1) {
    //     perror("Semaphore initialization failed.");
    //     exit(EXIT_FAILURE);
    // }

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
    thread_args.length = length;
    thread_args.index = 0;

    /* The master thread is the generator, all others are comparators */
    pthread_create(&child_thread, &attr, &compare, &thread_args);

    /* Wait for comparator threads to complete */
    pthread_join(child_thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);
}
