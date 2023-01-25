#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// IN THIS PROBLEM, the readers will try to just read the variable count and the writters will try to 
// Modiffy the count by multiplying it with 10

// This is FIRST Problem, So the 

#define N_Readers 10
#define N_Writters 5

sem_t rmutex;
sem_t resource;
sem_t serviceQueue;


int cnt = 1;
int numreader = 0;




void *writer(void *wno)
{   
    float r = (float)rand()/(float)RAND_MAX;
    sleep(r+0.2);
    int wno_int = *(int *) wno;
    // printf("Writ : %d\n",wno_int);
    // Get the writers lock
    sem_wait(&serviceQueue);
     
     // request exclusive access to readcount
     sem_wait(&resource);
     
     // let next in line be serviced
     sem_post(&serviceQueue);

   

    // Perform Writting
    cnt = cnt*10;
    printf("[Accesed] : Writer %d modified cnt to %d\n",wno_int,cnt);

    // release the resource lock
    sem_post(&resource);

}

void *reader(void *rno)
{   
    float r = (float)rand()/(float)RAND_MAX;
    sleep(r+0.4);
    int rno_int = *(int *) rno;

    // printf("Read : %d\n",rno_int);
    // Indicate a reader is trying to enter
    sem_wait(&serviceQueue);

    //lock entry section to avoid race condition with other readers
    sem_wait(&rmutex);

    //report yourself as a reader
    numreader++;

    // if the current reader is the first one
    if(numreader ==1)
    {
        //if you are first reader, lock  the resource
        sem_wait(&resource);
    }

    sem_post(&serviceQueue);

    //release the entry lock
    sem_post(&rmutex);

   

    //Read the variable count
    printf("[Accesed] : Reader %d: read cnt as %d\n",rno_int,cnt);

    sem_wait(&rmutex);

    //reduce num reader
    numreader--;

    //if last reader, release the resource
    if(numreader == 0)
    {
        sem_post(&resource);
    }

    sem_post(&rmutex);
}

int main()
{   

    pthread_t read[N_Readers],write[N_Writters];
    
    // Intialize to 1
    sem_init(&rmutex,0,1);
    sem_init(&serviceQueue,0,1);
    sem_init(&resource,0,1);


    int TotalElements = N_Readers + N_Writters;


    for(int i = 0; i < 10; i++) {
        void *vargp = malloc(sizeof(int));
        *(int *) vargp = i;
        pthread_create(&read[i], NULL, (void *)reader, vargp);

    }
    for(int i = 0; i < 5; i++) {
        void *vargp = malloc(sizeof(int));
        *(int *) vargp = i;
        pthread_create(&write[i], NULL, (void *)writer, vargp);
    }

    for(int i = 0; i < 10; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }

    
    sem_destroy(&rmutex);
    sem_destroy(&serviceQueue);
    sem_destroy(&resource);

    return 0;
    
}
