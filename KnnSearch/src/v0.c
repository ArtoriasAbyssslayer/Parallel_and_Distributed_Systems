#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cblas.h>
#include <math.h>
#include <sys/time.h>

//definition of RANDMAX as unsigned "ifnotdefined" 

#ifndef RAND_MAX
#define RAND_MAX ((int)((unsigned)~0 >> 1))
#endif

//row major format
//Definition of the kNN result struct
typedef struct knnresult
{
    int *nidx;     //!< Indices (0-based) of nearest neighbors [m-by-k]
    double *ndist; //!< Distance of nearest neighbors          [m-by-k]
    int m;         //!< Number of query points                 [scalar]
    int k;         //!< Number of nearest neighbors            [scalar]
} knnresult;
//! Compute k nearest neighbors of each point in X [n-by-d]
/*!

  \param  X      Corpus data points              [n-by-d]
  \param  Y      Query data points               [m-by-d]
  \param  n      Number of corpus points         [scalar]
  \param  m      Number of query points          [scalar]
  \param  d      Number of dimensions            [scalar]
  \param  k      Number of neighbors             [scalar]

  \return  The kNN result
*/
//main kNN  function returning knnresult
knnresult kNN(double *X, double *Y, int n, int m, int d, int k){
    /*initialize the returning struct*/
    knnresult result;
    result.nidx = malloc(m * k * sizeof(int));
    result.ndist = malloc(m * d * sizeof(double));
    
    /*initialize some matrices for calculating the euclidean distance*/

    double *xx = malloc( n * d * sizeof(double)); 
    double *yy = malloc ( m * d * sizeof(double));
    double *x_sum = malloc(n * sizeof(double));
    double *y_sum = malloc(m * sizeof(double));
    double *distances = malloc( m * n * sizeof(double));

    /*Sum (X.^2,2)*/
    for (int i = 0; i < n*d; i++)
    {
        xx[i] = X[i]*X[i];
    }
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < d; j++)
        {
            //row major format store 
            x_sum[i] += xx[d*i + j];
        }
        
    }

    /* calculate the sum(Y.^2,2) not transposed */
    for (int i = 0; i < m * d; i++)
    {
        yy[i] = Y[i] * Y[i];
    }

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < d; j++)
        {
            y_sum[i] = yy[d*i + j];
        }
        
    }
    
    /* - 2*X*Y' */
    /* use cblas_dgemm function to calculate the multiplication*/
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, n, m, d, -2, X, d, Y, d, 0, distances, m);
    double sum = 0;
    /* (sum(X.^2,2) - 2 * X*Y.' + sum(Y.^2,2).')  */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            distances[j + m*i] += x_sum[i] + y_sum[i];
            distances[j + m * i] = sqrt(distances[j + m * i]);
            sum += distances[j + m*i];
        }

    }
    //debug print
    printf("%f\n", sum);
    for (int i = 0; i < m * n; i++)     
    {
        printf("d[%d] : %f", i, distances[i]);
    }
    
    /*
        Distances or D matrix is ready in row major format
        Next i search each column for kNN using quick_select
     */

    for (int i = 0; i < m; i++)
    {
        for (int w = 0; w < k; w++)
        {
            double min = INFINITY;
            int index = -1;  
            for (int j = 0; j < n; j++)
            {
                if(min > distances[j * m + i]){
                    min = distances[j * m + i];
                    index = j * m + i;
                }
            }
            result.ndist[w*m + i] = min;  // set min distance in the data structure array
            result.nidx[w*m + i] = index; // set indices of nearest neighbours 
            distances[index] = INFINITY; // set new min = INFINITY;
        }
    }
    //another implementation with quickselect
    // for (int i = 0; i < k; i++)
    // {
    //     result.ndist[i] = kthSmallest(distances);
    // }
    

   return result;
}
//helper function for generating a random double bounded
double randomBounded(double lower_limit, double upper_limit){
    double r = 0;
    //r ~ rand/RANDMAX
    r = (double)rand() / ((double)RAND_MAX + 1);
    return (lower_limit + r*(upper_limit-lower_limit));
}

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

int main(int argc, char **argv)
{
    //todo get the n,d,k,m from argv
    printf("Insert n,m,d,k with breaks!\n");
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int d = atoi(argv[3]);
    int k = atoi(argv[4]);
    //print the above values
    printf("n: %d\n", n);
    printf("d: %d\n", d);
    printf("k: %d\n", k);
    printf("m: %d\n", m);
    printf("Processes: 1\n");
    
    knnresult knnresult;

    //malloc nidx , ndist of the struct
    knnresult.nidx = malloc(m * k * sizeof(int));
    knnresult.ndist = malloc(m * k * sizeof(double));
   
    knnresult.m = m;
    knnresult.k = k;

    //todo initialize random matrices X, Y malloc them first n*d*sizeofint, m*d
    
    srand(time(NULL)); //PUT THE SEED

    double *X = malloc(n * d * sizeof(double));
    double *Y = malloc(m * d * sizeof(double));

    //generate the arrays using randomBounded
    for (int i = 0; i < n * d; i++)
    {
        X[i] = randomBounded(10,100);
    }

    for (int i = 0; i < m * d; i++)
    {
        Y[i] = randomBounded(10,100);
    }
    
    clock_t begin = clock();

    knnresult = kNN(X, Y, n, m, d, k);
    
    clock_t end  = clock();
    double duration = (double)(end-begin) / CLOCKS_PER_SEC;
    printf("Duration is : %f\n", duration);
    return EXIT_SUCCESS;
}