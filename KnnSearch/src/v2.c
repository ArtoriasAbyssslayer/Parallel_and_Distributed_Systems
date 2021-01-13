#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cblas.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <sys/time.h>
#include "./read_X.h"

//for generating random numbers there are some routines that are not used when is use the data given in matrices

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
 
typedef struct minArray{
    int *nidx;
    double *ndist;
} minArray;

/* node struct with struct pointers to children has data,up where  x_i_data goes, mean etc*/
typedef struct node{
    double *data;
    double *up;
    int up_index;
    double mu;
    int left_size;
    int right_size;
    struct node *left;
    struct node *right;
} node;

node *vpTree_create(double *x_i_data, node *root, int m, int d){
    if (m == 0)
    {
        return NULL;
    }
    //initializing the root node 
    root = malloc(sizeof(node));
    root->up = malloc(d * sizeof(double));
    for (int i = 0; i <d; i++)
    {
        root->up[i] = x_i_data[i];
    }


    //Allocate space for left and right children
    int left_size = 0;
    int right_size = 0;

    root->left = malloc(sizeof(node));
    root->right = malloc(sizeof(node));

    root->right->data = malloc(d * right_size * sizeof(double));
    root->left->data = malloc( d * left_size * sizeof(double));

    //calculate the mean of root
    double sum = 0;

    for (int i = 1; i < m; i++)
    {
        double temp = 0;
        for (int j = 0; j < d; j++)
        {
            temp += (x_i_data[j] - x_i_data[i * d + j]) * (x_i_data[j] - x_i_data[i * d + j]);
        }
        temp = sqrt(abs(temp));
        sum += temp;
    }
    
    root->mu = sum /(m-1);

    double val = 0;
    //create left and right node depending on the if
    for (int i = 1; i < m; i++)
    {
        double temp = 0;
        for (int j = 0; j < d; j++)
        {
            temp += (x_i_data[j] - x_i_data[i * d + j]) * (x_i_data[j] - x_i_data[i * d + j]);
        }
        temp = sqrt(abs(temp));
        //create left and right node depending on the if
        if(temp < root->mu){
            left_size++;
            root->left_size = left_size;
            root->left->data = realloc(root->left->data, d*left_size*sizeof(double));
            for (int j = 0; j < d; j++)
            {
                root->left->data[(left_size - 1)*d + j] = x_i_data[i*d + j];
            }
            
        }else
        {
            right_size ++;
            root->right_size = right_size;
            root->right->data = realloc(root->right->data, d * right_size * sizeof(double));
            for (int j = 0; j < d; j++)
            {
                root->right->data[(right_size - 1) * d + j] = x_i_data[i * d + j];
            }
        }

    }

    //create the tree leaves recursively
    root->left = vpTree_create(root->left->data, root->left, left_size, d);
    root->right = vpTree_create(root->right->data, root->right, right_size, d);
    return root;
}


void searchVPT(double *x_query, node *root, int d, int k, minArray* min_arr){
    if (root == NULL)
    {
        //~previous root = leaf
        return;
    }
   
   
    /*Calculate the distance from the root vantage point*/

    double dist = 0;

    for (int j = 0; j < d; j++)
    {
        dist += (root->up[j] - x_query[j]) * (root->up[j] - x_query[j]);
    }
    dist = sqrt(abs(dist));

    for (int i = 0; i < k; i++)
    {
        if (dist < min_arr->ndist[i])   
        {
            for (int j = k-1; j > i; j--)
            {
                //
                min_arr->ndist[j] = min_arr->ndist[j-1];
                min_arr->nidx[j] = min_arr->nidx[j-1];
            }
           // store the dist and up_index in min_arr
            min_arr->ndist[i] = dist;
            min_arr->nidx[i] = root->up_index;
            break;
            
        }
        
    }
    
    //find radius

    double radius = min_arr->ndist[k-1];

    if (dist < root->mu + radius)
    {
        /* Search for the left child */
        searchVPT(x_query,root->left, d, k, min_arr);
    }else if (dist >= root->mu - radius)
    {
        //Search for the right child
        searchVPT(x_query,root->right, d, k, min_arr);
    }

}

//helper function for generating a random double bounded
// double randomBounded(double lower_limit, double upper_limit)
// {
//     double r = 0;
//     //r ~ rand/RANDMAX
//     r = (double)rand() / ((double)RAND_MAX + 1);
//     return (lower_limit + r * (upper_limit - lower_limit));
// }



