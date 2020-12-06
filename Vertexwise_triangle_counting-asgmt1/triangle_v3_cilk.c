#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#include "mmio.h"
#include "coo2csc.h"
#include <stdbool.h>
#include "cilk/cilk.h"
#include <pthread.h>
#include <cilk/cilk_api.h>

#define false 0
#define true 1


void printarray(int* array, int size){
    int i = 0;
    for(i = 0; i < size; i++){
        printf("%d: %d \n",i, array[i]);
    }
}

int main(int argc, char* argv[]){
    int ret_code;
    MM_typecode matcode;
    FILE *f;
    uint32_t M, N, nnz;   
    int *I, *J;
    double *val;
    int isBinary = atoi(argv[2]);
    int num_of_threads = atoi(argv[3]);

    char* num_of_threads_string = argv[3];
    
    struct timeval start,end;

    if (argc < 2){
		fprintf(stderr, "Usage: %s [martix-market-filename] [0 for non binary 1 for binary matrix] [num of threads]\n", argv[0]);
		exit(1);
	}
    else{ 
        if ((f = fopen(argv[1], "r")) == NULL) 
            exit(1);
    }

    __cilkrts_set_param("nworkers",num_of_threads_string);
    int numWorkers = __cilkrts_get_nworkers();
    
    printf("The number of workers is %d.\n", numWorkers);

        if (mm_read_banner(f, &matcode) != 0)
    {
        printf("Could not process Matrix Market banner.\n");
        exit(1);
    }


    /*  This is how one can screen matrix types if their application */
    /*  only supports a subset of the Matrix Market data types.      */

    if (mm_is_complex(matcode) && mm_is_matrix(matcode) && 
            mm_is_sparse(matcode) )
    {
        printf("Sorry, this application does not support ");
        printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
        exit(1);
    }

    /* find out size of sparse matrix .... */

    if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nnz)) !=0)
        exit(1);
    

        /* reseve memory for matrices */
    /* For the COO format */
    I = (uint32_t *) malloc(nnz * sizeof(uint32_t));
    J = (uint32_t *) malloc(nnz * sizeof(uint32_t));
    val = (double *) malloc(nnz * sizeof(double));
    /* For the CSC format */
    uint32_t* CSCRow = (uint32_t *) malloc(nnz * sizeof(uint32_t));
    uint32_t* CSCColumn = (uint32_t *) malloc((N + 1) * sizeof(uint32_t));
    if(I == NULL ||  J == NULL || val == NULL){
        fprintf(stderr,"%s","Problem while allocating memory");
    }
    if(CSCRow == NULL || CSCColumn == NULL){
        fprintf(stderr,"%s","Problem while allocating memory");
    }


    /*read file according to if the matrix isBinary or non binary*/

    switch (isBinary)
    {
    case 0:
        //non binary matrix 
        for (uint32_t i=0; i<nnz; i++)
        {
            /* I is for the rows and J for the columns */
            fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
            I[i]--;  /* adjust from 1-based to 0-based */
            J[i]--;
        }
        break;
    case 1:
    /*if the source file is binary use this code*/
        for (uint32_t i=0; i<nnz; i++)
        {
            /* I is for the rows and J for the columns */
            fscanf(f, "%d %d \n", &I[i], &J[i]);
            I[i]--;  /* adjust from 1-based to 0-based */
            J[i]--;
        }
        break;
    default:
        printf("Not valid 'isBinary' identifier argument was passed \n");
        break;
    }

    if (f !=stdin) 
        fclose(f);

    if(M != N) {
        printf("COO matrix' columns and rows are not the same");
    }



      // Make c3 and fill it with zeros
    int* c3;
    c3 = malloc(N*sizeof c3);
    if (c3 == NULL)
    {
        fprintf(stderr,"%s","Problem allocating c3");
    }
    for (unsigned int i = 0; i < N; i++)
    {
      c3[i] = 0;
    }
    
    /* Code to convert  the symetric matrices in upper triangular*/
    /*
      The coo2csc code works for an upper triangular matrix.
      So we change th J,I according to the symmetric table that we have as input (I[O],J[0])
      The switch is done with the help of the flag TRIANG_LOWER
      TRIANG_LOWER = false upper triangular-> I.J || TRIANG_LOWER = true  lower triangular -> J,I
    */
    bool TRIANG_LOWER = false;
    if (I[0] > J[0]){
      TRIANG_LOWER = true;
    }

    if (!TRIANG_LOWER)
    {
      printf("case upper triangular \n");
      coo2csc(CSCRow,CSCColumn,I,J,nnz,M,0);
    }else
    {
      printf("case lower triangular\n");
      coo2csc(CSCRow,CSCColumn,J,I,nnz,N,0);
    }

    printf("Matrix Loaded, Search Starts!\n");

    pthread_mutex_t mutex; //defome the lock
    pthread_mutex_init(&mutex,NULL); //initialize the lock

    gettimeofday(&start,NULL);
        int triangle_sum = 0;
    
    cilk_for(int i = 1; i < N; i++){
       for (int j = 0; j < CSCColumn[i+1]-CSCColumn[i]; j++){
         int row1 = CSCRow[CSCColumn[i] + j];
         int col1 = i;
         for(int k = 0; k < CSCColumn[row1+1] - CSCColumn[row1]; k++){
           int row3 = CSCRow[CSCColumn[row1]+k];
           int col2 = row1;
           /*
           search for the x,y element
           where x = col1, y = row3
           */
          if(row3>col1){ //lowertriang
              //loop the entire row3 column 
              for(int m = 0; m < CSCColumn[row3+1]- CSCColumn[row3]; m++){
                int row2 = CSCRow[CSCColumn[row3] + m];
                  if(row2 == col1){
                    pthread_mutex_lock(&mutex); //lock-prevents other threads from running this code
                    triangle_sum++;
                    c3[col1]++;
                    c3[col2]++;
                    c3[row3]++;
                    pthread_mutex_unlock(&mutex); //give access to other threads after work is done
                  }

              }
          }
          else//uppertriag
          {
           //loop the entire col1 column
            for (int l = 0; l < CSCColumn[col1+1] - CSCColumn[col1]; l++) {
              int row2 = CSCRow[CSCColumn[col1] + l];
              if(row2 == row3) {
                pthread_mutex_lock(&mutex); //lock - prevents other threads from running this code
                triangle_sum++;
                c3[col1]++;
                c3[row3]++;
                c3[col2]++;
                pthread_mutex_unlock(&mutex);
              }
            }
          }
            
         }

      }
   
      
    }
    //stop measuring time
    gettimeofday(&end,NULL);
    double duration = (end.tv_sec + (double)end.tv_usec/1000000) - (start.tv_sec + (double)start.tv_usec/1000000); 
    printf("Total Triangles: %d \n",triangle_sum);
    printf("Duration: %f \n", duration);

    //Deallocate the arrays
    free(I);
    free(J);
    free(CSCColumn);
    free(CSCRow);
    free(c3);
    free(val);
    
    return 0;
}