#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <immintrin.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include "compute.h"

#pragma STDC FP_CONTRACT ON 

#define COEF_D 0.1035533905932737724
#define COEF_S 0.1464466094067262691

void do_compute(const struct parameters *p, struct results *r)
{
    size_t n_cols       = p->M;
    size_t n_rows       = p->N;
    size_t n_cells      = n_rows * n_cols;
    size_t n_iters      = p->maxiter;
    size_t n_report     = p->period;
    size_t printreports = p->printreports;

    size_t n_cpy_cols   = 2;
    size_t n_pad_cols   = n_cols % 4 - 1;
    size_t n_pad_cells  = (n_cols + n_pad_cols) * n_rows;
    size_t n_tot_cells  = (n_cols + n_pad_cols + n_cpy_cols) * n_rows;

    __m256d coef_d = _mm256_set_pd(COEF_D, COEF_D, COEF_D, COEF_D);
    __m256d coef_s = _mm256_set_pd(COEF_S, COEF_S, COEF_S, COEF_S);
    __m256d ones   = _mm256_set_pd(1.0, 1.0, 1.0, 1.0);

    // Allocating memory for the matrices. The heat matrix has 2 additional rows 
    // for the halo values.
    // NOTE: we create 2 heat matrices in order to have a front and back buffer
    // for computation that we swap at every iteration.
    double *m_heat_a = (double *)_mm_malloc(sizeof(double) * (n_tot_cells + 2 * (n_cols + n_pad_cols + n_cpy_cols)), 32);
    double *m_heat_b = (double *)_mm_malloc(sizeof(double) * (n_tot_cells + 2 * (n_cols + n_pad_cols + n_cpy_cols)), 32);
    double *m_coef   = (double *)_mm_malloc(sizeof(double) * (n_pad_cells),                                          32);

    /*
    NOTE:
    Our matrices now have padding on the right side and copycat columns. 
    Therefore, we cannot use memcpy in the same way we did for the 
    sequential implementation.

    c   x x x x x   c   p p
    -----------------------
    1 | 1 0 0 0 0 | 0 | 0 0
    0 | 0 1 0 0 0 | 0 | 0 0

    0 | 0 0 0 | 0
    0 | 0 0 0 | 0
    1 | 1 0 1 | 1

    h1h1h2h...hNpp
    x1x1x2x...xNpp
    x1x1x2x...xNpp
    h1h1h2h...hNpp

    where h = halo, x = value, p = padding (zero init), c = copycat.
    */    
    for (size_t row = 0; row < n_rows + 2; row++)
    {
        size_t idx_row      = row * n_cols;
        size_t idx_pad_row  = row * (n_cols + n_pad_cols);
        size_t idx_row_prev = idx_row - n_cols;

        for (size_t col = 0; col < n_cols + 2; col++)
        {
            // top halo
            if (row == 0)
            {
                if (col == 0)
                {
                    m_heat_a[idx_pad_row] = p->tinit[idx_row];
                }
                else if (col == n_cols + n_cpy_cols - 1)
                {
                    m_heat_a[idx_pad_row + col] = p->tinit[idx_row + col - 2];
                }
                else
                {
                    m_heat_a[idx_pad_row + col] = p->tinit[idx_row + col - 1];
                }
            } 
            // bottom halo
            else if (row == n_rows + 1)
            {
                if (col == 0)
                {
                    m_heat_a[idx_pad_row] = p->tinit[(n_rows - 1) * n_cols];
                }
                else if (col == n_cols + n_cpy_cols - 1)
                {
                    m_heat_a[idx_pad_row + col] = p->tinit[(n_rows - 1) * n_cols + col - 2];
                }
                else
                {
                    m_heat_a[idx_pad_row + col] = p->tinit[(n_rows - 1) * n_cols + col - 1];
                }
            }
            // actual matrix
            else 
            {
                if (col == 0)
                {
                    m_heat_a[idx_pad_row] = p->tinit[idx_row_prev];
                }
                else if (col == n_cols + n_cpy_cols - 1)
                {
                    m_heat_a[idx_pad_row + col] = p->tinit[idx_row_prev + col - 2];
                }
                else
                {
                    m_heat_a[idx_pad_row + col] = p->tinit[idx_row_prev + col - 1];
                }
            }
        }
    }

    memcpy(m_heat_b, m_heat_a, sizeof(double) * (n_tot_cells + + 2 * (n_cols + n_pad_cols + n_cpy_cols)));

    for (size_t row = 0; row < n_rows; row++)
    {
        size_t idx_row     = row * n_cols;
        size_t idx_pad_row = row * (n_cols + n_pad_cols);

        for (size_t col = 0; col < n_cols; col ++)
        {
            m_coef[idx_pad_row + col] = p->conductivity[idx_row + col];
        }
    }

    // iteration number
    size_t i = 1;

    // we initialize this to enter the while loop
    r->maxdiff = p->threshold;

    // NOTE: we do not include setup operations in time calculations
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    while (i < n_iters + 1 && r->maxdiff >= p->threshold)
    {
        double heat_sum = 0;

        if (i % n_report == 0 || i == n_iters)
        {
            r->niter    = i;
            r->tmax     = 0.0;
            r->tmin     = DBL_MAX;
            r->maxdiff  = 0.0;
        }

        // swapping front and back buffer at every iteration
        double *m_heat_prev = i % 2 == 0 ? m_heat_a : m_heat_b;
        double *m_heat_next = i % 2 == 0 ? m_heat_b : m_heat_a;

#ifdef DRAW_PGM
        begin_picture(i, n_cols, n_rows, p->io_tmin, p->io_tmax);
#endif

        for (size_t row = 1; row < n_rows + 1; ++row)
        {
            size_t idx_row      = row * (n_cols + n_pad_cols);
            size_t idx_row_prev = idx_row - (n_cols + n_pad_cols);
            size_t idx_row_next = idx_row + (n_cols + n_pad_cols);

            for (size_t col = 1; col < n_cols + 1; col += 4)
            {
                size_t col_prev = col - 1;
                size_t col_next = col + 1;

                // loading AVX data structures
                // simple neighbors
                __m256d neighbors_t  = _mm256_load_pd(&m_heat_prev[idx_row_prev + col]);
                __m256d neighbors_b  = _mm256_load_pd(&m_heat_prev[idx_row_next + col]);
                __m256d neighbors_l  = _mm256_load_pd(&m_heat_prev[idx_row + col_prev]);
                __m256d neighbors_r  = _mm256_load_pd(&m_heat_prev[idx_row + col_next]);
                // diagonal neighbors
                __m256d neighbors_tl = _mm256_load_pd(&m_heat_prev[idx_row_prev + col_prev]);
                __m256d neighbors_tr = _mm256_load_pd(&m_heat_prev[idx_row_prev + col_next]);
                __m256d neighbors_br = _mm256_load_pd(&m_heat_prev[idx_row_next + col_next]);
                __m256d neighbors_bl = _mm256_load_pd(&m_heat_prev[idx_row_next + col_prev]);

                // previous temperatures
                __m256d prev_heat = _mm256_load_pd(&m_heat_prev[idx_row + col]);

                // coefficients (this matrix does not have copycat columns)
                __m256d coefs   = _mm256_load_pd(&m_coef[idx_row_prev + col_prev]);                
                __m256d coefs_r = _mm256_sub_pd(ones, coefs);

                // simulation logic
                __m256d sum_neighbors_tb = _mm256_add_pd(neighbors_t, neighbors_b);
                __m256d sum_neighbors_lr = _mm256_add_pd(neighbors_l, neighbors_r);
                __m256d sum_s            = _mm256_add_pd(sum_neighbors_tb, sum_neighbors_lr);

                __m256d sum_neighbors_tltr = _mm256_add_pd(neighbors_tl, neighbors_tr);
                __m256d sum_neighbors_brbl = _mm256_add_pd(neighbors_br, neighbors_bl);
                __m256d sum_d              = _mm256_add_pd(sum_neighbors_tltr, sum_neighbors_brbl);

                __m256d sum_neighbors = _mm256_fmadd_pd(sum_s, coef_s,    _mm256_mul_pd(sum_d, coef_d));                
                __m256d next_heat     = _mm256_fmadd_pd(coefs, prev_heat, _mm256_mul_pd(coefs_r, sum_neighbors));

                double *next_heat_ptr = (double *)&next_heat;

                size_t col2 = col + 1;
                size_t col3 = col + 2;
                size_t col4 = col + 3;

                // update next temperature value
                m_heat_next[idx_row + col] = next_heat_ptr[0];
                if (col2 < n_cols + 1) m_heat_next[idx_row + col + 1] = next_heat_ptr[1];
                if (col3 < n_cols + 1) m_heat_next[idx_row + col + 2] = next_heat_ptr[2];
                if (col4 < n_cols + 1) m_heat_next[idx_row + col + 3] = next_heat_ptr[3];

                // update duplicate boundaries
                if (col == 1)
                {
                    m_heat_next[idx_row + n_cols + 1] = next_heat_ptr[0];
                }
                else if (col == n_cols)
                { 
                    m_heat_next[idx_row] = next_heat_ptr[0];
                }

                if (i % n_report == 0 || i == n_iters)
                {
                    __m256d heat_abs_diff = _mm256_sub_pd(prev_heat, next_heat);
                    double *heat_abs_diff_ptr = (double *)&heat_abs_diff;

                    heat_abs_diff_ptr[0] = fabs(heat_abs_diff_ptr[0]);
                    heat_abs_diff_ptr[1] = fabs(heat_abs_diff_ptr[1]);
                    heat_abs_diff_ptr[2] = fabs(heat_abs_diff_ptr[2]);
                    heat_abs_diff_ptr[3] = fabs(heat_abs_diff_ptr[3]);

                    heat_sum += next_heat_ptr[0];
                    if (col2 < n_cols + 1) heat_sum += next_heat_ptr[1];
                    if (col3 < n_cols + 1) heat_sum += next_heat_ptr[2];
                    if (col4 < n_cols + 1) heat_sum += next_heat_ptr[3];

                    r->tmax = fmax(r->tmax, next_heat_ptr[0]);
                    if (col2 < n_cols + 1) fmax(r->tmax, next_heat_ptr[1]);
                    if (col3 < n_cols + 1) fmax(r->tmax, next_heat_ptr[2]);
                    if (col4 < n_cols + 1) fmax(r->tmax, next_heat_ptr[3]);

                    r->tmin = fmin(r->tmin, next_heat_ptr[0]);
                    if (col2 < n_cols + 1) fmin(r->tmin, next_heat_ptr[1]);
                    if (col3 < n_cols + 1) fmin(r->tmin, next_heat_ptr[2]);
                    if (col4 < n_cols + 1) fmin(r->tmin, next_heat_ptr[3]);

                    r->maxdiff = fmax(r->maxdiff, heat_abs_diff_ptr[0]);
                    if (col2 < n_cols + 1) fmax(r->maxdiff, heat_abs_diff_ptr[1]);
                    if (col3 < n_cols + 1) fmax(r->maxdiff, heat_abs_diff_ptr[2]);
                    if (col4 < n_cols + 1) fmax(r->maxdiff, heat_abs_diff_ptr[3]);
                }

#ifdef DRAW_PGM
                draw_point(col, row - 1, next_heat_ptr[0]);
                draw_point(col, row - 1, next_heat_ptr[1]);
                draw_point(col, row - 1, next_heat_ptr[2]);
                draw_point(col, row - 1, next_heat_ptr[3]);
#endif
            }
        }
        
        if (i % n_report == 0 || i == n_iters) 
        {
            clock_gettime(CLOCK_MONOTONIC, &after);
            
            r->tavg  = heat_sum / (double) n_cells;
            r->time  = (double)(after.tv_sec - before.tv_sec) +
                       (double)(after.tv_nsec - before.tv_nsec) / 1e9;

            if (printreports && i != n_iters) report_results(p, r);
        }

        i++;

#ifdef DRAW_PGM
        end_picture();
#endif
    }

    _mm_free(m_coef);
    _mm_free(m_heat_a);
    _mm_free(m_heat_b);
}
