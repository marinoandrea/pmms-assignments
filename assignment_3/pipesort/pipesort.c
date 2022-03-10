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
typedef struct thread_comp_args_t {
    int *values;
    int *dirty_flags;
    int *initialized_flags;
    pthread_mutex_t *locks;
    int value_in;
    int *value_out;
    size_t index;
} thread_comp_args_t;

typedef struct thread_gen_args_t {
    int *values;
    size_t length;
} thread_gen_args_t;

/* Pthread attributes are shared between all threads */
static pthread_attr_t attr;

void *compare(void *arg) 
{    
    thread_comp_args_t args = *(thread_comp_args_t *)arg;

    int value_state;
    int value_pass;
    int count_iter = 0;
    int count_end = 0;
    int count_end_pass = 0; 

    while (count_end < 2)
    {
        if (!args.initialized_flags[args.index]) continue;

        // Read loop
        for (;;)
        {
            pthread_mutex_lock(&args.locks[args.index]);
            if (!args.dirty_flags[args.index])
            {
                pthread_mutex_unlock(&args.locks[args.index]);
                continue;
            }

            if (args.value_in == END && count_end == 0)
            {
                count_end++;
                value_pass = value_state;
                args.dirty_flags[args.index] = 0;
                pthread_mutex_unlock(&args.locks[args.index]);   
                break;
            }
    
            if (count_end == 1 || count_iter == 0)
            {
                value_pass = args.value_in;
                args.dirty_flags[args.index] = 0;
                pthread_mutex_unlock(&args.locks[args.index]);   
                break;
            }

            if (args.value_in > value_state)
            {
                value_pass = value_state;
                value_state = args.value_in;
            } 
            else 
            {
                value_pass = args.value_in;
            }

            args.dirty_flags[args.index] = 0;

            pthread_mutex_unlock(&args.locks[args.index]);   
            break;
        }

        if (count_end_pass == 0 && count_end == 1)
        {
            for (;;)
            {
                pthread_mutex_lock(&args.locks[args.index + 1]);
                if (args.dirty_flags[args.index + 1])
                {
                    pthread_mutex_unlock(&args.locks[args.index + 1]);
                    continue;
                }

                *args.value_out = END;

                pthread_mutex_unlock(&args.locks[args.index + 1]);   
                break;
            }

            count_end_pass++;
        }

        // Write loop
        for (;;)
        {
            if (count_iter == 0) break;

            pthread_mutex_lock(&args.locks[args.index + 1]);
            if (args.dirty_flags[args.index + 1])
            {
                pthread_mutex_unlock(&args.locks[args.index + 1]);
                continue;
            }

            *args.value_out = value_pass;
            args.dirty_flags[args.index + 1] = 1;
            
            if (!args.initialized_flags[args.index + 1])
            {
                args.initialized_flags[args.index + 1] = 1;
            }

            pthread_mutex_unlock(&args.locks[args.index + 1]);   
            break;    
        }

        count_iter++;
    }

    // FIXME: only write if we do not have END as value
    args.values[args.index] = value_state;
}


void *generate(void *arg)
{
    thread_gen_args_t gen_args = *(thread_gen_args_t *)arg;
    gen_args.length += 2;
        
    int *dirty_flags        = (int *)malloc(gen_args.length * sizeof(int));
    int *initialized_flags  = (int *)malloc(gen_args.length * sizeof(int));

    pthread_mutex_t *locks = (pthread_mutex_t *)malloc(gen_args.length * sizeof(pthread_mutex_t));
 
    pthread_t *thread_ids = (pthread_t *)malloc(gen_args.length * sizeof(pthread_t));
 
    thread_comp_args_t *thread_args = (thread_comp_args_t *)malloc(gen_args.length * sizeof(thread_comp_args_t));

    // initialize the thread locks
    for (int i = 0; i < gen_args.length; i++)  
    {
        dirty_flags[i] = 0;
        initialized_flags[i] = 0;
    
        pthread_mutex_init(&locks[i], NULL);
        thread_args[i].values = gen_args.values;
        thread_args[i].dirty_flags = dirty_flags;
        thread_args[i].locks = locks;
        thread_args[i].index = i;

        if (i < gen_args.length - 1)
            thread_args[i].value_out = &thread_args[i + 1].value_in;
        else
            thread_args[i].value_out = NULL;
    }

    for (int i = 0; i < gen_args.length; i++)
    {
        pthread_mutex_lock(&locks[0]);

        thread_args[0].value_in = i < gen_args.length - 2 ? rand() : END;
        
        if (i == 0) initialized_flags[0] = 1;

        pthread_mutex_unlock(&locks[0]);

        pthread_create(&thread_ids[i], &attr, &compare, &thread_args[i]);
    }

    for (int i = 0; i < gen_args.length; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

}

int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    long length = 1e4;

    struct timespec before;
    struct timespec  after;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "l:s:p:")) != -1) {
        switch(c) {
            case 'l':
                length = atol(optarg);
                break;
            case 's':
                seed = atoi(optarg);
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
    // pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + sysconf(_SC_PAGESIZE));

    int *out_values = (int *)malloc(length * sizeof(int));

    pthread_t generator_thread;
    thread_gen_args_t gen_args = { 0 };
    gen_args.length = length;
    gen_args.values = out_values;

    /* The master thread is the generator, all others are comparators */
    pthread_create(&generator_thread, &attr, &generate, &gen_args);

    /* Wait for comparator threads to complete */
    pthread_join(generator_thread, NULL);

#ifdef PRINT_ARRAY
    for (int i = 0; i < length; i++) printf("%i ", out_values[i]);
    printf("\n");
#endif

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);
}
