#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "compute.h"

#define DEBUG

void do_compute(const struct parameters *p, struct results *r)
{
    size_t n_cols   = p->M;
    size_t n_rows   = p->N;
    size_t n_cells  = n_rows * n_cols;
    size_t n_iters  = p->maxiter;
    size_t n_report = p->period;

    r->niter    = 0;
    r->tmax     = 0;
    r->tmin     = 0;
    r->tavg     = 0;
    r->maxdiff  = 0;
    r->time     = 0;

    double heat_sum = 0;

    // allocating stack space for the matrices
    // the heat matrix has 2 additional rows for the halo values
    double *m_heat = (double *)malloc(sizeof(double) * (n_cells + 2 * n_cols));
    double *m_coef = (double *)malloc(sizeof(double) * (n_cells));

    // we copy the first and last row of the heat matrix twice for the halo values
    memcpy(m_heat,                     p->tinit,                      sizeof(double) * n_cols);
    memcpy(m_heat + n_cols,            p->tinit,                      sizeof(double) * n_cells);    
    memcpy(m_heat + n_cells + n_cols,  p->tinit + n_cells - n_cols,   sizeof(double) * n_cols);    
    memcpy(m_coef,                     p->conductivity,               sizeof(double) * n_cells);

    double neighbors[8]; // clockwise, starting at the top

    // constants
    double w_coef_d = sqrt(2) / (sqrt(2) + 1);
    double w_coef_s = 1       / (sqrt(2) + 1);

    for (size_t i = 1; i < n_iters + 1; ++i)
    {
        if (i % n_report == 0 || i == n_iters) 
        {
            r->tmax     = 0;
            r->tmin     = 0;
            r->tavg     = 0;
            r->maxdiff  = 0;
            r->time     = 0;
        } 

#ifdef DEBUG
        begin_picture(i, n_cols, n_rows, p->io_tmin, p->io_tmax);
#endif

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

                neighbors[0] = m_heat[idx_prev_row + col];
                neighbors[1] = m_heat[idx_prev_row + next_col];
                neighbors[2] = m_heat[idx_row      + next_col];
                neighbors[3] = m_heat[idx_next_row + next_col];
                neighbors[4] = m_heat[idx_next_row + col];
                neighbors[5] = m_heat[idx_next_row + prev_col];
                neighbors[6] = m_heat[idx_row      + prev_col];
                neighbors[7] = m_heat[idx_prev_row + prev_col];

                // diagonal (d) and direct (s) coefficients
                double coef_r = 1 - coef;
                double coef_d = coef_r * w_coef_d;
                double coef_s = coef_r * w_coef_s;

                // partial diagonal (d) and direct (s) sums
                double sum_d = neighbors[1] + neighbors[3] + neighbors[5] + neighbors[7];
                double sum_s = neighbors[0] + neighbors[2] + neighbors[4] + neighbors[6];

                double prev_heat        = m_heat[row * n_cols + col];
                double next_heat        = coef * prev_heat + sum_d * coef_d + sum_s * coef_s;
                double heat_abs_diff    = abs(prev_heat - next_heat);
                m_heat[row * n_cols + col] = next_heat;
                
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
                draw_point(col, row - 1, next_heat);
#endif
            }
        }

        if (i % n_report == 0 || i == n_iters) 
        {
            r->niter = i;
            r->tavg  = heat_sum / n_cells;
            report_results(p, r);
        } 

#ifdef DEBUG
        end_picture();
#endif
    }

    free(m_coef);
    free(m_heat);
}
