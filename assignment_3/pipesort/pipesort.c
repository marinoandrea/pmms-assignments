#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>

#define END -1

/* Arguments for a comparator thread */
struct thread_args_t {
    int *values;
    int index;
    int length;
};

pthread_attr_t attr;

void *compare(void *thread_args) {
    /* Receive parameters struct from parent thread */
    struct thread_args_t *args = (struct thread_args_t *) thread_args;
    int *values = args->values;
    int length = args->length;
    int index = args->index;

    pthread_t child_thread;
    int max = 0;

    return EXIT_SUCCESS;

    int next = values[index];

    /* How do we get a next value from the parent? */

    while(next != END) {
        /* Compare incoming to stored number, store maximum */

        /* Forward other value to next thread */

        /* Create a child thread to receive lower numbers */
        pthread_create(&child_thread, &attr, &compare, NULL);
    }

    /* Forward internally stored number (int max) */

    /* Forward sorted values from parent thread until second END is received */
    while(next != END) {
        /* Write incoming number from parent thread */
    }

    /* Wait for child thread to complete */
    pthread_join(child_thread, NULL);

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

    struct thread_args_t thread_args;
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

    /* Find a thread-safe way to keep track of total number of threads */
    /* Maybe a semaphore? */

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    clock_gettime(CLOCK_MONOTONIC, &before);

    /* Use only kernel threads */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    int values[length];

    for (int i = 0; i < length; ++i) {
        values[i] = rand();
        printf("%d\n", values[i]);
    }

    thread_args.values = values;
    thread_args.length = length;
    thread_args.index = 0;

    /* The master thread is the generator thread, all others are comparators */
    pthread_create(&child_thread, &attr, &compare, &thread_args);

    pthread_join(child_thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);
}
