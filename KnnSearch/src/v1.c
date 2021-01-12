#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cblas.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <sys/time.h>
#include "./read_X.h"
#ifndef RAND_MAX
#define RAND_MAX ((int)((unsigned)~0 >> 1))
#endif

// Definition of the kNN result struct
typedef struct knnresult
{
    int *nidx;     //!< Indices (0-based) of nearest neighbors [m-by-k]
    double *ndist; //!< Distance of nearest neighbors          [m-by-k]
    int n;         //!< Number of query points                 [scalar]
    int k;         //!< Number of nearest neighbors            [scalar]
} knnresult;

knnresult kNN(double *X, double *Y, int n, int m, int d, int k, int rank)
{

    knnresult knn_result;
    knn_result.nidx = malloc(m * k * sizeof(int));
    knn_result.ndist = malloc(m * k * sizeof(double));

    double *x_squared = malloc(n * d * sizeof(double));
    double *y_squared = malloc(m * d * sizeof(double));
    double *x_sum = malloc(n * sizeof(double));
    double *y_sum = malloc(m * sizeof(double));
    double *d_matrix = malloc(n * m * sizeof(double));

    /* sum(X.^2,2) */
    for (int i = 0; i < n * d; i++)
    {
        x_squared[i] = X[i] * X[i];
    }

    // printf("x_sum\n");
    for (int i = 0; i < n; i++)
    {
        double temp = 0;
        for (int j = 0; j < d; j++)
        {
            temp += x_squared[d * i + j];
        }
        x_sum[i] = temp;
    }

    /* sum(Y.^2,2) (not transposed) */
    for (int i = 0; i < m * d; i++)
    {
        y_squared[i] = Y[i] * Y[i];
    }

    // printf("y_sum\n");
    for (int i = 0; i < m; i++)
    {
        double temp = 0;
        for (int j = 0; j < d; j++)
        {
            temp += y_squared[d * i + j];
        }
        y_sum[i] = temp;
    }

    /* -2 X Y' */
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, n, m, d, -2, X, d, Y, d, 0, d_matrix, m);

    double sum = 0;
    /* (sum(X.^2,2) - 2 * X*Y.' + sum(Y.^2,2).') */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            d_matrix[j + m * i] += x_sum[i] + y_sum[j];
            d_matrix[j + m * i] = sqrt(d_matrix[j + m * i]);
            sum += d_matrix[j + m * i];
        }
    }

    // We have in our hands the D matrix in row major format
    // Next we have to search each column for the kNN
    for (int i = 0; i < m; i++)
    {
        for (int w = 0; w < k; w++)
        {
            double min = INFINITY;
            int index = -1;
            for (int j = 0; j < n; j++)
            {
                if (min > d_matrix[j * m + i])
                {
                    min = d_matrix[j * m + i];
                    index = j * m + i;
                }
            }
            knn_result.ndist[w * m + i] = min;
            knn_result.nidx[w * m + i] = index;
            d_matrix[index] = INFINITY;
        }
    }
    return knn_result;
}
//helper function for generating a random double bounded
// double randomBounded(double lower_limit, double upper_limit)
// {
//     double r = 0;
//     //r ~ rand/RANDMAX
//     r = (double)rand() / ((double)RAND_MAX + 1);
//     return (lower_limit + r * (upper_limit - lower_limit));
// }

