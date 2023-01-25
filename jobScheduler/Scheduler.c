

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

// For adding Colors
/* FOREGROUND */
#define RST "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#define FILE_REDIRECT dup2

#define MIN_NUM_PROC 1
#define MAX_NUM_PROC 8

typedef struct process
{
    int processID;        // process ID
    pthread_t tid;        // Thread ID
    char *command;        // command for Execution
    char *start;          // save the start time of process
    int exitStatus;       // Checks whether the process has exited or not
    char outFileName[10]; // Output file for Redirecting output of function
    char errFileName[10]; // Error file for the output
    char *jobStatus;
    char *stop; // Save the end time of process

} process;

typedef struct queue
{
    int count;        // Count
    int size;         // Size of Queue
    process **buffer; // buffer for Process
    int end;          // end of Queue
    int start;        // start of Queue
} queue;

// A function to continiously get lines from the Console
int obtainUserInput(int number, char *string)
{
    int i, c;
    for (i = 0; i < number - 1 && (c = getchar()) != '\n'; ++i)
    {
        if (c != EOF)
            string[i] = c;
        else
            return -999;
    }
    string[i] = '\0';
    return i;
}

// get the location of the Command from given argument
char *getUserCommand(char *line)
{

    char *string = strstr(line, "submit") + 6;
    printf("user command : %s\n", string);
    int i = 0;
    char ch = string[0];
    while (ch == ' ' || ch == '\t' || ch == '\n')
    {
        ++i;
        ch = string[i];
    }
    return string + i;
}



// Refered from internet for getting time String
char *current_datetime_str()
{
    time_t timeObject = time(NULL);
    int i, ch;
    char *formatedTimeString;

    char* timeString = ctime(&timeObject);
    i = -1;
    formatedTimeString = malloc(sizeof(char) * strlen(timeString));
    while (ch != '\n' && (ch = timeString[++i]) != '\0' )
        formatedTimeString[i] = ch;
    formatedTimeString[i] = '\0';
    
    return formatedTimeString;
}



