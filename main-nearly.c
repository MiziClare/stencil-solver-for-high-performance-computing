/* ------------------------------------------------------------
 *  main-nearly.c   (nearly-serial driver)
 *
 *  Usage:
 *      ./program <input_file> <filter_file> <output_file>
 *
 *  Reads:
 *      input_file   ──  b × m × n  image batch
 *      filter_file  ──  k × k      filter (header may be "k" or "k k")
 *  Runs:
 *      stencil()    ──  OpenMP 2-D box-stencil
 *  Writes:
 *      output_file  ──  header "b m n", followed by flattened output
 * ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* ------------ prototypes from file-reader.c ------------ */
int read_num_dims(char *filename);
int *read_dims(char *filename, int num_dims);
float *read_array(char *filename, int *dims, int num_dims);
void *write_to_output_file(char *filename, float *output,
                           int *dims, int num_dims);
int product(int *dims, int num_dims);

/* ------------ prototype of the OpenMP stencil ----------- */
void stencil(float *input_vec, float *output_vec,
             float *filter_vec, int m, int n, int k, int b);

/* ======================================================== */
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr,
                "Usage: %s <input_file> <filter_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *input_file = argv[1];
    char *filter_file = argv[2];
    char *output_file = argv[3];

    /* ---------- read input (b, m, n) ---------- */
    int ndims_in = read_num_dims(input_file);
    if (ndims_in != 3)
    {
        fprintf(stderr, "Error: input file must have 3 dims (b m n)\n");
        return EXIT_FAILURE;
    }
    int *dims_in = read_dims(input_file, ndims_in); /* [b, m, n] */
    if (!dims_in)
        return EXIT_FAILURE;

    int b = dims_in[0];
    int m = dims_in[1];
    int n = dims_in[2];

    float *input = read_array(input_file, dims_in, ndims_in);
    if (!input)
    {
        free(dims_in);
        return EXIT_FAILURE;
    }

    /* ---------- read filter (k × k) ---------- */
    int ndims_f = read_num_dims(filter_file);
    if (ndims_f != 1 && ndims_f != 2)
    {
        fprintf(stderr,
                "Error: filter header must have 1 or 2 dims (k or k k)\n");
        free(dims_in);
        free(input);
        return EXIT_FAILURE;
    }
    int *dims_f = read_dims(filter_file, ndims_f);
    if (!dims_f)
    {
        free(dims_in);
        free(input);
        return EXIT_FAILURE;
    }

    int k = dims_f[0];
    if (ndims_f == 2 && dims_f[1] != k)
    {
        fprintf(stderr, "Error: filter dims must form a square (k k)\n");
        free(dims_in);
        free(dims_f);
        free(input);
        return EXIT_FAILURE;
    }

    float *filter = read_array(filter_file, dims_f, ndims_f);
    if (!filter)
    {
        free(dims_in);
        free(dims_f);
        free(input);
        return EXIT_FAILURE;
    }

    /* ---------- allocate output ---------- */
    long long total = (long long)b * m * n;
    float *output = (float *)malloc(total * sizeof(float));
    if (!output)
    {
        fprintf(stderr, "Error: unable to allocate output array\n");
        free(dims_in);
        free(dims_f);
        free(input);
        free(filter);
        return EXIT_FAILURE;
    }

    /* ---------- compute ---------- */
    /* ============ BEGIN TIMING ============ */
    double t0 = omp_get_wtime();
    stencil(input, output, filter, m, n, k, b);
    double t1 = omp_get_wtime();
    printf("STENCIL_TIME: %f\n", t1 - t0);
    /* ============ END TIMING ============ */

    /* ---------- write result ---------- */
    int dims_out[3] = {b, m, n}; /* only 3 dims in header */
    if (!write_to_output_file(output_file, output, dims_out, 3))
    {
        fprintf(stderr, "Error: failed to write output file\n");
        /* continue to cleanup */
    }

    /* ---------- cleanup ---------- */
    free(dims_in);
    free(dims_f);
    free(input);
    free(filter);
    free(output);
    return EXIT_SUCCESS;
}
