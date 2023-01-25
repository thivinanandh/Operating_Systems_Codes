#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define N_Users 5

struct TransactionLedger
{
    int time[100];
    int amount[100];
    int sender[100];
    int receiver[100];
    int transactionnumber[100];
};

struct threadParam
{
    int threadNo;
    struct TransactionLedger ledger;
};

// Global Parameters
int GlobaltransactionNumber = 0;
int GlobalTime = 0;
int GlobalSender;
int GlobalReceiver;
int GlobalAmount;

int GlobalBalance[N_Users];
FILE **fptr;
FILE *fptrNew;
int numLinesWritten[N_Users];

void printbalance()
{
    printf("Balance Details : <<< ");
    for (int k = 0; k < N_Users; k++)
        printf("Thread %d : %d , ", k, GlobalBalance[k]);
    printf(" >>> \n");
}

sem_t TransactionMutex;

void transaction(void *args)
{
    // Get the struct details
    struct threadParam threadObj = *(struct threadParam *)args;
    int threadnum = threadObj.threadNo;

    int transactionPerThread = 2;
    int transactionCount = 0;

    

    while (transactionPerThread > transactionCount)
    {
        // Add a Random Sleep function for each thread
        float r = (float)rand() / (float)RAND_MAX;
        sleep(r);

        printf("[Entry] : Thread No %d has reached \n", threadnum);

        // Pick a random Number and deceide whether to loan or Borrow.
        double rr = (double)rand();

        int borrow = 0; // Borrow = 1, if its going to borrow
        if (rr > 0.5)
            borrow = 1;

        // generate an random amount within 300
        double rrrr = (double)rand() / (double)RAND_MAX;
        int amt = rrrr * 300;

        // pick a random thread for transaction
        float rrr = (float)rand() / (float)RAND_MAX;
        int nextProcessor = (int)(rrr * 10000) % N_Users;

        while (threadnum == nextProcessor)
        {
            rrr = (float)rand() / (float)RAND_MAX;
            nextProcessor = (int)(rrr * 10000) % N_Users;
            // printf("stuck - next proc : %d , threadnum : %d\n",nextProcessor,threadnum);
        }

        // printf("[Waiting] Thread no %d is waiting to aquire lock \n",threadnum);
        // Check the balance in the next Processor
        sem_wait(&TransactionMutex);
        // printf("[Acquire] : mutex lock acquired by %d\n",threadnum);

        if (borrow)
            printf("[Transfer] Thread No : %d  wants to borrow %d from thread %d\n", threadnum, amt, nextProcessor);
        else
            printf("[Transfer] Thread No : %d  wants to lend %d to thread %d\n", threadnum, amt, nextProcessor);
        // printf(" [Transer] Thread No : %d ",threadnum);
        // printf(" , Borrow : %d , next processor : %d, amount : %d\n",borrow,nextProcessor,amt);

        if (borrow)
        {
            if (GlobalBalance[nextProcessor] < amt) // If the balance is less
                printf("[Error] : Transaction declined due to low balance \n");
            else // Transfer the money and update the balane and ledger
            {
                // Update balance
                GlobalBalance[threadnum] = GlobalBalance[threadnum] + amt;
                GlobalBalance[nextProcessor] = GlobalBalance[nextProcessor] - amt;

                int transactionNo = GlobaltransactionNumber;
                // Update Ledger
                threadObj.ledger.amount[transactionNo] = amt;
                threadObj.ledger.sender[transactionNo] = nextProcessor;
                threadObj.ledger.receiver[transactionNo] = threadnum;
                threadObj.ledger.transactionnumber[transactionNo] = transactionNo;

                GlobaltransactionNumber++;
                printf("[success] : Transaction no : %d ::-> Transfered %d usd from thread %d to thread %d \n", transactionNo, amt, nextProcessor, threadnum);
                printbalance();
            }
        }

        else // Send Money
        {
            if (amt > GlobalBalance[threadnum])
            {
                printf("Amount not sufficient to send to next processor \n");
            }
            else
            {
                // Update balance
                GlobalBalance[threadnum] = GlobalBalance[threadnum] - amt;
                GlobalBalance[nextProcessor] = GlobalBalance[nextProcessor] + amt;

                int transactionNo = GlobaltransactionNumber;
                // Update Ledger
                threadObj.ledger.amount[transactionNo] = amt;
                threadObj.ledger.sender[transactionNo] = threadnum;
                threadObj.ledger.receiver[transactionNo] = nextProcessor;
                threadObj.ledger.transactionnumber[transactionNo] = transactionNo;

                GlobaltransactionNumber++;
                printf("[success] :  Transaction no : %d ::-> Transfered %d usd from Thread %d to Thread %d \n", transactionNo, amt, threadnum, nextProcessor);
                printbalance();
            }
        }

        transactionCount++;

        // Write all Transactions to a all local Files Before proceeding
        for (int kk = 0; kk < N_Users; kk++)
        {
            // char str1[100];
            // sprintf(str1, "Transaction_Thread_%d.csv", kk);
            // printf("%s\n",str1);
            // fptrNew = fopen(str1, "a");
            // printf("Thread : %d , NumLines  : %d\n",kk, GlobaltransactionNumber);
            // printf("Comes here : %d , start : %d , end : %d \n ",kk,numLinesWritten[kk],GlobaltransactionNumber);
            // int lli = 0;
            if (numLinesWritten[kk] != GlobaltransactionNumber)
            {       
                for (int k = numLinesWritten[kk]; k < GlobaltransactionNumber; k++)
                {
                    // printf("*********************************************************** %d, %d \n",threadObj.ledger.transactionnumber[k],threadObj.ledger.sender[k]);
                    fprintf(fptr[kk], "%d,%d,%d,%d\n", threadObj.ledger.transactionnumber[k], threadObj.ledger.sender[k], threadObj.ledger.receiver[k], threadObj.ledger.amount[k]);
                    // printf( "%d,%d,%d,%d\n", threadObj.ledger.transactionnumber[k], threadObj.ledger.sender[k], threadObj.ledger.receiver[k], threadObj.ledger.amount[k]);
                    numLinesWritten[kk] ++;
                    sleep(0.8);
                }
            }

        }

        sleep(0.8);
        // Release the lock
        sem_post(&TransactionMutex);
        sleep(0.5);
        printf("[Release] : mutex lock released by %d\n", threadnum);
    }
    // Close file pointer
    
}