/* Open a log file with the given filename and return its file descriptor. */
int fileOut(char *fn)
{
    int fd;
    if ((fd = open(fn, O_CREAT | O_APPEND | O_WRONLY, 0755)) == -1)
    {
        fprintf(stderr, "Error: failed to open \"%s\"\n", fn);
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}

// Funciton for AppendinfG String
char *AppendFileName(int n, char *s)
{
    char *str = malloc(sizeof(char) * 11);
    sprintf(str, "%d.%s", n, s);
    return str;
}



// Creates a process with
process startJob(char *command, int processID)
{
    process j;
    j.processID = processID;
    j.start = j.stop = NULL;
    j.jobStatus = "waiting";

    char *copyCommand = malloc(sizeof(char) * strlen(command) + 1);
    strncpy(copyCommand, command, strlen(command) + 1);
    j.command = copyCommand;

    j.exitStatus = -1;

    char *outFileName;
    char *errFileName;
    outFileName = AppendFileName(j.processID, "outLog");
    errFileName = AppendFileName(j.processID, "errLog");

    strcpy(j.outFileName, outFileName);
    strcpy(j.errFileName, outFileName);
    return j;
}

// Displays the lists of Jobs as they are Added to the queue
void list_jobs(process *jobs, int n, char *mode)
{
    int i;
    if (jobs != NULL && n != 0)
    {

        if (strcmp(mode, "showjobs") == 0)
        {
            int oneAvail = 0;
            for (i = 0; i < n; ++i)
            {
                if (strcmp(jobs[i].jobStatus, "complete") != 0)
                {
                    printf(FBLU("Job ID: | Command: \t \t| Status: \t|\n"));
                    printf(FBLU("-------------------------------------------------\n"));
                    oneAvail = 1;
                    break;
                }
                
            }

            if (oneAvail)
            {
                for (i = 0; i < n; ++i)
                {
                    if (strcmp(jobs[i].jobStatus, "complete") != 0)
                    {
                        printf("   %d      %s                %s \n",jobs[i].processID,jobs[i].command,jobs[i].jobStatus);
                        oneAvail++;
                    }
                }
            }

           if(oneAvail)  printf(FBLU("Number in Queue : %d\n"),oneAvail-1);
        }
        else if (strcmp(mode, "submithistory") == 0)
        {
            int oneAvail = 0;
            for (i = 0; i < n; ++i)
            {
                  if (strcmp(jobs[i].jobStatus, "complete") == 0)
                {
                    printf(FBLU("Job ID: | Command: \t \t| Start: \t\t\t| End   \t\t\t |   Status   | \n"));
                    printf(FBLU("--------------------------------------------------------------------------------------------------------------\n"));
                    oneAvail = 1;
                    break;
                }     
            }

            for (int i = 0; i < n; ++i)
            {
                // printf("Status : %s  at %d\n",jobs[i].jobStatus,i);  
                  if (strcmp(jobs[i].jobStatus, "complete") == 0)
                {
                    printf("   %d      %s               %s          %s             %d\n", jobs[i].processID,jobs[i].command,jobs[i].start,jobs[i].stop,jobs[i].exitStatus);
                    oneAvail++;
                }     
            }

            if(oneAvail) printf(FBLU("Total History : %d \n"),oneAvail-1);
        }
    }
}

/*
Queue implemented from given Tar.gz file    
*/
queue *queue_init(int n)
{
    queue *q = malloc(sizeof(queue));
    q->size = n;
    q->buffer = malloc(sizeof(process *) * n);
    q->start = 0;
    q->end = 0;
    q->count = 0;

    return q;
}

/*
Insert to queue
*/
int queue_insert(queue *q, process *jp)
{
    if ((q == NULL) || (q->count == q->size))
        return -1;

    q->buffer[q->end % q->size] = jp;
    q->end = (q->end + 1) % q->size;
    ++q->count;

    return q->count;
}

/*
Delete from queue
*/
process *queue_delete(queue *q)
{
    if ((q == NULL) || (q->count == 0))
        return (process *)-1;

    process *j = q->buffer[q->start];
    q->start = (q->start + 1) % q->size;
    --q->count;

    return j;
}


// Delete the queue 

void queue_destroy(queue *q)
{
    free(q->buffer);
    free(q);
}

#define M_NumJobs 50
#define M_JobQueueLength 100
#define M_LengthOfLine 200

int G_Concurrent;
int G_NumWorking;
process ProcessList[M_NumJobs]; /* permanent array of submitted jobs */
queue *G_JobsQueue;             /* queue of pointers to waiting jobs */

void inputParser()
{
    char *cmd;
    int processCounter;
    char *userArguments;
    char line[M_LengthOfLine];

    printf(FYEL("************************************************************************************************\n"));
    printf(FYEL("**----------------------- Welcome to Custom Job Scheduler -----------------------------------**\n"));
    printf(FYEL("**-------- ##   With Great power  comes great Responsibilty ##--------------------------------**\n"));
    printf(FYEL("************************************************************************************************\n"));
    printf(FBLU("1. submit <COMMAND>"));
    printf(" - runs a process.provide args at end\n");
    printf(FBLU("2. submithistory"));
    printf(" - provides history of all jobs Submitted\n");
    printf(FBLU("3. showjobs"));
    printf(" - displays all th jobs\n");
    printf(FBLU("4. exit"));
    printf(" - exits Scheduler \n");

    processCounter = 0;
    while (printf(FYEL("Custom_Job_Scheduler # ")) && obtainUserInput(M_LengthOfLine, line) != -999)
    {
        char *lineNew = malloc(sizeof(char) * strlen(line));
        char *lineCopy = malloc(sizeof(char) * strlen(line));
        strcpy(lineNew, line);
        strcpy(lineCopy, line);

        if ((userArguments = strtok(lineNew, " \t\n\r")) != NULL)
        {
            if (strcmp(userArguments, "showjobs") == 0)
                list_jobs(ProcessList, processCounter, userArguments);
            else if (strcmp(userArguments, "submit") == 0)
            {
                if (G_JobsQueue->count >= G_JobsQueue->size)
                    printf(FGRN("Cannot add more jobs - job Queue Full \n"));
                else
                {
                    ProcessList[processCounter] = startJob(getUserCommand(lineCopy), processCounter);
                    queue_insert(G_JobsQueue, ProcessList + processCounter);
                    printf(FGRN("process %d Added to the process queue\n"), processCounter++);
                }
            }
            else if (strcmp(userArguments, "submithistory") == 0)
            {
                list_jobs(ProcessList, processCounter, userArguments);
            }

            else if (strcmp(userArguments, "exit") == 0)
                kill(0, 9);
        }
    }
    kill(0, SIGINT);
}


void redirectOutput(char* out, char* err)
{
    FILE_REDIRECT(fileOut(out), 1); //redirect to std ut
    FILE_REDIRECT(fileOut(err), 2); // redirect to std err
}


void *runJobThread(void *arg)
{
    process *jp;
    char **args; 
    pid_t processIDno;   

    jp = (process *)arg;

    ++G_NumWorking;

    jp->jobStatus = "working";
    jp->start = current_datetime_str();

    processIDno = fork();
    if (processIDno == 0) 
    {
        redirectOutput(jp->outFileName,jp->errFileName);
        // Get the Arguments
        char *copy = malloc(sizeof(char) * (strlen(jp->command) + 1));
        strcpy(copy, jp->command);

        char *arguments;
        char **arrayOfArguments = malloc(sizeof(char *));
        int token = 0;
        while ((arguments = strtok(copy, " \t")) != NULL)
        {
            arrayOfArguments[token] = malloc(sizeof(char) * (strlen(arguments) + 1));
            strcpy(arrayOfArguments[token], arguments);
            arrayOfArguments = realloc(arrayOfArguments, sizeof(char *) * (++token + 1));
            copy = NULL;
        }   
        arrayOfArguments[token] = NULL;

        execvp(arrayOfArguments[0], arrayOfArguments);
        fprintf(stderr, "Error: command execution failed for \"%s\"\n", args[0]);
        perror("execvp");
        exit(1);
    }
    else if (processIDno > 0) /* parent process */
    {
        waitpid(processIDno, &jp->exitStatus, 2);
        jp->jobStatus = "complete";

        jp->stop = current_datetime_str();
    }
    else
    {
        fprintf(stderr, "Error: process fork failed\n");
        perror("fork");
        exit(1);
    }

    --G_NumWorking;
    return NULL;
}


void *runJobs(void *arg)
{
    process *jp; /* process pointer */

    G_NumWorking = 0;
    for (;;)
    {
        if ( G_NumWorking - 1< G_Concurrent -1 && G_JobsQueue->count + 1 > 1)
        {
            /* pull next process from queue */
            jp = queue_delete(G_JobsQueue);

            /* use a new thread to complete process */
            pthread_create(&jp->tid, NULL, runJobThread, jp);

            /* mark new thread detached to free its resources when done */
            pthread_detach(jp->tid);
        }
        sleep(1);
    }
    return NULL;
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Enter the Number of Process we need to run concurrently as an argument : ");
        exit(0);
    }

    char *errFileName; /* filename where main stderr is redirected */
    pthread_t tid;     /* thread ID */

 
    G_Concurrent = atoi(argv[1]);
    if (G_Concurrent > MAX_NUM_PROC )
        G_Concurrent = MAX_NUM_PROC; 
    else if (G_Concurrent < MIN_NUM_PROC)
       G_Concurrent = MIN_NUM_PROC;

    /* redirect main stderr to file */
    errFileName = malloc(sizeof(char) * (strlen(argv[0]) + 5));
    sprintf(errFileName, "%s.err", argv[0]);
    FILE_REDIRECT(fileOut(errFileName), 2);

    G_JobsQueue = queue_init(M_JobQueueLength);

    /* use a new thread to complete jobs */
    pthread_create(&tid, NULL, runJobs, NULL);

    /* use the main thread to handle input */
    inputParser();

    exit(0);
}
