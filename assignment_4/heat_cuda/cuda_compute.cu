#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cuda.h>
extern "C" 
{
    #include "compute.h"
}

#define BLOCK_SIZE 512

__device__ __constant__ double COEF_D = 0.1035533905932;
__device__ __constant__ double COEF_S = 0.1464466094067;

/**
 * @brief Copy side columns in the temperature matrix.
 * This function is supposed to be called with a thread per each row.
 * 
 * @param m_heat Pointer to the temperature matrix.
 * @param n_cols Number of columns in the matrix.
 * @param n_rows Number of rows in the matrix.
 * @return __global__ 
 */
__global__ void k_copy_columns(double* m_heat, size_t n_cols, size_t n_rows)
{
    size_t g_idx = blockIdx.x * blockDim.x + threadIdx.x;
 
    if (g_idx >= n_rows) return;
  
    size_t row_idx = g_idx * n_cols;

    m_heat[row_idx]              = m_heat[row_idx + n_cols - 2];
    m_heat[row_idx + n_cols - 1] = m_heat[row_idx + 1];
}

__global__ void k_compute_temp(
    double* m_heat_prev, 
    double* m_heat_next, 
    double* m_coef,
    size_t n_cols, size_t n_cols_actual,
    size_t n_rows, size_t n_rows_actual)
{
    size_t idx_cell = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx_cell < n_rows * n_cols) 
    {
        size_t row = idx_cell / n_cols;
        size_t col = idx_cell - (n_cols * row);
        
        size_t idx_cell_actual          = ((row + 1) * n_cols_actual) + col + 1;
        size_t idx_cell_actual_prev_row = idx_cell_actual - n_cols_actual;
        size_t idx_cell_actual_next_row = idx_cell_actual + n_cols_actual;

        double coef      = m_coef[idx_cell_actual];
        double prev_heat = m_heat_prev[idx_cell_actual];            

        double sum_s = m_heat_prev[idx_cell_actual_prev_row]
                    + m_heat_prev[idx_cell_actual + 1]
                    + m_heat_prev[idx_cell_actual_next_row]
                    + m_heat_prev[idx_cell_actual - 1];
        double sum_d = m_heat_prev[idx_cell_actual_prev_row + 1]
                    + m_heat_prev[idx_cell_actual_next_row + 1]
                    + m_heat_prev[idx_cell_actual_next_row - 1]
                    + m_heat_prev[idx_cell_actual_prev_row - 1];

        double next_heat = (1.0 - coef) * (sum_d * COEF_D + sum_s * COEF_S) + coef * prev_heat;

        m_heat_next[idx_cell_actual] = next_heat;
    }
}

