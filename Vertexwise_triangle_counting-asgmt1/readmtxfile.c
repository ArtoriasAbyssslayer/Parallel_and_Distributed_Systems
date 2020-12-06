#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "mmio.h"

void print1dMatrix(int* array, int size){
    for(unsigned int i = 0; i < size; i++){
        printf("%d: %d \n",i, array[i]);
    }
}

void readfile_chk(FILE *f, uint32_t *m, uint32_t *n, uint32_t *nnz, int *i, int *j){

    int ret_code;
    MM_typecode matcode;
    if(mm_read_banner(f, &matcode) != 0){
        printf("Error processing Matrix Market banner.\n");
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

    if ((ret_code = mm_read_mtx_crd_size(f, m, n, nnz)) !=0)
        exit(1);
}

void readfile_scan(FILE *f, uint32_t *m, uint32_t *n, uint32_t *nnz_pt, int *I, int *J, double *val){
    uint32_t M = *m;
    uint32_t N = *n;
    uint32_t nnz = *nnz_pt;

    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    /* Replace missing val column with 1s and change the fscanf to match patter matrices*/

    //read the binary file of the adjacency matrix 
    for(uint32_t i=0; i<nnz; i++){
            /* I is for the rows and J for the columns */
            fscanf(f, "%d %d \n", &I[i], &J[i]);
            I[i]--;  /* adjust from 1-based to 0-based */
            J[i]--;
    }

    if (f !=stdin) 
        fclose(f);

    if(M != N) {
        printf("COO matrix' columns and rows are not the same");
    }


}