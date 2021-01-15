#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include <cblas.h>
#include <math.h>
#include <sys/time.h>

#ifndef RAND_MAX
#define RAND_MAX ((int) ((unsigned) ~0 >> 1))
#endif

// Definition of the kNN result struct
typedef struct knnresult{
  int    * nidx;    //!< Indices (0-based) of nearest neighbors [m-by-k]
  double * ndist;   //!< Distance of nearest neighbors          [m-by-k]
  int      m;       //!< Number of query points                 [scalar]
  int      k;       //!< Number of nearest neighbors            [scalar]
} knnresult;

knnresult kNN(double * X, double * Y, int n, int m, int d, int k){

    knnresult knn_result;
    knn_result.nidx = malloc(m * k * sizeof(int));
    knn_result.ndist = malloc(m * k * sizeof(double));

    double *xx = malloc(n * d * sizeof(double));
    double *yy = malloc(m * d * sizeof(double));
    double *x_sum = malloc(n * sizeof(double));
    double *y_sum = malloc(m * sizeof(double));
    double *distances = malloc(n * m * sizeof(double));

    /* sum(X.^2,2) */
    for(int i = 0; i < n * d; i++) {
        xx[i] = X[i] * X[i];
    }

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < d; j++) {
            x_sum[i] += xx[d*i + j];
        }
    }

    /* sum(Y.^2,2) (not transposed) */
    for(int i = 0; i < m * d; i++) {
        yy[i] = Y[i] * Y[i];
    }

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < d; j++) {
            y_sum[i] += yy[d*i + j];
        }
    }
    /* -2 X Y' */
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, n, m, d, -2, X, d, Y, d, 0, distances, m);
    
    double sum = 0;
    /* (sum(X.^2,2) - 2 * X*Y.' + sum(Y.^2,2).') */
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++) {
            distances[j + m*i] += x_sum[i] + y_sum[j];
            distances[j + m*i] = sqrt(distances[j + m*i]);
            sum += distances[j + m*i];
        }
    }
    // printf("%f\n", sum);

    // We have in our hands the D matrix in row major format
    // Next we have to search each column for the kNN    
    for(int i = 0; i < m; i++) {
        for(int w = 0; w < k; w++) {
            double min = INFINITY; 
            int index = -1;
            for(int j = 0; j < n; j++) {
                if(min > distances[j * m + i]) {
                    min = distances[j * m + i];
                    index = j * m + i;
                }
            }
            knn_result.ndist[w*m + i] = min;
            knn_result.nidx[w*m + i] = index;
            distances[index] =  INFINITY;
        }        
    }

    return knn_result;
};
/*------------QUICKSELECT ALGORITHM-----------*/
void swap(double x, double y){
    double temp = x;
    x = y;
    y = temp;
}
int partition(double arr[], int l , int r){
    double x = arr[r];
    int i = l;
    for (int j = l; j <= r - 1; j++)
    {
        if (arr[j] <= x)
        {
            swap(arr[i], arr[j]);
            i++;
        }
    }
    swap(arr[i], arr[r]);
    return i;
}
// This function returns k'th smallest
// element in arr[l..r] using QuickSort
// based method.  ASSUMPTION: ALL ELEMENTS
// IN ARR[] ARE DISTINCT
int kthSmallest(double arr[], int l, int r, int k)
{
    // If k is smaller than number of
    // elements in array
    if (k > 0 && k <= r - l + 1)
    {

        // Partition the array around last
        // element and get position of pivot
        // element in sorted array
        int index = partition(arr, l, r);

        // If position is same as k
        if (index - l == k - 1)
            return arr[index];

        // If position is more, recur
        // for left subarray
        if (index - l > k - 1)
            return kthSmallest(arr, l, index - 1, k);

        // Else recur for right subarray
        return kthSmallest(arr, index + 1, r,
                           k - index + l - 1);
    }

    // If k is more than number of
    // elements in array
    return TMP_MAX;
}
/*------------QUICKSELECT ALGORITHM-----------*/
double randomBounded(double low, double high) {
    double r;

    r = (double) rand() / ((double) RAND_MAX + 1);
    return (low + r * (high - low));
}

int main(int argc, char **argv) {
    // argv[1] = n
    // argv[2] = d
    // argv[3] = k
    // argv[4] = m
    int n = atoi(argv[1]);
    int d = atoi(argv[2]);
    int m = atoi(argv[4]);
    int k = atoi(argv[3]);
    printf("n: %d\n", n);
    printf("d: %d\n", d);
    printf("k: %d\n", k);
    printf("m: %d\n", m);
    printf("Processes: 1\n");
    knnresult knnresult;
    knnresult.nidx = malloc(m * k * sizeof(int));
    knnresult.ndist = malloc(m * k * sizeof(int));
    knnresult.m = m;
    knnresult.k = k;

    srand(time(NULL));

    double* X = malloc(n * d * sizeof(double));

    double* Y = malloc(m * d * sizeof(double));

    // Create an X array n x d
    for(int i = 0; i < n * d; i++) {
        X[i] = randomBounded(0, 100);
        // X[i] = 1;
    }

    // Create a Υ array m x d
    for(int i = 0; i < m * d; i++) {
        Y[i] = randomBounded(0, 100);
        // Y[i] = 1;
    }

    // Start measuring time
    clock_t start = clock();

    knnresult =  kNN(X, Y, n, m, d, k);  

    // Stop measuring time  
    clock_t end = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Duration: %f\n", duration);
    return 0;
}