extern "C" 
void cuda_do_compute(const struct parameters *p, struct results *r)
{
    struct timespec before, after;

    int i = 1;

    int n_cols       = p->M;
    int n_rows       = p->N;
    int n_cells      = n_rows * n_cols;
    int n_iters      = p->maxiter;
    int n_report     = p->period;
    int printreports = p->printreports;

    // we initialize this to enter the while loop
    r->maxdiff = p->threshold;

    int n_cols_actual  = n_cols + 2;
    int n_rows_actual  = n_rows + 2;
    int n_cells_actual = n_cols_actual * n_rows_actual;

    double *m_heat_prev = (double*) malloc(n_cells_actual * sizeof(double));
    double *m_heat_next = (double*) malloc(n_cells_actual * sizeof(double));
    double *m_coef   = (double*) malloc(n_cells_actual * sizeof(double));

    double *m_heat_prev_device = NULL;
    double *m_heat_next_device = NULL;
    double *m_coef_device      = NULL;

    if (cudaMalloc(&m_heat_prev_device, n_cells_actual * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&m_heat_next_device, n_cells_actual * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&m_coef_device,      n_cells_actual * sizeof(double)) != cudaSuccess) goto end;

    if (m_heat_prev_device == NULL || m_heat_next_device == NULL || m_coef_device == NULL) goto end;

    // copy real matrix
    for (int row = 0; row < n_rows; ++row)
    {
        for (int col = 0; col < n_cols; ++col)
        {
            m_heat_prev[(row + 1) * n_cols_actual + (col + 1)] = p->tinit[row * n_cols + col];
            m_coef[(row + 1) * n_cols_actual + (col + 1)] = p->conductivity[row * n_cols + col];
        }
    }

    // copy halo
    for (int col = 0; col < n_cols_actual; ++col)
    {
        m_heat_prev[col] = m_heat_prev[n_cols_actual + col];
        m_heat_prev[(n_rows_actual - 1) * n_cols_actual + col] = m_heat_prev[(n_rows_actual - 2) * n_cols_actual + col]; 
    }

    memcpy(m_heat_next, m_heat_prev, n_cells_actual * sizeof(double));

    if (cudaMemcpy(m_heat_prev_device, m_heat_prev, n_cells_actual * sizeof(double), cudaMemcpyHostToDevice) != cudaSuccess) goto end;
    if (cudaMemcpy(m_heat_next_device, m_heat_next, n_cells_actual * sizeof(double), cudaMemcpyHostToDevice) != cudaSuccess) goto end;    
    if (cudaMemcpy(m_coef_device,      m_coef,      n_cells_actual * sizeof(double), cudaMemcpyHostToDevice) != cudaSuccess) goto end;

    clock_gettime(CLOCK_MONOTONIC, &before);

    while (i < n_iters + 1 && r->maxdiff >= p-> threshold)
    {
        // swap buffers
        { 
            double *temp = m_heat_prev_device; 
            m_heat_prev_device = m_heat_next_device; 
            m_heat_next_device = temp; 
        }

        // copy border cells
        k_copy_columns<<<n_rows_actual / BLOCK_SIZE + 1, BLOCK_SIZE>>>(m_heat_prev_device, n_cols_actual, n_rows_actual);
        // perform simulation
        k_compute_temp<<<n_cells / BLOCK_SIZE + 1, BLOCK_SIZE>>>(
            m_heat_prev_device, 
            m_heat_next_device, 
            m_coef_device, 
            n_cols, n_cols_actual, 
            n_rows, n_rows_actual);

        if (i % n_report == 0 || i == n_iters) 
        {
            cudaDeviceSynchronize();

            clock_gettime(CLOCK_MONOTONIC, &after);

            if (cudaMemcpy(m_heat_prev, m_heat_prev_device, n_cells_actual * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) goto end;
            if (cudaMemcpy(m_heat_next, m_heat_next_device, n_cells_actual * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) goto end;    

            double heat_sum = 0.0;

            r->niter    = i;
            r->tmax     = -INFINITY;
            r->tmin     = INFINITY;
            r->maxdiff  = 0.0;

            for (int row = 1; row < n_rows_actual - 1; ++row)
            {
                int idx_row = row * n_cols_actual;

                for (int col = 1; col < n_cols_actual - 1; ++col)
                {   
                    double next_heat = m_heat_next[idx_row + col];
                    double prev_heat = m_heat_prev[idx_row + col];

                    double heat_abs_diff = fabs(prev_heat - next_heat);

                    r->tmax     = fmax(r->tmax, next_heat);
                    r->tmin     = fmin(r->tmin, next_heat);
                    r->maxdiff  = fmax(r->maxdiff, heat_abs_diff);

                    heat_sum += next_heat;
                }
            }

            r->tavg  = heat_sum / (double) n_cells;
            r->time  = (double)(after.tv_sec - before.tv_sec) +
                       (double)(after.tv_nsec - before.tv_nsec) / 1e9;

            if (printreports && i != n_iters) report_results(p, r);
        }

        i++;
    }

end:

    cudaFree(m_coef_device);
    cudaFree(m_heat_prev_device);
    cudaFree(m_heat_next_device);

    free(m_coef);
    free(m_heat_prev);
    free(m_heat_next);
}
