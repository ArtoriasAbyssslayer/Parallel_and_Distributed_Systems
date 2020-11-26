#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000L;



int main(int argc, char* argv[]){
    int nodes;
    printf("Insert the number of Graph's nodes :\n");
    scanf("%d",&nodes);
    printf("The number of nodes is set to: %d\n", nodes);

    /*
        Graph's adjacency matrix creation
    */
   int **adjacencyMatrix = (int **)malloc(nodes*sizeof (*adjacencyMatrix));
   for (unsigned int i = 0; i < nodes; i++)
   {
       adjacencyMatrix[i] = malloc(nodes*sizeof (*adjacencyMatrix[i]));
   }
   
   srand(time(NULL));
    // Initialize the adjacency Matrix 
    for(unsigned int i = 0; i < nodes; i++){
       
        for(unsigned int j = i; j < nodes; j++){
            if(i == j) {
                //the cells in the diagonal should be zero
               adjacencyMatrix[i][j] = 0; 
            }
            else{
                // Inialize the upper half of the adjacency matrix 
                adjacencyMatrix[i][j] = random() % 2;     // random int between 0 and 1 */
                adjacencyMatrix[j][i] = adjacencyMatrix[i][j];      // adjacency Matrix is symetrical 
            }
            
        }
    }
    
    //------------------------------------------------------------------------
    int* c3;
    c3 = malloc(nodes*sizeof c3);
    for (unsigned int i = 0; i < nodes; i++)
    {
        c3[i] = 0;

    }

     for (unsigned int i = 0; i <  nodes; i++) {
         for (unsigned int j = 0; j < nodes; j++){
             printf("\t%d ", adjacencyMatrix[i][j]);
         } 
            
         printf("\n");
     }
       
    



    //Deallocate memory
    for(unsigned int i = 0; i < nodes; i++){
        free(adjacencyMatrix[i]);
    }
    free(adjacencyMatrix);
    free(c3);


   

    return 0;
}