int main()
{
    pthread_t blockChainUsers[N_Users];

    struct TransactionLedger ledger;

    sem_init(&TransactionMutex, 0, 1);

    fptr = malloc( N_Users * sizeof(FILE*) );

    for (int i = 0; i < N_Users; i++)
    {
        // srand(time(NULL));   // Initialization, should only be called once.
        double r = (double)rand() / (double)RAND_MAX;
        // printf("%f\n",r);
        int amt = (int)(r * 1000);
        GlobalBalance[i] = amt;
        numLinesWritten[i] = 0;
        char str[100];
        sprintf(str, "Transaction_Thread_%d.csv", i);
        // Remove previous transaction files
        if (access(str, F_OK) == 0)
        {
            remove(str);
        }
        sprintf(str, "Transaction_Thread_%d.csv", i);
        // generate File Pointers
        fptr[i] = fopen(str,"a");

        // printf("Thread Number %d has Balance of : %d\n", i,amt);
    }
    printbalance();

    // Create 5
    for (int i = 0; i < N_Users; i++)
    {
        struct threadParam k;
        k.threadNo = i;
        k.ledger = ledger;

        void *vargp = malloc(sizeof(struct threadParam));
        *(struct threadParam *)vargp = k;


        //
        
        // Add a Balance to given
        pthread_create(&blockChainUsers[i], NULL, (void *)transaction, (struct threadParam *)vargp);
        sleep(0.4);
    }

    for (int i = 0; i < N_Users; i++)
    {
        pthread_join(blockChainUsers[i], NULL);
        // printf("threadclosed : %d\n",i);
    }

    //  for (int i = 0; i < N_Users; i++)
    // {
    //     fclose(fptr[i]);
    // }

    sem_destroy(&TransactionMutex);
    return 0;
}