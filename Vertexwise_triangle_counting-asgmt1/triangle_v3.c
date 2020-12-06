#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "mmio.h"
#include "coo2csc.h"
#include "readmtxfile.h"
#include <stdbool.h>


#define false 0
#define true 1

int main(int argc, char* argv[]){
    FILE *f;
    uint32_t M, N, nnz;
    int i, *I, *J;
    double *val;
  //for time measuring
    struct timeval start, end;
    if (argc < 2)
	{
		fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
		exit(1);
	}
    else
    {
        if ((f = fopen(argv[1], "r")) == NULL) 
            exit(1);
    }
    readfile_chk(f, &M, &N, &nnz, I, J);
    /*Allocate memory for the COO*/
    I = (uint32_t *) malloc(nnz * sizeof(uint32_t));
    J = (uint32_t *) malloc(nnz * sizeof(uint32_t));
    val = (double *) malloc(nnz * sizeof(double));
    if(I == NULL ||  J == NULL || val == NULL){
        fprintf(stderr,"%s","Problem while allocating memory");
    }

    readfile_scan(f, &M, &N, &nnz, I, J, val);


    printf("Matrix read!\n");
    /*allocate memory for the CSC */

    uint32_t* CSCRow = (uint32_t*) malloc (nnz * sizeof(uint32_t));
    uint32_t* CSCColumn = (uint32_t *) malloc((N + 1) * sizeof(uint32_t));
    if(CSCRow == NULL || CSCColumn == NULL){
        fprintf(stderr,"%s","Problem while allocating memory");
    }

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

    //start measuring time of the algorithm
    gettimeofday(&start,NULL);
    /*
    V3:
      for i = 1:n-2
        for j = adjacent to i % data structure only lists A(i,j)~=0
          for k = adjacent to j % data structure only lists A(j,k)~=0
    */
    /* START OF THE TRIANGLE COUNTING ALGORITHM*/
    int triangle_sum = 0;
    
    for (int i = 1; i < N; i++){
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
                  if(row2 = col1){
                    triangle_sum++;
                    c3[col1]++;
                    c3[col2]++;
                    c3[row3]++;
                  }

              }
          }
          else//uppertriag
          {
           //loop the entire col1 column
            for (int l = 0; l < CSCColumn[col1+1] - CSCColumn[col1]; l++) {
              int row2 = CSCRow[CSCColumn[col1] + l];
              if(row2 == row3) {
                triangle_sum++;
                c3[col1]++;
                c3[row3]++;
                c3[col2]++;
              }
            }
          }
            
         }

      }
   
      
    }
    

    /* END OF THE TRIANGLE COUNTING ALGORITHM*/
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