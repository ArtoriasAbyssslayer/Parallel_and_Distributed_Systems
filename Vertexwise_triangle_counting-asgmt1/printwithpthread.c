#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


void *print_message_function(void *ptr);


void main(){
    pthread_t thread_1, thread_2;
    char *message1 =  "Thread 1";
    char *message2 =  "Thread 2";
    int iret1,iret2;
    // Create indeependent threads each of which will execute function

    iret1 = pthread_create(&thread_1,NULL, print_message_function,(void*) message1);
    iret2 = pthread_create( &thread_2, NULL, print_message_function, (void*) message2);

    pthread_join( thread_1, NULL);
    pthread_join( thread_2, NULL); 

    printf("Thread 1 returns: %d\n",iret1);
    printf("Thread 2 returns: %d\n",iret2);
    exit(0);
}

void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
}