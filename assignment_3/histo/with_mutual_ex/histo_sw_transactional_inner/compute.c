#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdatomic.h>

typedef struct {
    int *histo;
    int *image;
    size_t idx_start;
    size_t idx_end;
} task_t;


void *compute_bins(void *arg)
{
    task_t *task = (task_t *)arg;

    int local_bins[256] = { 0 };

    for (size_t i = task->idx_start; i < task->idx_end; i++)
    {
        local_bins[task->image[i]]++;
    }

    
    for (int i = 0; i < 256; i++)
    {
        __transaction_atomic {
            task->histo[i] += local_bins[i];
        }
    }

    return NULL;
}


void histogram(int *histo, int *image, int n_threads, size_t img_size)
{
    pthread_t *thread_ids = (pthread_t *)malloc(n_threads * sizeof(pthread_t));
    
    pthread_attr_t thread_attrs = { 0 };
    pthread_attr_init(&thread_attrs);
    
    task_t *tasks = (task_t *)malloc(n_threads * sizeof(task_t));

    int chunk_size = img_size / n_threads;
    int remainder = img_size - (n_threads * chunk_size);

    int temp_remainder = remainder;
    int last_pixel = 0;

    for (int i = 0; i < n_threads; i++)
    {
        tasks[i].histo     = histo;
        tasks[i].image     = image;
        tasks[i].idx_start = last_pixel;
        tasks[i].idx_end   = last_pixel + chunk_size;

        if (temp_remainder > 0)
        {
            temp_remainder--;
            tasks[i].idx_end++;
        }
        
        last_pixel = tasks[i].idx_end;

        pthread_create(&thread_ids[i], &thread_attrs, compute_bins, &tasks[i]);
    }

    for (int i = 0; i < n_threads; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }
}