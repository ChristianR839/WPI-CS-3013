#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>


// Debug Args

char* arg1 = "FIFO";  // Policy name
char* arg2 = "tests/workload_5.in";  // Job trace
char* arg3 = "3";  // Time slice


// Data Structures

enum Policy { FIFO, SJF, RR };

struct Job {
    int id;
    int length;
    bool complete;
    pthread_t thread;

    int startTime;
    int endTime;
    int runTime;
    int waitTime;

    struct Job* next;
};


// Variables

enum Policy policy;

FILE* jobFile;
char* lineRead = NULL;
size_t bufferSize = 0;
ssize_t lineLength;
unsigned long lineNumber = 0;

unsigned int timeSlice;
unsigned int elapsedTime = 0;

char* policyNameArg;
char* jobTraceArg;
char* timeSliceArg;

typedef struct Job* jobNode;
jobNode jobList;

sem_t semRR;


// Function Prototypes

void policyFIFO();
void policySJF();
void policyRR();
jobNode createJob();
jobNode addJob(jobNode head, int id, int length);
int runJob(jobNode job);
void printFIFOAnalytics();
void printSJFAnalytics();
void printRRAnalytics();


// Main

int main(int argc, char **argv){

    // printf("Hello, please help me schedule!\n");

    // Verify arg count
    if (argc < 4) {
        printf("[Scheduler]: Insufficient argc. Exiting...\n");
        exit(1);
    }

    policyNameArg = argv[1];
    jobTraceArg = argv[2];
    timeSliceArg = argv[3];

    // Verify policy
    if (!strcmp(policyNameArg, "FIFO")) policy = FIFO;
    else if (!strcmp(policyNameArg, "SJF")) policy = SJF;
    else if (!strcmp(policyNameArg, "RR")) policy = RR;
    else {
        printf("[Scheduler]: Unexpected policy %s. Exiting...\n", policyNameArg);
        exit(1);
    }
    // printf("[Scheduler]: Policy: %s\n", policyNameArg);
    
    // Verify job trace
    jobFile = fopen(jobTraceArg, "r");
    if (jobFile == NULL) {
        printf("[Scheduler]: Job trace %s not found. Exiting...\n", jobTraceArg);
        exit(1);
    }

    // Initialize job list
    while ((lineLength = getline(&lineRead, &bufferSize, jobFile)) != -1) {
        jobList = addJob(jobList, lineNumber, atoi(lineRead));
        lineNumber += 1;
    }

    fclose(jobFile);
    // printf("[Scheduler]: Job trace: %s\n", jobTraceArg);

    // Verify time slice
    if (policy == RR) {
        timeSlice = atoi(timeSliceArg);
        // printf("[Scheduler]: Time slice: %i\n", timeSlice);
    }

    switch(policy) {
        case FIFO:
            printf("Execution trace with FIFO:\n");
            policyFIFO();
            printf("End of execution with FIFO.\n");

            printf("Begin analyzing FIFO:\n");
            printFIFOAnalytics();
            printf("End analyzing FIFO.\n");
            break;
        case SJF:
            printf("Execution trace with SJF:\n");
            policySJF();
            printf("End of execution with SJF.\n");

            printf("Begin analyzing SJF:\n");
            printSJFAnalytics();
            printf("End analyzing SJF.\n");
            break;
        case RR:
            printf("Execution trace with RR:\n");
            policyRR();
            printf("End of execution with RR.\n");

            printf("Begin analyzing RR:\n");
            printRRAnalytics();
            printf("End analyzing RR.\n");
            break;
    }
}


// Functions

jobNode createJob() {

    jobNode node;

    node = (jobNode) malloc(sizeof(struct Job));
    node->next = NULL;

    return node;
}

jobNode addJob(jobNode head, int id, int length) {
    
    jobNode node, nextNode;

    node = createJob();
    node->id = id;
    node->length = length;
    node->complete = false;
    node->startTime = -1;

    if (head == NULL) {
        head = node;
    } else {
        nextNode = head;
        while (nextNode->next != NULL) {
            nextNode = nextNode->next;
        }
        nextNode->next = node;
    }

    return head;
}

jobNode sortedJobInsert(jobNode head, jobNode newNode) {
    struct Job dummy;
    jobNode current = &dummy;
    dummy.next = head;

    while (current->next != NULL && current->next->length <= newNode->length) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;

    return dummy.next;
}

jobNode sortJobs(jobNode head) {
    jobNode result = NULL;
    jobNode current = head;
    jobNode next;

    while (current != NULL) {
        next = current->next;

        result = sortedJobInsert(result, current);
        current = next;
    }

    return result;
}

