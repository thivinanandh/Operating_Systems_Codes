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
sem_t wmutex;
sem_t readTry;
sem_t resource;


int cnt = 1;
int numreader = 0;
int numwritter = 0;



void *writer(void *wno)
{   
    float r = (float)rand()/(float)RAND_MAX;
    sleep(r*0.4);
    int wno_int = *(int *) wno;
    // Get the writers lock
    sem_wait(&wmutex);

    // Update the writers count
    numwritter++;

    //if you're first, then you must lock the readers out. Prevent them from trying to enter CS
    if(numwritter ==1)
    {
        sem_wait(&readTry);
    }

    // Give up the writters mutex lock
    sem_post(&wmutex);

    // Perform Writting
    cnt = cnt*10;
    printf("[Accesed] : Writer %d modified cnt to %d\n",wno_int,cnt);

    // release the resource lock
    sem_post(&resource);

    //get writters mutex to decrease count
    sem_wait(&wmutex);

    // decrease the write count
    numwritter--;

    if(numwritter ==0)
    {
        sem_post(&readTry);
    }

    sem_post(&wmutex);
 

}
void *reader(void *rno)
{   
    float r = (float)rand()/(float)RAND_MAX;
    sleep(r*0.4);
    int rno_int = *(int *) rno;
    // Indicate a reader is trying to enter
    sem_wait(&readTry);

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

    //release the entry lock
    sem_post(&rmutex);

    //release 
    sem_post(&readTry);

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
    
    sem_init(&rmutex,0,1);
    sem_init(&wmutex,0,1);
    sem_init(&readTry,0,1);
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

    
    sem_destroy(&wmutex);
    sem_destroy(&rmutex);
    sem_destroy(&readTry);
    sem_destroy(&resource);

    return 0;
    
}
