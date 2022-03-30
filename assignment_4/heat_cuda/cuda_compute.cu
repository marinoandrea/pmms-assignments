#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cuda.h>
extern "C" 
{
    #include "compute.h"
}

#define BLOCK_SIZE 128
#define BLOCK_SIZE_REPORT 128

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
 
    if (g_idx < n_rows)
    {
        size_t row_idx = g_idx * n_cols;

        m_heat[row_idx]              = m_heat[row_idx + n_cols - 2];
        m_heat[row_idx + n_cols - 1] = m_heat[row_idx + 1];
    }
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

__global__ void k_compute_report(
    double* m_heat_prev, 
    double* m_heat_next, 
    size_t n_cols, size_t n_cols_actual,
    size_t n_rows, size_t n_rows_actual,
    double *out_g_sums,
    double *out_g_difs,
    double *out_g_mins,
    double *out_g_maxs)
{
    __shared__ double s_data[BLOCK_SIZE_REPORT * sizeof(double) * 4];

    double *s_sums = s_data;
    double *s_difs = &s_data[BLOCK_SIZE];
    double *s_mins = &s_data[BLOCK_SIZE * 2];
    double *s_maxs = &s_data[BLOCK_SIZE * 3];

    unsigned int tid = threadIdx.x;
    unsigned int gid = blockIdx.x * blockDim.x + threadIdx.x;

    s_sums[tid] = 0.0;
    s_mins[tid] = INFINITY;
    s_maxs[tid] = -INFINITY;
    s_difs[tid] = -INFINITY;

    if (gid < n_rows * n_cols)
    {
        size_t row = gid / n_cols;
        size_t col = gid - (n_cols * row);
        size_t idx_cell_actual = ((row + 1) * n_cols_actual) + col + 1;

        s_sums[tid] = m_heat_next[idx_cell_actual];
        s_mins[tid] = m_heat_next[idx_cell_actual];
        s_maxs[tid] = m_heat_next[idx_cell_actual];
        s_difs[tid] = fabs(m_heat_next[idx_cell_actual] - m_heat_prev[idx_cell_actual]);
        __syncthreads();

        for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) 
        {
            if (tid < s)
            {
                s_sums[tid] += s_sums[tid + s];
                s_mins[tid] = fmin(s_mins[tid + s], s_mins[tid]);
                s_maxs[tid] = fmax(s_maxs[tid + s], s_maxs[tid]);
                s_difs[tid] = fmax(s_difs[tid + s], s_difs[tid]);
            }
            __syncthreads();
        }

        if (tid == 0)
        {
            out_g_sums[blockIdx.x] = s_sums[0];
            out_g_mins[blockIdx.x] = s_mins[0];
            out_g_maxs[blockIdx.x] = s_maxs[0];
            out_g_difs[blockIdx.x] = s_difs[0];
        }
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

    unsigned int n_blocks = n_cells / BLOCK_SIZE + 1;

    // we initialize this to enter the while loop
    r->maxdiff = p->threshold;

    int n_cols_actual  = n_cols + 2;
    int n_rows_actual  = n_rows + 2;
    int n_cells_actual = n_cols_actual * n_rows_actual;

    // host temperature and coefficient matrices
    double *m_heat_prev = (double*) malloc(n_cells_actual * sizeof(double));
    double *m_heat_next = (double*) malloc(n_cells_actual * sizeof(double));
    double *m_coef      = (double*) malloc(n_cells_actual * sizeof(double));

    // host redution buffers
    double *sums = (double*) malloc(n_blocks * sizeof(double));
    double *difs = (double*) malloc(n_blocks * sizeof(double));
    double *mins = (double*) malloc(n_blocks * sizeof(double));
    double *maxs = (double*) malloc(n_blocks * sizeof(double));

    // device temperature and coefficient matrices
    double *m_heat_prev_device = NULL;
    double *m_heat_next_device = NULL;
    double *m_coef_device      = NULL;

    // device redution buffers
    double *sums_device = NULL;
    double *difs_device = NULL;
    double *mins_device = NULL;
    double *maxs_device = NULL;

    // allocate device global memory temperature and coefficient matrices
    if (cudaMalloc(&m_heat_prev_device, n_cells_actual * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&m_heat_next_device, n_cells_actual * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&m_coef_device,      n_cells_actual * sizeof(double)) != cudaSuccess) goto end;

    if (m_heat_prev_device == NULL || m_heat_next_device == NULL || m_coef_device == NULL) goto end;

    // allocate device global memory reduction buffers
    if (cudaMalloc(&sums_device, n_blocks * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&difs_device, n_blocks * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&mins_device, n_blocks * sizeof(double)) != cudaSuccess) goto end;
    if (cudaMalloc(&maxs_device, n_blocks * sizeof(double)) != cudaSuccess) goto end;

    if (sums_device == NULL || difs_device == NULL || mins_device == NULL || maxs_device == NULL) goto end;

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
        k_compute_temp<<<n_blocks, BLOCK_SIZE>>>(
            m_heat_prev_device, 
            m_heat_next_device, 
            m_coef_device, 
            n_cols, n_cols_actual, 
            n_rows, n_rows_actual);
            
        if (i % n_report == 0 || i == n_iters) 
        {
            cudaDeviceSynchronize();

            clock_gettime(CLOCK_MONOTONIC, &after);

            k_compute_report<<<n_cells / BLOCK_SIZE_REPORT + 1, BLOCK_SIZE_REPORT>>>(
                m_heat_prev_device, 
                m_heat_next_device, 
                n_cols, n_cols_actual, 
                n_rows, n_rows_actual,
                sums_device,
                difs_device,
                mins_device,
                maxs_device);

            if (cudaMemcpy(sums, sums_device, n_blocks * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) goto end;
            if (cudaMemcpy(mins, mins_device, n_blocks * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) goto end;    
            if (cudaMemcpy(maxs, maxs_device, n_blocks * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) goto end;
            if (cudaMemcpy(difs, difs_device, n_blocks * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) goto end;

            // complete reduction on CPU
            double heat_sum = 0.0;

            r->niter    = i;
            r->tmax     = -INFINITY;
            r->tmin     = INFINITY;
            r->maxdiff  = -INFINITY;

            for (int idx = 0; idx < n_blocks; ++idx)
            {                
                r->tmax     = fmax(r->tmax,    maxs[idx]);
                r->tmin     = fmin(r->tmin,    mins[idx]);
                r->maxdiff  = fmax(r->maxdiff, difs[idx]);
                heat_sum    += sums[idx];
            }

            r->tavg  = heat_sum / (double) n_cells;
            r->time  = (double)(after.tv_sec - before.tv_sec) +
                       (double)(after.tv_nsec - before.tv_nsec) / 1e9;

            if (printreports && i != n_iters) report_results(p, r);
        }

        i++;
    }

end:

    cudaFree(sums_device);
    cudaFree(mins_device);
    cudaFree(maxs_device);
    cudaFree(difs_device);

    cudaFree(m_coef_device);
    cudaFree(m_heat_prev_device);
    cudaFree(m_heat_next_device);

    free(sums);
    free(mins);
    free(maxs);
    free(difs);

    free(m_coef);
    free(m_heat_prev);
    free(m_heat_next);
}
