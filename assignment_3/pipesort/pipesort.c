#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

/* Output from rand() is >= 0, so guaranteed to be different from END. */
#define END -1

#define BUFFER_SIZE 2048

typedef struct thread_buffer_t {
   int data[BUFFER_SIZE];
   int occupied;
   int next_in;
   int next_out;
   pthread_mutex_t lock;
   pthread_cond_t items;
   pthread_cond_t space;
} thread_buffer_t;

/* Function arguments for the comparator threads */
typedef struct thread_comp_args_t {
    size_t index;
    thread_buffer_t *producer_buffer;
    thread_buffer_t *consumer_buffer;
} thread_comp_args_t;

typedef struct thread_out_args_t {
    thread_buffer_t *consumer_buffer;
} thread_out_args_t;

typedef struct thread_gen_args_t {
    int *values;
    size_t length;
} thread_gen_args_t;

typedef enum 
{
    READ_0,
    READ_1,
    READ_2,
    WRITE_1,
    WRITE_2,
    WRITE_ONCE,
    WRITE_END_1,
    WRITE_END_2
} comparator_state;

/* Pthread attributes are shared between all threads */
static pthread_attr_t attr;

void *output(void *arg)
{
    thread_out_args_t *args = (thread_out_args_t *)arg;

    int count_end = 0;

    while (count_end < 2)
    {
        pthread_mutex_lock(&args->consumer_buffer->lock);
        while (args->consumer_buffer->occupied == 0)
        {
            pthread_cond_wait(&args->consumer_buffer->items, 
                              &args->consumer_buffer->lock);
        }

        int value_in = args->consumer_buffer->data[args->consumer_buffer->next_out];
        
        if (value_in == END)
        {
            count_end++;
        }
        else if (count_end == 1)
        {
            printf("%i ", value_in);
        }

        args->consumer_buffer->next_out = (args->consumer_buffer->next_out + 1) % BUFFER_SIZE;
        args->consumer_buffer->occupied--;

        pthread_cond_signal(&args->consumer_buffer->space);
        pthread_mutex_unlock(&args->consumer_buffer->lock);   
    }

    printf("\n");
    return NULL;
}

void *compare(void *arg) 
{    
    thread_comp_args_t *args = (thread_comp_args_t *)arg;

    int value_hold = 0;
    int value_pass = 0;
    comparator_state state = READ_0;

    for (;;)
    {
        switch (state)
        {
        case READ_0:
        {
            pthread_mutex_lock(&args->consumer_buffer->lock);
            while (args->consumer_buffer->occupied == 0)
            {
                pthread_cond_wait(&args->consumer_buffer->items, 
                                  &args->consumer_buffer->lock);
            }
            
            value_hold = args->consumer_buffer->data[args->consumer_buffer->next_out];
            
            state = READ_1;

            args->consumer_buffer->next_out = (args->consumer_buffer->next_out + 1) % BUFFER_SIZE;
            args->consumer_buffer->occupied--;

            pthread_cond_signal(&args->consumer_buffer->space);
            pthread_mutex_unlock(&args->consumer_buffer->lock);  

            continue;
        }

        case READ_1:
        {
            pthread_mutex_lock(&args->consumer_buffer->lock);
            while (args->consumer_buffer->occupied == 0)
            {
                pthread_cond_wait(&args->consumer_buffer->items, 
                                  &args->consumer_buffer->lock);
            }

            int value_in = args->consumer_buffer->data[args->consumer_buffer->next_out];

            if (value_in == END)
            {
                state = WRITE_END_1;
            } 
            else
            {
                if (value_in > value_hold)
                {
                    value_pass = value_hold;
                    value_hold = value_in;
                } 
                else 
                {
                    value_pass = value_in;
                }

                state = WRITE_1;
            }

            args->consumer_buffer->next_out = (args->consumer_buffer->next_out + 1) % BUFFER_SIZE;
            args->consumer_buffer->occupied--;

            pthread_cond_signal(&args->consumer_buffer->space);
            pthread_mutex_unlock(&args->consumer_buffer->lock);  

            continue;
        }

        case WRITE_ONCE:
        case WRITE_END_1:
        {
            pthread_mutex_lock(&args->producer_buffer->lock);
            while (args->producer_buffer->occupied == BUFFER_SIZE)
            {
                pthread_cond_wait(&args->producer_buffer->space, 
                                  &args->producer_buffer->lock);
            }

            args->producer_buffer->data[args->producer_buffer->next_in] = (state == WRITE_END_1) ? END : value_hold;

            state = (state == WRITE_END_1) ? WRITE_ONCE : READ_2;

            args->producer_buffer->next_in = (args->producer_buffer->next_in + 1) % BUFFER_SIZE;
            args->producer_buffer->occupied++;

            pthread_cond_signal(&args->producer_buffer->items);
            pthread_mutex_unlock(&args->producer_buffer->lock);   

            continue;
        }

        case READ_2:
        {
            pthread_mutex_lock(&args->consumer_buffer->lock);
            while (args->consumer_buffer->occupied == 0)
            {
                pthread_cond_wait(&args->consumer_buffer->items, 
                                  &args->consumer_buffer->lock);
            }

            int value_in = args->consumer_buffer->data[args->consumer_buffer->next_out];

            if (value_in == END)
            {
                state = WRITE_END_2;
            }
            else
            {
                value_pass = value_in;
                state = WRITE_2;
            }

            args->consumer_buffer->next_out = (args->consumer_buffer->next_out + 1) % BUFFER_SIZE;
            args->consumer_buffer->occupied--;

            pthread_cond_signal(&args->consumer_buffer->space);
            pthread_mutex_unlock(&args->consumer_buffer->lock);

            continue;
        }

        case WRITE_1:
        case WRITE_2:
        {
            pthread_mutex_lock(&args->producer_buffer->lock);
            while (args->producer_buffer->occupied == BUFFER_SIZE)
            {
                pthread_cond_wait(&args->producer_buffer->space, 
                                  &args->producer_buffer->lock);
            }
            
            args->producer_buffer->data[args->producer_buffer->next_in] = value_pass;

            state = (state == WRITE_1) ? READ_1 : READ_2;

            args->producer_buffer->next_in = (args->producer_buffer->next_in + 1) % BUFFER_SIZE;
            args->producer_buffer->occupied++;

            pthread_cond_signal(&args->producer_buffer->items);
            pthread_mutex_unlock(&args->producer_buffer->lock);   

            continue;
        }

        case WRITE_END_2:
        {
            pthread_mutex_lock(&args->producer_buffer->lock);
            while (args->producer_buffer->occupied == BUFFER_SIZE)
            {
                pthread_cond_wait(&args->producer_buffer->space, 
                                  &args->producer_buffer->lock);
            }
                                    
            args->producer_buffer->data[args->producer_buffer->next_in] = END;

            args->producer_buffer->next_in = (args->producer_buffer->next_in + 1) % BUFFER_SIZE;
            args->producer_buffer->occupied++;

            pthread_cond_signal(&args->producer_buffer->items);
            pthread_mutex_unlock(&args->producer_buffer->lock);      

            return NULL;
        }

        default:
            continue;
        }        
    }
}


