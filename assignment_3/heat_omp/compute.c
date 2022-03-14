#include <time.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "compute.h"

#pragma STDC FENV_ACCESS ON
#pragma STDC FP_CONTRACT ON

#define COEF_D 0.1035533905932
#define COEF_S 0.1464466094067

void do_compute(const struct parameters *p, struct results *r)
{
    int n_cols       = p->M;
    int n_rows       = p->N;
    int n_cells      = n_rows * n_cols;
    int n_iters      = p->maxiter;
    int n_report     = p->period;
    int printreports = p->printreports;

    int n_cols_actual  = n_cols + 2;
    int n_rows_actual  = n_rows + 2;
    int n_cells_actual = n_cols_actual * n_rows_actual;

    omp_set_num_threads(p->nthreads);

    // Allocating memory for the matrices. The heat matrix has 2 additional rows 
    // for the halo values.
    // NOTE: we create 2 heat matrices in order to have a front and back buffer
    // for computation that we swap at every iteration.
    double *m_heat_a = malloc(n_cells_actual * sizeof(double));
    double *m_heat_b = malloc(n_cells_actual * sizeof(double));
    double *m_coef   = malloc(n_cells_actual * sizeof(double));

    // copy real matrix
    for (int row = 0; row < n_rows; ++row)
    {
        for (int col = 0; col < n_cols; ++col)
        {
            m_heat_a[(row + 1) * n_cols_actual + (col + 1)] = p->tinit[row * n_cols + col];
            m_coef[(row + 1) * n_cols_actual + (col + 1)] = p->conductivity[row * n_cols + col];
        }
    }

    // copy halo
    for (int col = 0; col < n_cols_actual; ++col)
    {
        m_heat_a[col] = m_heat_a[n_cols_actual + col];
        m_heat_a[(n_rows_actual - 1) * n_cols_actual + col] = m_heat_a[(n_rows_actual - 2) * n_cols_actual + col]; 
    }

    memcpy(m_heat_b, m_heat_a, n_cells_actual * sizeof(double));

    // iteration number
    int i = 1;

    // we initialize this to enter the while loop
    r->maxdiff = p->threshold;

    struct timespec before, after;

    clock_gettime(CLOCK_MONOTONIC, &before);

    while (i < n_iters + 1 && r->maxdiff >= p->threshold)
    {
        // swapping front and back buffer at every iteration
        double *m_heat_prev = i % 2 == 0 ? m_heat_a : m_heat_b;
        double *m_heat_next = i % 2 == 0 ? m_heat_b : m_heat_a;

        // copy border cells
        for (int row = 0; row < n_rows_actual; ++row)
        {
            m_heat_prev[row * n_cols_actual] = m_heat_prev[row * n_cols_actual + n_cols_actual - 2];
            m_heat_prev[row * n_cols_actual + n_cols_actual - 1] = m_heat_prev[row * n_cols_actual + 1];
        }

#ifdef DRAW_PGM
        begin_picture(i, n_cols, n_rows, p->io_tmin, p->io_tmax);
#endif
        int row;

        #pragma omp parallel for private(row) schedule(guided)
        for (row = 1; row < n_rows_actual - 1; ++row)
        {
            int idx_row      = row * n_cols_actual;
            int idx_row_prev = idx_row - n_cols_actual;
            int idx_row_next = idx_row + n_cols_actual;

            for (int col = 1; col < n_cols_actual - 1; ++col)
            {
                double coef = m_coef[idx_row + col];

                int col_prev = col - 1;
                int col_next = col + 1;

                double prev_heat = m_heat_prev[idx_row + col];            

                double sum_s = m_heat_prev[idx_row_prev + col]
                            + m_heat_prev[idx_row      + col_next]
                            + m_heat_prev[idx_row_next + col]
                            + m_heat_prev[idx_row      + col_prev];
                double sum_d = m_heat_prev[idx_row_prev + col_next]
                            + m_heat_prev[idx_row_next + col_next]
                            + m_heat_prev[idx_row_next + col_prev]
                            + m_heat_prev[idx_row_prev + col_prev];

                double next_heat = (1.0 - coef) * (sum_d * COEF_D + sum_s * COEF_S) + coef * prev_heat;

                m_heat_next[idx_row + col] = next_heat;

#ifdef DRAW_PGM
                draw_point(col - 1, row - 1, next_heat);
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

#ifdef DRAW_PGM
        end_picture();
#endif
    }

    free(m_coef);
    free(m_heat_a);
    free(m_heat_b);
}
