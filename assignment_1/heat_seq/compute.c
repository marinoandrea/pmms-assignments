#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "compute.h"

void do_compute(const struct parameters *p, struct results *r)
{
    size_t n_cols = p->M;
    size_t n_rows = p->N;
    size_t n_cells = n_rows * n_cols;
    size_t n_iters = p->maxiter;

    // allocating stack space for the matrices
    // the heat matrix has 2 additional rows for the halo values
    double *m_heat = (double *)alloca(sizeof(double) * (n_cells + 2 * n_cols));
    double *m_coef = (double *)alloca(sizeof(double) * (n_cells));

    // we copy the first and last row of the heat matrix twice for the halo values
    memcpy(&m_heat,                     &p->tinit,                      sizeof(double) * n_cols);
    memcpy(&m_heat + n_cols,            &p->tinit,                      sizeof(double) * n_cells);
    memcpy(&m_heat + n_cells + n_cols,  &p->tinit + n_cells - n_cols,   sizeof(double) * n_cols);
    memcpy(&m_coef,                     &p->conductivity,               sizeof(double) * n_cells);

    double neighbors[8]; // clockwise, starting at the top

    // constants
    double w_coef_d = sqrt(2) / (sqrt(2) + 1);
    double w_coef_s = 1       / (sqrt(2) + 1);

    for (size_t i = 0; i < n_iters; ++i)
    {
        for (size_t row = 1; row < n_rows + 1; ++row)
        {
            size_t prev_row = row - 1;
            size_t next_row = row + 1;

            for (size_t col = 0; col < n_cols; ++col)
            {
                // the coef. matrix does not contain halo values
                // so we take the value for row - 1
                double coef = m_coef[prev_row * n_cols + col];

                // accounting for the cilinder wrapping around
                size_t prev_col = (col - 1) % n_cols;
                size_t next_col = (col + 1) % n_cols;

                neighbors[0] = m_heat[prev_row * n_cols + col];
                neighbors[1] = m_heat[prev_row * n_cols + next_col];
                neighbors[2] = m_heat[row      * n_cols + next_col];
                neighbors[3] = m_heat[next_row * n_cols + next_col];
                neighbors[4] = m_heat[next_row * n_cols + col];
                neighbors[5] = m_heat[next_row * n_cols + prev_col];
                neighbors[6] = m_heat[row      * n_cols + prev_col];
                neighbors[7] = m_heat[prev_row * n_cols + prev_col];

                // diagonal (d) and direct (s) coefficients
                double coef_r = 1 - coef;
                double coef_d = coef_r * w_coef_d;
                double coef_s = coef_r * w_coef_s;

                // partial diagonal (d) and direct (s) sums
                double sum_d = neighbors[1] + neighbors[3] + neighbors[5] + neighbors[7];
                double sum_s = neighbors[0] + neighbors[2] + neighbors[4] + neighbors[6];

                double prev_heat = m_heat[row * n_cols + col];

                m_heat[row * n_cols + col] = coef * prev_heat + sum_d * coef_d + sum_s * coef_s;
            }
        }
    }
}
