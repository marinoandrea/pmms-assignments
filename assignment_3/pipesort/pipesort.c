#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#define PRINT_ARRAY
/* Output from rand() is >= 0, so guaranteed to be different from END. */
#define END -1

/* Function arguments for the comparator threads */
typedef struct thread_comp_args_t {
    int *states;
    int *dirty_flags;
    int value_in;
    int *value_out;
    pthread_mutex_t *locks;
    size_t index;
} thread_comp_args_t;

typedef struct thread_gen_args_t {
    int *values;
    size_t length;
} thread_gen_args_t;

typedef enum 
{
    INIT,
    READ_0,
    READ_1,
    READ_2,
    WRITE_1,
    WRITE_2,
    WRITE_ONCE,
    WRITE_END,
    COMPLETE
} comparator_state;

/* Pthread attributes are shared between all threads */
static pthread_attr_t attr;

void lock_on_condition(pthread_mutex_t *lock, int *condition, int condition_false)
{
    for (;;)
    {
        pthread_mutex_lock(lock);
        if (condition_false ? *condition : !*condition)
        {
            pthread_mutex_unlock(lock);
            continue;
        }
        break;
    }
}

void *compare(void *arg) 
{    
    thread_comp_args_t args = *(thread_comp_args_t *)arg;

    int value_hold;
    int value_pass;

    while (args.states[args.index] != COMPLETE)
    {        
        // printf("thread: %i, state: %i, value_hold: %i\n", args.index, args.states[args.index], value_hold);

        switch (args.states[args.index])
        {
        case INIT:
            continue;
        
        case READ_0:
        {
            lock_on_condition(&args.locks[args.index], &args.dirty_flags[args.index], 0);

            value_hold = args.value_in;
            args.dirty_flags[args.index] = 0;
            args.states[args.index] = READ_1;

            pthread_mutex_unlock(&args.locks[args.index]);   
            continue;
        }

        case READ_1:
        {
            lock_on_condition(&args.locks[args.index], &args.dirty_flags[args.index], 1);

            if (args.value_in == END)
            {
                args.states[args.index] = WRITE_ONCE;
            } 
            else
            {
                if (args.value_in > value_hold)
                {
                    value_pass = value_hold;
                    value_hold = args.value_in;
                } 
                else 
                {
                    value_pass = args.value_in;
                }

                args.dirty_flags[args.index] = 0;
                args.states[args.index] = WRITE_1;
            }

            pthread_mutex_unlock(&args.locks[args.index]);   
            continue;
        }

        case WRITE_ONCE:
        case WRITE_END:
        {
            lock_on_condition(&args.locks[args.index + 1], &args.dirty_flags[args.index + 1], 0);

            *args.value_out = args.states[args.index] == WRITE_ONCE ? value_hold : END;
            args.dirty_flags[args.index + 1] = 1;
            args.states[args.index] = args.states[args.index] == WRITE_ONCE ? WRITE_END : READ_2;

            pthread_mutex_unlock(&args.locks[args.index + 1]);
            continue;
        }

        case READ_2:
        {
            lock_on_condition(&args.locks[args.index], &args.dirty_flags[args.index], 1);

            if (args.value_in == END)
            {
                args.states[args.index] = COMPLETE;
            }
            else
            {
                value_pass = args.value_in;
                args.dirty_flags[args.index] = 0;
                args.states[args.index] = WRITE_2;
            }

            pthread_mutex_unlock(&args.locks[args.index]);
            continue;
        }

        case WRITE_1:
        case WRITE_2:
        {
            lock_on_condition(&args.locks[args.index + 1], &args.dirty_flags[args.index + 1], 0);

            *args.value_out = value_pass;
            args.dirty_flags[args.index + 1] = 1;
            
            args.states[args.index] = args.states[args.index] == WRITE_1 ? READ_1 : READ_2;
            
            if (args.states[args.index + 1] == INIT)
            {
                args.states[args.index + 1] = READ_0;
            }

            pthread_mutex_unlock(&args.locks[args.index + 1]);
            continue;
        }

        default:
            continue;
        }        
    }
}


void *generate(void *arg)
{
    thread_gen_args_t gen_args = *(thread_gen_args_t *)arg;

    int output_value;
    
    const size_t GENERATOR_IDX = gen_args.length; 

    // the last flag is for the generator thread output queue
    int *dirty_flags = (int *)malloc(sizeof(int) * (gen_args.length + 1));
    // the last lock is for the generator thread       
    pthread_mutex_t *locks = (pthread_mutex_t *)malloc((gen_args.length + 1) * sizeof(pthread_mutex_t));    
    // initialize generator thread synchronization tools
    dirty_flags[GENERATOR_IDX] = 0;
    pthread_mutex_init(&locks[GENERATOR_IDX], NULL);

    // comparator threads data structures
    comparator_state *states = (comparator_state *)malloc(sizeof(comparator_state) * gen_args.length);
    pthread_t *thread_ids = (pthread_t *)malloc(gen_args.length * sizeof(pthread_t));
    thread_comp_args_t *thread_args = (thread_comp_args_t *)malloc(gen_args.length * sizeof(thread_comp_args_t));

    // initialize the thread locks
    for (int i = 0; i < gen_args.length; i++)  
    {
        states[i] = INIT;
    
        pthread_mutex_init(&locks[i], NULL);
        thread_args[i].states = states;
        thread_args[i].locks = locks;
        thread_args[i].dirty_flags = dirty_flags;
        thread_args[i].index = i;
        thread_args[i].value_out = (i < gen_args.length - 1) ? &thread_args[i + 1].value_in : &output_value;
    }

    for (int i = 0; i < gen_args.length + 2; i++)
    {
        pthread_mutex_lock(&locks[0]);

        thread_args[0].value_in = i < gen_args.length - 2 ? rand() : END;
    
        dirty_flags[0] = 1;
        
        if (i == 0) states[0] = READ_0;

        pthread_mutex_unlock(&locks[0]);

        if (i < gen_args.length)
        {
            pthread_create(&thread_ids[i], &attr, &compare, &thread_args[i]);
        }
    }

    for (int i = 0; i < gen_args.length; i++)
    {
        lock_on_condition(&locks[GENERATOR_IDX], &dirty_flags[GENERATOR_IDX], 0);

        gen_args.values[i] = output_value;
        dirty_flags[GENERATOR_IDX] = 0;

        pthread_mutex_unlock(&locks[GENERATOR_IDX]);
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
