#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "compute.h"

#define DEBUG

#define COEF_D 0.103553391
#define COEF_S 0.146446609

void do_compute(const struct parameters *p, struct results *r)
{
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    size_t n_cols       = p->M;
    size_t n_rows       = p->N;
    size_t n_cells      = n_rows * n_cols;
    size_t n_iters      = p->maxiter;
    size_t n_report     = p->period;
    size_t printreports = p->printreports;

    // allocating stack space for the matrices
    // the heat matrix has 2 additional rows for the halo values
    // NOTE: we create 2 heat matrices in order to have a front and back buffer
    // for computation that we swap at every iteration
    double *m_heat_a = (double *)malloc(sizeof(double) * (n_cells + 2 * n_cols));
    double *m_heat_b = (double *)malloc(sizeof(double) * (n_cells + 2 * n_cols));
    double *m_coef   = (double *)malloc(sizeof(double) * (n_cells));

    // we copy the first and last row of the heat matrix twice for the halo values
    memcpy(m_heat_a,                    p->tinit,                      sizeof(double) * n_cols);
    memcpy(m_heat_a + n_cols,           p->tinit,                      sizeof(double) * n_cells);
    memcpy(m_heat_a + n_cells + n_cols, p->tinit + n_cells - n_cols,   sizeof(double) * n_cols);
    memcpy(m_heat_b,                    m_heat_a,                      sizeof(double) * (n_cells + 2 * n_cols));
    memcpy(m_coef,                      p->conductivity,               sizeof(double) * n_cells);

    // iteration number
    size_t i = 1;

    // we initialize this to be higher than the threshold
    r->maxdiff  = p->threshold + 1;
    r->tmax     = p->io_tmax;
    r->tmin     = p->io_tmin;

    while (i < n_iters + 1 && r->maxdiff >= p->threshold)
    {
        double heat_sum = 0;

        // swapping front and back buffer at every iteration
        double *m_heat_prev = i % 2 == 0 ? m_heat_a : m_heat_b;
        double *m_heat_next = i % 2 == 0 ? m_heat_b : m_heat_a;

#ifdef DEBUG
        begin_picture(i, n_cols, n_rows, r->tmin, r->tmax);
#endif

        if (i % n_report == 0 || i == n_iters) 
        {
            r->tmax     = 0;
            r->tmin     = 0;
            r->tavg     = 0;
            r->maxdiff  = 0;
        } 

        for (size_t row = 1; row < n_rows + 1; ++row)
        {
            size_t idx_row      = row * n_cols;
            size_t idx_prev_row = idx_row - n_cols;
            size_t idx_next_row = idx_row + n_cols;

            for (size_t col = 0; col < n_cols; ++col)
            {
                // the coef. matrix does not contain halo values
                // so we take the value for row - 1
                double coef = m_coef[idx_prev_row + col];

                // accounting for the cilinder wrapping around
                size_t prev_col = (col - 1) % n_cols;
                size_t next_col = (col + 1) % n_cols;

                double neighbors[8]; 

                // clockwise, starting at the top
                neighbors[0] = m_heat_prev[idx_prev_row + col];
                neighbors[1] = m_heat_prev[idx_prev_row + next_col];
                neighbors[2] = m_heat_prev[idx_row      + next_col];
                neighbors[3] = m_heat_prev[idx_next_row + next_col];
                neighbors[4] = m_heat_prev[idx_next_row + col];
                neighbors[5] = m_heat_prev[idx_next_row + prev_col];
                neighbors[6] = m_heat_prev[idx_row      + prev_col];
                neighbors[7] = m_heat_prev[idx_prev_row + prev_col];

                // partial diagonal (d) and direct (s) sums
                double sum_d = neighbors[1] + neighbors[3] + neighbors[5] + neighbors[7];
                double sum_s = neighbors[0] + neighbors[2] + neighbors[4] + neighbors[6];

                double prev_heat = m_heat_prev[idx_row + col];
                double next_heat = coef * (sum_d * COEF_D + sum_s * COEF_S) + (1 - coef) * prev_heat;

                double heat_abs_diff = fabs(prev_heat - next_heat);
                
                m_heat_next[idx_row + col] = next_heat;
                
                // NOTE: this is gonna slow us down considerably due to branching
                // and calling functions (fmax, fmin) inside the loop. However, we
                // do it only when the report is due and at the end of the iteration
                // so most of the times we should not be affected.
                if (i % n_report == 0 || i == n_iters) 
                {
                    r->tmax     = r->tmax > next_heat ? r->tmax : next_heat; // fmax(r->tmax, next_heat);
                    r->tmin     = r->tmin > next_heat ? r->tmin : next_heat; // fmin(r->tmin, next_heat);
                    r->maxdiff  = r->maxdiff > heat_abs_diff ? r->maxdiff : heat_abs_diff; // fmax(r->maxdiff, abs(prev_heat - next_heat));
                    heat_sum    += next_heat;
                }

#ifdef DEBUG
                draw_point(col, row - 1, prev_heat);
#endif
            }
        }

        if (i % n_report == 0) 
        {
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
                      (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            r->niter = i;
            r->tavg  = heat_sum / n_cells;
            if (printreports)
            {
                report_results(p, r);
            }
        } 

        ++i;

#ifdef DEBUG
        end_picture();
#endif
    }
    free(m_coef);
    free(m_heat_a);
    free(m_heat_b);
}