int runJob(jobNode job) {

    unsigned int runTime;

    if (policy == RR) {

        if ((job->length) == 0 && (job->complete) == false) {
            printf("Job %d ran for: %d\n", job->id, job->length);
        }

        if ((job->length) == 0) return 0;

        if ((job->length) < timeSlice) {
            runTime = job->length;
        } else {
            runTime = timeSlice;
        }

        job->runTime += runTime;
        
        usleep(runTime);

        job->length = (job->length) - runTime;

        printf("Job %d ran for: %d\n", job->id, runTime);
        return runTime;
    } else {
        usleep((job->length));

        printf("Job %d ran for: %d\n", job->id, job->length);
        return job->length;
    }
}

void policyFIFO() {
    jobNode currentJob = jobList;
    int runTime;
    while (currentJob != NULL) {
        currentJob->startTime = elapsedTime;
        runTime = runJob(currentJob);
        elapsedTime += runTime;
        currentJob->endTime = elapsedTime;
        currentJob = currentJob->next;
    }
}

void policySJF() {
    jobNode currentJob = sortJobs(jobList);
    jobList = currentJob;
    int runTime;
    while (currentJob != NULL) {
        currentJob->startTime = elapsedTime;
        runTime = runJob(currentJob);
        elapsedTime += runTime;
        currentJob->endTime = elapsedTime;
        currentJob = currentJob->next;
    }
}

void policyRR() {
    jobNode circularList = jobList;
    unsigned int jobCount = 0;
    int runTime;

    // Make the job list circular
    while (circularList != NULL) {
        jobCount += 1;
        if (circularList->next == NULL) {
            circularList->next = jobList;
            circularList = circularList->next;
            break;
        }
        circularList = circularList->next;
    }
    
    // Run all the jobs with RR policy for jobCount jobs
    while (jobCount > 0) {

        if (circularList->startTime == -1) {
            circularList->startTime = elapsedTime;
        }

        if (circularList->length > 0) {
            circularList->waitTime += elapsedTime - circularList->endTime;
        }

        runTime = runJob(circularList);

        elapsedTime += runTime;

        if (circularList->complete == false) {
            circularList->endTime = elapsedTime;
        }

        // If a job has not yet been marked complete AND it has no more time left
        // to run, mark it as complete and decrement the job count
        if (circularList->complete == false && circularList->length == 0) {
            circularList->complete = true;
            circularList->endTime = elapsedTime;
            jobCount -= 1;
        }

        circularList = circularList->next;
    }  
}

void printFIFOAnalytics() {
    jobNode currentJob = jobList;
    int count = 0;
    float avgResponseTime = 0, avgTurnaround = 0, avgWait = 0;

    while (currentJob != NULL) {

        count += 1;

        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
                currentJob->id, currentJob->startTime, currentJob->endTime, currentJob->startTime);
        
        avgResponseTime += currentJob->startTime;
        avgTurnaround += currentJob->endTime;
        avgWait += currentJob->startTime;

        currentJob = currentJob->next;
    }

    avgResponseTime /= count;
    avgTurnaround /= count;
    avgWait /= count;

    printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", avgResponseTime, avgTurnaround, avgWait);
}

void printSJFAnalytics() {
    jobNode currentJob = sortJobs(jobList);
    int count = 0;
    float avgResponseTime = 0, avgTurnaround = 0, avgWait = 0;

    while (currentJob != NULL) {

        count += 1;

        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
                currentJob->id, currentJob->startTime, currentJob->endTime, currentJob->startTime);
        
        avgResponseTime += currentJob->startTime;
        avgTurnaround += currentJob->endTime;
        avgWait += currentJob->startTime;

        currentJob = currentJob->next;
    }

    avgResponseTime /= count;
    avgTurnaround /= count;
    avgWait /= count;

    printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", avgResponseTime, avgTurnaround, avgWait);
}

void printRRAnalytics() {
    jobNode currentJob = jobList;
    int count = 0;
    float avgResponseTime = 0, avgTurnaround = 0, avgWait = 0;

    do {
        count += 1;

        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
                currentJob->id, currentJob->startTime, currentJob->endTime, currentJob->waitTime);
        
        avgResponseTime += currentJob->startTime;
        avgTurnaround += currentJob->endTime;
        avgWait += currentJob->waitTime;

        currentJob = currentJob->next;
    } while (currentJob->id != 0);

    avgResponseTime /= count;
    avgTurnaround /= count;
    avgWait /= count;

    printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", avgResponseTime, avgTurnaround, avgWait);
}