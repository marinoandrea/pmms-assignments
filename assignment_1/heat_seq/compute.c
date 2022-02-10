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
    memcpy(&m_heat,                         &p->tinit,                      sizeof(double) * (n_cols));
    memcpy(&m_heat + n_cols,                &p->tinit,                      sizeof(double) * (n_cells));
    memcpy(&m_heat + (n_cells + n_cols),    &p->tinit + (n_cells - n_cols), sizeof(double) * (n_cols));
    memcpy(&m_coef,                         &p->conductivity,               sizeof(double) * (n_cells));

    double neighbors[8]; // clockwise, starting at the top

    for (size_t i = 0; i < n_iters; ++i)
    {
        for (size_t row = 1; row < n_rows + 1; ++row)
        {
            for (size_t col = 0; col < n_cols; ++col)
            {
                double coef = m_coef[(row - 1) * n_cols + col];

                // accounting for the cilinder wrapping around
                size_t prev_col = (col - 1) % n_cols;
                size_t next_col = (col + 1) % n_cols;

                neighbors[0] = m_heat[(row - 1) * n_cols + (col)];
                neighbors[1] = m_heat[(row - 1) * n_cols + next_col];
                neighbors[2] = m_heat[row * n_cols + next_col];
                neighbors[3] = m_heat[(row + 1) * n_cols + next_col];
                neighbors[4] = m_heat[(row + 1) * n_cols + col];
                neighbors[5] = m_heat[(row + 1) * n_cols + prev_col];
                neighbors[6] = m_heat[row * n_cols + prev_col];
                neighbors[7] = m_heat[(row - 1) * n_cols + prev_col];
            }
        }
    }
}
