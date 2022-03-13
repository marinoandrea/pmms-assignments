#include <time.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "compute.h"
#include "omp.h"

#pragma STDC FENV_ACCESS ON
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

    omp_set_num_threads(p->nthreads);

    // Allocating memory for the matrices. The heat matrix has 2 additional rows 
    // for the halo values.
    // NOTE: we create 2 heat matrices in order to have a front and back buffer
    // for computation that we swap at every iteration.
    double *m_heat_a = (double *)calloc(n_cells + 2 * n_cols, sizeof(double));
    double *m_heat_b = (double *)calloc(n_cells + 2 * n_cols, sizeof(double));
    double *m_coef   = (double *)calloc(n_cells,              sizeof(double));

    // we copy the first and last row of the heat matrix twice for the halo values
    memcpy(m_heat_a,                    p->tinit,                      sizeof(double) * n_cols);
    memcpy(m_heat_a + n_cols,           p->tinit,                      sizeof(double) * n_cells);
    memcpy(m_heat_a + n_cells + n_cols, p->tinit + n_cells - n_cols,   sizeof(double) * n_cols);
    memcpy(m_heat_b,                    m_heat_a,                      sizeof(double) * (n_cells + 2 * n_cols));
    memcpy(m_coef,                      p->conductivity,               sizeof(double) * n_cells);

    // precomputing column boundaries to avoid redundant modulo calculations
    size_t *lookup_prev_col = (size_t *)calloc(n_cols, sizeof(size_t));
    size_t *lookup_next_col = (size_t *)calloc(n_cols, sizeof(size_t));

    for (size_t i = 0; i < n_cols; i++)
    {
        lookup_prev_col[i] = i == 0 ? n_cols - 1 : i - 1;
        lookup_next_col[i] = i == n_cols - 1 ? 0 : i + 1;
    }

    // iteration number
    size_t i = 1;

    // we initialize this to enter the while loop
    r->maxdiff = p->threshold;

    struct timespec before, after;

    clock_gettime(CLOCK_MONOTONIC, &before);

    while (i < n_iters + 1 && r->maxdiff >= p->threshold)
    {
        // swapping front and back buffer at every iteration
        double *m_heat_prev = i % 2 == 0 ? m_heat_a : m_heat_b;
        double *m_heat_next = i % 2 == 0 ? m_heat_b : m_heat_a;

#ifdef DRAW_PGM
        begin_picture(i, n_cols, n_rows, p->io_tmin, p->io_tmax);
#endif

        size_t row;
        
        #pragma omp parallel for private(row) schedule(guided)
        for (row = 1; row < n_rows + 1; ++row)
        {
            size_t idx_row      = row * n_cols;
            size_t idx_row_prev = idx_row - n_cols;
            size_t idx_row_next = idx_row + n_cols;

            for (size_t col = 0; col < n_cols; ++col)
            {
                // the coef. matrix does not contain halo values
                // so we take the value for row - 1
                double coef = m_coef[idx_row_prev + col];

                size_t col_prev = lookup_prev_col[col];
                size_t col_next = lookup_next_col[col];

                double sum_s = m_heat_prev[idx_row_prev + col]
                             + m_heat_prev[idx_row      + col_next]
                             + m_heat_prev[idx_row_next + col]
                             + m_heat_prev[idx_row      + col_prev];
                double sum_d = m_heat_prev[idx_row_prev + col_next]
                             + m_heat_prev[idx_row_next + col_next]
                             + m_heat_prev[idx_row_next + col_prev]
                             + m_heat_prev[idx_row_prev + col_prev];

                double prev_heat = m_heat_prev[idx_row + col];
                double next_heat = (1.0 - coef) * (sum_d * COEF_D + sum_s * COEF_S) + coef * prev_heat;

                m_heat_next[idx_row + col] = next_heat;

#ifdef DRAW_PGM
                draw_point(col, row - 1, next_heat);
#endif
            }
        }
        
        if (i % n_report == 0 || i == n_iters) 
        {
            clock_gettime(CLOCK_MONOTONIC, &after);

            double heat_sum = 0.0;

            r->niter    = i;
            r->tmax     = -INFINITY;
            r->tmin     = INFINITY;
            r->maxdiff  = 0.0;

            for (size_t row = 1; row < n_rows + 1; ++row)
            {
                size_t idx_row = row * n_cols;

                for (size_t col = 0; col < n_cols; ++col)
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

#ifdef DRAW_PGM
        end_picture();
#endif
    }

    free(lookup_next_col);
    free(lookup_prev_col);
    free(m_coef);
    free(m_heat_a);
    free(m_heat_b);
}