void main(int argc, char **argv)
{
    // MPI
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int p;
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    //argv[1] = path to matrix argv[2]= k
    int n ;
    int d ;
    int k = atoi(argv[2]);

    clock_t begin;
    clock_t end;

    int chunks ;
    double *x_i_data;
    double *y_i_send_data;
    double *y_i_receive_data;
    int process_n;
    int process_m;
    int flag = -1;
    int knn_counter = 0;

    knnresult sub_knnresult;

    if (world_rank == 0)
    {
        //srand(time(NULL));

        // printf("n: %d\n", n);
        // printf("d: %d\n", d);
        // printf("k: %d\n", k);
        // printf("Processes: %d\n", p);

        double *x_data;

        x_data = read_X(&n, &d, argv[1]);
        chunks = n/p;
        // Start measuring time
        begin = clock();

        if (p > 1)
        {
            //Broadcast to all other processes
            for (int i = 1; i < p - 1; i++)
            {
                // MPI_Send to each process the appropriate array
                MPI_Send(&x_data[i * chunks * d], chunks * d, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            }
            MPI_Send(&x_data[(p - 1) * chunks * d], (chunks + n % p) * d, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD);
        }

        // After sending the message and receiving confirmation we are ready to call kNN on our data
        //Initialize variables for zero process
        process_n = chunks;
        process_m = chunks;
        x_i_data = malloc(process_m * d * sizeof(double));
        x_i_data = x_data;

        sub_knnresult.k = k;
        sub_knnresult.n = process_m;
        sub_knnresult.ndist = malloc(process_m * k * sizeof(double));
        sub_knnresult.nidx = malloc(process_m * k * sizeof(int));
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&d, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (world_rank == p - 1)
    {
        chunks = n / p;
        //Initialize variables for this process
        process_n = (chunks + n % p);
        process_m = (chunks + n % p);
        x_i_data = malloc(process_m * d * sizeof(double));

        // First receive from the mother process
        MPI_Recv(x_i_data, process_m * d, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        sub_knnresult.k = k;
        sub_knnresult.n = process_m;
        sub_knnresult.ndist = malloc(process_m * k * sizeof(double));
        sub_knnresult.nidx = malloc(process_m * k * sizeof(int));
    }
    else
    {
        chunks = n / p;
        //Initialize variables for this process
        process_n = chunks;
        process_m = chunks;
        x_i_data = malloc(process_m * d * sizeof(double));

        // First receive from the mother process
        MPI_Recv(x_i_data, process_m * d, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        sub_knnresult.k = k;
        sub_knnresult.n = process_m;
        sub_knnresult.ndist = malloc(process_m * k * sizeof(double));
        sub_knnresult.nidx = malloc(process_m * k * sizeof(int));
    }

    // Every process has processed its own x_i_data, now they have to pass around data to each other in a ring mode
    // That has to repeat until each process has processed all possible data aka p times
    chunks = n / p;
    // For the first iteration set the y_i_send_data array that will be passed
    process_n = chunks;
    y_i_send_data = malloc(process_n * d * sizeof(double));
    //REALLY IMPORTANT NOT TO SET THE POINTERS EQUAL HERE
    memcpy(y_i_send_data, x_i_data, process_m * d * sizeof(double));

    for (int i = 0; i < p; i++)
    {
        // Process zero sends first then waits for the last process
        if (world_rank != 0)
        {
            process_n = chunks;
            y_i_receive_data = malloc(process_n * d * sizeof(double));
            MPI_Recv(y_i_receive_data, process_n * d, MPI_DOUBLE, world_rank - 1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Send(y_i_send_data, process_n * d, MPI_DOUBLE, (world_rank + 1) % p, i, MPI_COMM_WORLD);
        if (world_rank != 0)
        {
            // After sending the previous y data that the process holds, in the next iteration, we want to send the new data we just received
            memcpy(y_i_send_data, y_i_receive_data, process_m * d * sizeof(double));
        }
        if (world_rank == 0)
        {
            process_m = chunks;
            y_i_receive_data = malloc(process_n * d * sizeof(double));
            MPI_Recv(y_i_receive_data, process_n * d, MPI_DOUBLE, p - 1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // After sending the previous y data that the process holds, in the next iteration, we want to send the new data we just received
            memcpy(y_i_send_data, y_i_receive_data, process_m * d * sizeof(double));
        }

        knnresult temp_knnresult;
        temp_knnresult.ndist = malloc(process_m * k * sizeof(double));
        temp_knnresult.nidx = malloc(process_m * k * sizeof(int));

        if (!knn_counter)
        {
            // First call of knn so we store the result in the permanent sub_knnresult
            sub_knnresult = kNN(y_i_receive_data, x_i_data, process_n, process_m, d, k, world_rank);
            knn_counter++;
        }
        else
        {
            temp_knnresult = kNN(y_i_receive_data, x_i_data, process_n, process_m, d, k, world_rank);
            // We have the sub_knnresult from the previous comparisons and the temp_knnresult from the last knn operation
            // We have to store in the sub_knnresult the smallest values from both
            for (int i = 0; i < process_m; i++)
            {
                double temp_dist[k];
                int temp_index[k];
                int kappa_one = 0;
                int kappa_two = 0;
                for (int w = 0; w < k; w++)
                {
                    if (temp_knnresult.ndist[kappa_one * process_m + i] < sub_knnresult.ndist[kappa_two * process_m + i])
                    {
                        temp_dist[w] = temp_knnresult.ndist[kappa_one * process_m + i];
                        temp_index[w] = temp_knnresult.nidx[kappa_one * process_m + i];
                        kappa_one++;
                    }
                    else
                    {
                        temp_dist[w] = sub_knnresult.ndist[kappa_two * process_m + i];
                        temp_index[w] = sub_knnresult.nidx[kappa_two * process_m + i];
                        kappa_two++;
                    }
                }
                for (int w = 0; w < k; w++)
                {
                    sub_knnresult.ndist[w * process_m + i] = temp_dist[w];
                    sub_knnresult.nidx[w * process_m + i] = temp_index[w];
                }
            }

            // Don't forget to clear temp_knnresult after this comment
            free(temp_knnresult.ndist);
            free(temp_knnresult.nidx);
        }
    }

    // Send the sub_knn_results to zero process
    if (world_rank != 0)
    {
        MPI_Send(sub_knnresult.ndist, process_m * k, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        knnresult knnResult;
        knnResult.nidx = malloc(n * k * sizeof(int));
        if (knnResult.nidx == NULL)
        {
            printf("error allocating memory\n");
            exit(0);
        }
        knnResult.ndist = malloc(n * k * sizeof(double));
        if (knnResult.ndist == NULL)
        {
            printf("error allocating memory\n");
            exit(0);
        }
        knnResult.n = n;
        knnResult.k = k;
        for (int i = 1; i < p; i++)
        {
            double *temp = malloc(process_m * k * sizeof(double));
            MPI_Recv(temp, process_m * k, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Position the elements in the appropriate position of the knnResult.ndist array
            printf("\n");
            for (int j = 0; j < process_m; j++)
            {
                for (int w = 0; w < k; w++)
                {
                    // printf("%f\n", temp[process_m * w + j]);
                    knnResult.ndist[i * process_m + j + w * n] = temp[process_m * w + j];
                    // printf("%d\n", i * process_m + j + w*n);
                }
            }
        }
        // For i = 0
        for (int j = 0; j < process_m; j++)
        {
            for (int w = 0; w < k; w++)
            {
                knnResult.ndist[j + w * n] = sub_knnresult.ndist[process_m * w + j];
            }
        }

        // printf("FINAL KNN");
        // for(int i = 0; i < n * k ; i++) {
        //     printf("%d %f \n",i, knnResult.ndist[i]);
        // }

        end = clock();
        double duration = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Duration: %f\n", duration);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}