int main(int argc, char **argv){
    printf("Start");
    //MPI
    //Initialize the MPI invironment

    MPI_Init(&argc, &argv);

    //p number of processes
    int p;
    MPI_Comm_size( MPI_COMM_WORLD , &p);
    printf("Mpi environment set");

    //rank of the process

    int world_rank;
    MPI_Comm_rank( MPI_COMM_WORLD , &world_rank);

    //argv0 == path to matrix
    //argv1 == k
    
    int k = atoi(argv[4]);
    int n;
    int d;
    //initialize the chunks
    int chunks;

    double *x_i_data;
    double *y_i_data;
    double *y_i_send;
    double *y_i_receive;

    int process_n;
    int process_m;
    int flag = -1;
    int knn_count = 0;

    knnresult s_knn_result;
    

    clock_t start, end;

    node *root;
    minArray *min_arr;

    if(world_rank == 0){
        
       double *X = read_X(&n,&d,argv[3]);
       if(X == NULL){
           printf("Cannot allocate memory\n");
       }
       printf("Matrix read!"); 
       chunks = n/p;
       start = clock();

       if(p > 1){
           //broadcast to all other processes
           for (int i = 1; i < p-1; i++)
           {
               //send with MPI to each process the particular array
               MPI_Send(&X[i * chunks * d], chunks * d, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
           }
           MPI_Send(&X[(p - 1) * chunks * d], (chunks + n % p) * d, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD);
       }

       /* Message sent and received confirmation -> call kNN ~ vpt*/
       //parameter intialization for zero process

       process_n = chunks;
       process_m = chunks;

       x_i_data = malloc(process_m * d * sizeof(double));
       x_i_data = X;
    
       s_knn_result.k = k;
       s_knn_result.n = n;
       s_knn_result.ndist = malloc(process_m * k * sizeof(double));
       s_knn_result.nidx = malloc(process_m * k * sizeof(int));


       //Create Vantage Point tree of with the elements of process 0

       root = malloc(sizeof(node));

       root = vpTree_create(x_i_data, root, process_m, d);
       //set current node = nodeOn = root
       node* nodeOn = root;
    
       min_arr = malloc(sizeof(minArray));
       min_arr->ndist = malloc(k * sizeof(double));
       min_arr->nidx = malloc(k * sizeof(int));

       //x-query
       double* x_query = malloc(d * sizeof(double));
       for (int i = 0; i < process_m; i++)
       {
           //initialize min_arr
           for (int j = 0; j < k; j++)
           {
               min_arr->ndist[j] = INFINITY;
               min_arr->nidx[j] = -1;
           }
           for (int j = 0; j < d; j++)
           {
               x_query[j] = x_i_data[i*d + j];
           }
           searchVPT(x_query,root, d, k, min_arr);

           //now save the min_arr result to the appropriate s_knn_result positions
           for (int j = 0; j < k; j++)
           {
               s_knn_result.ndist[j * process_m + i] = min_arr->ndist[j];
               s_knn_result.nidx[j * process_m + i] = min_arr->nidx[i];
           }
           
       }
       //debug print
        printf("Hello World");
    }
    //send n,m values to other processes

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&d, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (world_rank == p - 1)
    {
        chunks = n/p;
        //process parameters initialization
        process_n = (chunks + n % p);
        process_m = (chunks + n % p);

        x_i_data = malloc(process_m * d * sizeof(double));
        MPI_Recv(x_i_data, process_m * d, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        s_knn_result.k = k;
        s_knn_result.n = process_m;
        s_knn_result.ndist = malloc(process_m * k * sizeof(double));
        s_knn_result.nidx = malloc(process_m * k * sizeof(int));


        // We now create the vp tree of the elements of process zero

        root = malloc(sizeof(node));
        root = vpTree_create(x_i_data, root, process_m, d);

        node *current_node = root;

        min_arr = malloc(sizeof(minArray));
        min_arr->ndist = malloc(k * sizeof(double));
        min_arr->nidx = malloc(k * sizeof(int));



        //prepare X_query

        double *x_query = malloc(d * sizeof(double));

        for (int i = 0; i < process_m; i++)
        {
            //Initialize minArray
            for (int j = 0; j < k; j++)
            {
                min_arr->ndist[j] = INFINITY;
                min_arr->nidx[j] = -1;
            }
            for (int j = 0; j < d; j++)
            {
                x_query[j] = x_i_data[i *d + j];
            }
            searchVPT(x_query,root,d,k,min_arr);

            //save the min array result to the appropriate s_knn_result positions
            for (int j = 0; j < k; j++)
            {
                s_knn_result.ndist[j * process_m + i] = min_arr->ndist[j];
                s_knn_result.nidx[j * process_m + i] = min_arr->nidx[j];

            }          
            
        }
    }
    else
    {
        chunks = n / p;
        process_n = chunks;
        process_m = chunks;
        x_i_data = malloc(process_m * d * sizeof(double));

        MPI_Recv(x_i_data, process_m * d, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        s_knn_result.k = k;
        s_knn_result.n = process_m;
        s_knn_result.ndist = malloc(process_m * k * sizeof(double));
        s_knn_result.nidx = malloc(process_m * k * sizeof(int));


        //Create the vp tree of elements of process 0

        root = malloc(sizeof(node));
        root = vpTree_create(x_i_data, root ,process_m , d);

        node *currentNode = root;
       
       
        min_arr = malloc(sizeof(minArray));
        min_arr->ndist = malloc(k * sizeof(double));
        min_arr->nidx = malloc(k * sizeof(int));
        //prepare X_query

        double *x_query = malloc(d * sizeof(double));

        for (int i = 0; i < process_m; i++)
        {
            //Initialize minArray
            for (int j = 0; j < k; j++)
            {
                min_arr->ndist[j] = INFINITY;
                min_arr->nidx[j] = -1;
            }
            for (int j = 0; j < d; j++)
            {
                x_query[j] = x_i_data[i * d + j];
            }
            searchVPT(x_query, root, d, k, min_arr);

            //save the min array result to the appropriate s_knn_result positions
            for (int j = 0; j < k; j++)
            {
                s_knn_result.ndist[j * process_m + i] = min_arr->ndist[j];
                s_knn_result.nidx[j * process_m + i] = min_arr->nidx[j];
            }
        }
    }

    // x_i_data of each process has been processed
    //Every process now will create its own vantage point tree
    // Then find knn for every x_i_data according  to this vp tree
    // NOW PASS DATA TO EACH OTHER ~ RING
    // REPEAT UNTIL EACH PROCESS HAS PROCESSED ALL POSSIBLE DATA ~ p times = number of processes

    chunks = n/p;

    process_m = chunks;
    process_n = chunks;
    //set the y_i_send array for first iteration

    y_i_send = malloc(process_n * d * sizeof(double));

    double *s_knn_result_ndist_send = malloc(process_m * k * sizeof(double));
    int *s_knn_result_nidx_send = malloc(process_m * k * sizeof(int));
    //copy the block of memory from x_i_data to y_i_send etc.
    memcpy(y_i_send,x_i_data,process_m*d*sizeof(double));
    memcpy(s_knn_result_ndist_send, s_knn_result.ndist, process_m*k*sizeof(double));
    memcpy(s_knn_result_nidx_send, s_knn_result.nidx, process_m * k * sizeof(int));


    for (int i = 0; i < p - 1; i++)
    {
        // 0 sends then waits for the last process
        if(world_rank != 0){
            process_n = chunks;
            y_i_receive = malloc(process_n * d * sizeof(double));
            MPI_Recv(y_i_receive, process_n * d, MPI_DOUBLE, world_rank - 1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(s_knn_result.ndist, process_m * k, MPI_DOUBLE, world_rank - 1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(s_knn_result.nidx, process_m*k, MPI_INT, world_rank - 1, i,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }

        MPI_Send(y_i_send, process_n *d, MPI_DOUBLE, (world_rank + 1)%p, i, MPI_COMM_WORLD);
        MPI_Send(s_knn_result_ndist_send, process_m *k, MPI_DOUBLE,(world_rank + 1)%p,i,MPI_COMM_WORLD);
        MPI_Send(s_knn_result_nidx_send, process_m * k, MPI_INT, (world_rank + 1)%p, i, MPI_COMM_WORLD);

        
        if(world_rank != 0){
            //After sending the previous y data the process halts,in the next iteration, i want to send the new data i received
            memcpy(y_i_send, y_i_receive, process_m *d *sizeof(double));
        }

        if(world_rank == 0){
            process_m = chunks;
            y_i_receive = malloc(process_n * d * sizeof(double));
            MPI_Recv(y_i_receive, process_n*d, MPI_DOUBLE, p-1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(s_knn_result.ndist, process_m * d, MPI_DOUBLE, p - 1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(s_knn_result_nidx_send, process_m * k, MPI_INT, p -1 , i,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //copy
            memcpy(y_i_send, y_i_receive, process_m*d*sizeof(double));
        }

        //Prepare x_query

        double *x_query = malloc(d * sizeof(double));
        for (int i = 0; i < process_m; i++)
        {
            //Initialize min_arr
            for (int j = 0; j < k; j++)
            {
                min_arr->ndist[j] = s_knn_result.ndist[j *process_m + i];
                min_arr->nidx[j] = s_knn_result.nidx[j*process_m +i];
            }
            for (int j = 0; j < d; j++)
            {
                x_query[j] = y_i_receive[i*d + j];
            }
            searchVPT(x_query, root, d, k, min_arr);
            
            // Now save the min_arr to the appropriate s_knn_result position
            for (int j = 0; j < k; j++)
            {
                s_knn_result.ndist[j*process_m + i] = min_arr->ndist[j];
                s_knn_result.nidx[j*process_m + i] = min_arr->nidx[j];
            }
            
            
        }
        memcpy(s_knn_result_ndist_send, s_knn_result.ndist, process_m * k * sizeof(double));
        memcpy(s_knn_result_nidx_send, s_knn_result.nidx, process_m * k * sizeof(int));
        
    }    
    
    
    
     //Signal to zero process that everything has finished calculating
    int token;
    if (world_rank != 0)
    {
        MPI_Send(&token, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }else
    {
        for (int i = 1; i < p; i++)
        {
            MPI_Recv(&token, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        // Stop the timer
        end = clock();
        double duration = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Duration: %f\n", duration);
    }

    MPI_Finalize();
    free(x_i_data);
    free(s_knn_result.ndist);
    free(s_knn_result.nidx);
    free(root);
    free(min_arr->ndist);
    free(min_arr->nidx);
    free(min_arr);
    free(y_i_send);
    free(s_knn_result_ndist_send);
    free(s_knn_result_nidx_send);
    
    return EXIT_SUCCESS; 

}