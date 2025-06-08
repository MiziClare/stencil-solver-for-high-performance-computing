#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

/*This code is for reading and writing to files for the 2024-2025 COMP328 CA*/

/*Use the functions in this file to read from the input file, and write to the output file*/

/*You should use this file when compiling your code*/

/*Declare these functions at the top of each 'main' file*/

/*If there are any issues with this code, please contact: h.j.forbes@liverpool.ac.uk*/

int read_num_dims(char *filename);
int *read_dims(char *filename, int num_dims);
float *read_array(char *filename, int *dims, int num_dims);
void *write_to_output_file(char *filename, float *output, int *dims, int num_dims);
int product(int *dims, int num_dims);

/*Reads the number of dimensions from a file*/
int read_num_dims(char *filename)
{

    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("Unable to open file: %s\n", filename);
        return -1; //-1 means error
    }

    char firstline[500];

    if (fgets(firstline, 500, file) == NULL)
    {
        printf("Error reading file %s\n", filename);
        return -1; //-1 means error
    }

    int line_length = strlen(firstline);
    int num_dims = 0;

    int i;
    for (i = 0; i < line_length; i++)
    {
        if (firstline[i] == ' ')
        {
            num_dims++;
        }
    }

    fclose(file);
    return num_dims;
}

/*Reads the dimensiosn themselves from a file*/
int *read_dims(char *filename, int num_dims)
{

    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("Unable to open file: %s\n", filename);
        return NULL;
    }

    char firstline[500];

    if (fgets(firstline, 500, file) == NULL)
    {
        printf("Error reading file %s\n", filename);
        return NULL;
    }

    const char s[2] = " ";
    char *token;
    token = strtok(firstline, s);

    int *dims = malloc((num_dims) * sizeof(int));

    if (dims == NULL)
    {
        printf("Error allocating memory for dimension array\n");
        return NULL;
    }

    int i = 0;
    while (token != NULL)
    {
        dims[i] = atoi(token);
        i++;
        token = strtok(NULL, s);
    }

    fclose(file);
    return dims;
}

/*Gets the data from the file. Returns as an array of floats. Ignores the first line*/
float *read_array(char *filename, int *dims, int num_dims)
{

    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("Unable to open file: %s\n", filename);
        return NULL; // error
    }

    char firstline[500];

    if (fgets(firstline, 500, file) == NULL)
    {
        printf("Error reading file\n");
        return NULL; // error
    }

    int total_elements = product(dims, num_dims);
    float *data = malloc(total_elements * sizeof(float));

    if (data == NULL)
    {
        printf("Error allocating memory for data from %s\n", filename);
        return NULL;
    }

    int i;
    for (i = 0; i < total_elements; i++)
    {
        if (fscanf(file, "%f", &data[i]) == EOF)
        {
            printf("Error reading file");
            return NULL; // error
        }
    }

    fclose(file);
    return data;
}

/*Writes to the output file*/
void *write_to_output_file(char *filename, float *output, int *dims, int num_dims)
{

    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        printf("Unable to open file %s\n", filename);
        return NULL;
    }

    printf("\nFile opened, writing dims\n");

    int i;
    for (i = 0; i < num_dims; i++)
    {
        fprintf(file, "%d ", dims[i]);
    }
    fprintf(file, "\n");

    int total_elements = product(dims, num_dims);

    printf("\nWriting output data\n");
    for (i = 0; i < total_elements; i++)
    {
        fprintf(file, "%.7lf ", output[i]);
    }

    fclose(file);
    return output;
}

/*Used for returning the number of elements given a 1 dimensional array of dimensions*/
int product(int *dims, int num_dims)
{

    int product = 1;
    int i;

    for (i = 0; i < num_dims; i++)
    {
        product *= dims[i];
    }

    return product;
}