void *generate(void *arg)
{
    thread_gen_args_t gen_args = *(thread_gen_args_t *)arg;

    pthread_t *thread_ids    = (pthread_t *)       malloc((gen_args.length + 1) * sizeof(pthread_t));
    thread_buffer_t *buffers = (thread_buffer_t *) malloc((gen_args.length + 1) * sizeof(thread_buffer_t));

    // comparator threads args
    thread_comp_args_t *thread_args = (thread_comp_args_t *)malloc(gen_args.length * sizeof(thread_comp_args_t));
    
    // output thread args
    thread_out_args_t output_args = { 0 };
    output_args.consumer_buffer = &buffers[gen_args.length];

    // spawn the threads
    for (int i = 0; i < gen_args.length + 1; i++)  
    {
        pthread_mutex_init(&buffers[i].lock, NULL);
        pthread_cond_init(&buffers[i].items, NULL);
        pthread_cond_init(&buffers[i].space, NULL);

        buffers[i].next_in = 0;
        buffers[i].next_out = 0;
        buffers[i].occupied = 0;

        if (i < gen_args.length)
        {
            thread_args[i].index = i;
            thread_args[i].consumer_buffer = &buffers[i];
            thread_args[i].producer_buffer = &buffers[i + 1];

            pthread_create(&thread_ids[i], &attr, &compare, &thread_args[i]);
        }
        else
        {
            pthread_create(&thread_ids[i], &attr, &output, &output_args);
        }
    }

    // generate numbers
    for (int i = 0; i < gen_args.length + 2; i++)
    {        
        pthread_mutex_lock(&buffers[0].lock);
        while (buffers[0].occupied == BUFFER_SIZE)
        {
            pthread_cond_wait(&buffers[0].space, 
                              &buffers[0].lock);
        }

        buffers[0].data[buffers[0].next_in] = (i < gen_args.length) ? rand() : END;

        buffers[0].next_in = (buffers[0].next_in + 1) % BUFFER_SIZE;
        buffers[0].occupied++;

        pthread_cond_signal(&buffers[0].items);
        pthread_mutex_unlock(&buffers[0].lock);
    }

    // terminate
    for (int i = 0; i < gen_args.length + 1; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

    return NULL;
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

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);
}
