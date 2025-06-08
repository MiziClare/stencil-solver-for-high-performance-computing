/* ------------------------------------------------------------
 *  main-mpi.c   –  Distributed driver using MPI + OpenMP
 *
 *  Usage:
 *      mpirun -np <P> ./program <input_file> <filter_file> <output_file>
 *
 *  Strategy:
 *      1. Rank-0 reads input (b, m, n) and filter (k × k).
 *      2. Broadcast dims (b, m, n, k) and filter to all ranks.
 *      3. Scatter input in batches across ranks.
 *      4. Each rank revokes stencil on its local input.
 *      5. Gather all local outputs on rank-0 using MPI_Gatherv.
 *      6. Rank-0 writes output (b, m, n) to file.
 *
 * ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* --------------- prototypes from file-reader.c --------------- */
int read_num_dims(char *filename);
int *read_dims(char *filename, int num_dims);
float *read_array(char *filename, int *dims, int num_dims);
void *write_to_output_file(char *filename, float *out,
                           int *dims, int num_dims);

/* --------------- prototype of our OpenMP stencil ------------- */
void stencil(float *input_vec, float *output_vec,
             float *filter_vec, int m, int n, int k, int b);

/* ============================================================ */
static void compute_counts(int b, int m, int n,
                           int size, int *counts, int *displs,
                           int rank, int *local_b)
{
    int q = b / size; /* base share */
    int r = b % size; /* first r ranks get +1 batch */

    int offset = 0;
    for (int p = 0; p < size; ++p)
    {
        int batches = q + (p < r ? 1 : 0);
        counts[p] = batches * m * n; /* element count per rank */
        displs[p] = offset;
        offset += counts[p];
        if (p == rank)
            *local_b = batches;
    }
}

/* ============================================================ */
int main(int argc, char *argv[])
{
    /* ---------- MPI init ---------- */
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4)
    {
        if (rank == 0)
            fprintf(stderr,
                    "Usage: %s <input_file> <filter_file> <output_file>\n", argv[0]);
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    char *input_file = argv[1];
    char *filter_file = argv[2];
    char *output_file = argv[3];

    /* ---------- variables shared by all ranks ---------- */
    int b = 0, m = 0, n = 0, k = 0;
    float *input_all = NULL;  /* only rank-0 owns full input */
    float *filter = NULL;     /* all ranks need filter later */
    float *output_all = NULL; /* only rank-0 owns full output */

    /* ---------- rank-0 reads files ---------- */
    if (rank == 0)
    {
        /* ---- read input image ---- */
        int nd_in = read_num_dims(input_file);
        if (nd_in != 3)
        {
            fprintf(stderr, "Error: input must have 3 dims (b m n)\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int *dims_in = read_dims(input_file, nd_in); /* [b,m,n] */
        b = dims_in[0];
        m = dims_in[1];
        n = dims_in[2];
        input_all = read_array(input_file, dims_in, nd_in);
        free(dims_in);

        /* ---- read filter ---- */
        int nd_f = read_num_dims(filter_file);
        if (nd_f != 1 && nd_f != 2)
        {
            fprintf(stderr, "Error: filter header must be 1 or 2 ints\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int *dims_f = read_dims(filter_file, nd_f); /* [k] or [k,k] */
        k = dims_f[0];
        if (nd_f == 2 && dims_f[1] != k)
        {
            fprintf(stderr, "Error: filter dims must form square (k k)\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        filter = read_array(filter_file, dims_f, nd_f);
        free(dims_f);

        /* ---- allocate space for gathered output ---- */
        output_all = (float *)malloc((long long)b * m * n * sizeof(float));
        if (!output_all)
        {
            fprintf(stderr, "Error: cannot allocate output array\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    /* ---------- broadcast scalar dims ---------- */
    int dims4[4];
    if (rank == 0)
    {
        dims4[0] = b;
        dims4[1] = m;
        dims4[2] = n;
        dims4[3] = k;
    }

    /* ================== BEGIN TIMING ================== */
    MPI_Barrier(MPI_COMM_WORLD); /* align starting point */
    double t0 = MPI_Wtime();

    MPI_Bcast(dims4, 4, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
    {
        b = dims4[0];
        m = dims4[1];
        n = dims4[2];
        k = dims4[3];
    }

    /* ---------- broadcast filter ---------- */
    int f_elems = k * k;
    if (rank != 0)
        filter = (float *)malloc((long long)f_elems * sizeof(float));

    MPI_Bcast(filter, f_elems, MPI_FLOAT, 0, MPI_COMM_WORLD);

    /* ---------- compute scatter/gather metadata ---------- */
    int *counts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    int local_b = 0;
    compute_counts(b, m, n, size, counts, displs, rank, &local_b);

    /* ---------- local buffers ---------- */
    float *local_in = NULL;
    float *local_out = NULL;
    if (counts[rank] > 0)
    {
        local_in = (float *)malloc((long long)counts[rank] * sizeof(float));
        local_out = (float *)malloc((long long)counts[rank] * sizeof(float));
    }

    /* ---------- scatter input ---------- */
    MPI_Scatterv(input_all, counts, displs, MPI_FLOAT,
                 local_in, counts[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);

    /* ---------- local computation ---------- */
    if (local_b > 0)
        stencil(local_in, local_out, filter, m, n, k, local_b);

    /* ---------- gather output ---------- */
    MPI_Gatherv(local_out, counts[rank], MPI_FLOAT,
                output_all, counts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD); /* make sure all ranks are done */
    double t1 = MPI_Wtime();

    /* ---------- rank-0 writes output file ---------- */
    if (rank == 0)
    {
        printf("STENCIL_TIME: %f\n", t1 - t0);
        fflush(stdout);
        int dims_out[3] = {b, m, n}; /* header: b m n */
        if (!write_to_output_file(output_file, output_all, dims_out, 3))
            fprintf(stderr, "Error: failed to write output file\n");
    }
    /* ================ END TIMING ================== */

    /* ---------- cleanup ---------- */
    if (rank == 0)
    {
        free(input_all);
        free(output_all);
    }
    free(filter);
    free(local_in);
    free(local_out);
    free(counts);
    free(displs);

    MPI_Finalize();
    return 0;
}
