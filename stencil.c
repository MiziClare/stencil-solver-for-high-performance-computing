/* stencil.c
 *
 * Multi-threaded 2-D box-stencil / convolution kernel.
 * Implements the operation described in Section 1 with a "copy-boundary"
 * strategy (no zero padding).  Parallelised with OpenMP.
 *
 */

#include <omp.h>

/*
 * input_vec  – pointer to the first element of a 1-D array that actually
 *              contains  b × m × n  floats (row-major order)
 * output_vec – same shape as input_vec, pre-allocated by the caller
 * filter_vec – pointer to  k × k  floats (row-major order)
 *
 * m, n – spatial dimensions of each matrix
 * k    – filter size (assumed  k ≥ 1, usually odd)
 * b    – number of matrices ("batch" dimension)
 */
void stencil(float *input_vec,
             float *output_vec,
             float *filter_vec,
             int m, int n, int k, int b)
{
    /* Re-interpret the 1-D storage as the requested higher-rank arrays.
       This makes indexing far clearer and incurs no run-time overhead. */
    float (*input)[m][n] = (float (*)[m][n])input_vec;
    float (*filter)[k] = (float (*)[k])filter_vec;
    float (*output)[m][n] = (float (*)[m][n])output_vec;

    /* Boundary widths: lower = floor((k-1)/2), upper = ceil((k-1)/2).   *
     * These work for both odd and even k, although assignment typically *
     * uses odd filters (e.g. 3×3, 5×5).                                 */
    const int lower = (k - 1) / 2;   /* floor((k-1)/2) */
    const int upper = k - lower - 1; /* ceil((k-1)/2)  */

/* Parallel region: collapse over batch and rows so each OpenMP      *
 * thread gets a sizeable contiguous chunk of work.  The innermost   *
 * column loop stays serial for data-locality of the stencil.        */
#pragma omp parallel for collapse(2) schedule(static)
    for (int batch = 0; batch < b; ++batch)
    {
        for (int i = 0; i < m; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                /* If the position lies in the boundary zone, copy it. */
                if (i < lower || i >= m - upper ||
                    j < lower || j >= n - upper)
                {
                    output[batch][i][j] = input[batch][i][j];
                }
                /* Interior point – apply the k×k stencil.            */
                else
                {
                    float acc = 0.0f;

                    /* Unroll over the filter window. */
                    for (int fi = 0; fi < k; ++fi)
                    {
                        int ii = i + fi - lower; /* input row */
                        const float *in_row = input[batch][ii];

                        for (int fj = 0; fj < k; ++fj)
                        {
                            int jj = j + fj - lower; /* input col */
                            acc += in_row[jj] * filter[fi][fj];
                        }
                    }
                    output[batch][i][j] = acc;
                }
            }
        }
    }
}
