#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

// IN THIS PROBLEM, the readers will try to just read the variable count and the writters will try to 
// Modiffy the count by multiplying it with 10

// This is FIRST Problem, So the 

#define N_Readers 8
#define N_Writters 12

sem_t wrt;
pthread_mutex_t mutex;
int cnt = 1;
int numreader = 0;


void shuffle(int *array, int n) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

void *writer(void *wno)
{   
    float r = (float)rand()/(float)RAND_MAX;
    sleep(r);
    int wno_int = *(int *) wno;
    printf("[Waiting] : Writer %d is waiting for the lock to be released\n",(*((int *)wno)));
    sem_wait(&wrt);
    cnt = cnt*2;
    printf("[Accesed] : Writer %d modified cnt to %d\n",wno_int,cnt);
    sem_post(&wrt);

}
void *reader(void *rno)
{   
    float r = (float)rand()/(float)RAND_MAX;
    sleep(r);
    int rno_int = *(int *) rno;
    printf("[Waiting] : Reader %d is waiting for the num Reader lock\n",(*((int *)rno)));
    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutex);
    // printf("[Waiting] : Reader %d , Obtained numReader lock\n",(*((int *)rno)));
    numreader++;
    if(numreader == 1) {
        // printf("[Block]   : Reader %d , has blocked the Writters Semaphore\n",(*((int *)rno)));
        sem_wait(&wrt); // If this id the first reader, then it will block the writer
    }
    pthread_mutex_unlock(&mutex);
    // printf("[unLock]  : Reader %d , has unlocked access to Count variable\n",(*((int *)rno)));
    // Reading Section
    printf("[Accesed] : Reader %d: read cnt as %d\n",rno_int,cnt);

    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutex);
    numreader--;
    if(numreader == 0) {
        // printf("[unLock]  : Reader %d , has unlocked access to writters Semaphore\n",(*((int *)rno)));
        sem_post(&wrt); // If this is the last reader, it will wake up the writer.
    }
    pthread_mutex_unlock(&mutex);
}

int main()
{   

    pthread_t read[N_Readers],write[N_Writters];
    pthread_mutex_init(&mutex, NULL);
    sem_init(&wrt,0,1);



    for(int i = 0; i < N_Readers; i++) {
        void *vargp = malloc(sizeof(int));
        *(int *) vargp = i;
        pthread_create(&read[i], NULL, (void *)reader, vargp);

    }
    for(int i = 0; i < N_Writters; i++) {
        void *vargp = malloc(sizeof(int));
        *(int *) vargp = i;
        pthread_create(&write[i], NULL, (void *)writer, vargp);
    }

    for(int i = 0; i < N_Readers; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < N_Writters; i++) {
        pthread_join(write[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&wrt);

    return 0;
    
}
