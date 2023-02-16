#include <stdlib.h>
#include <stdio.h>

#define MAX_LIFESPAN 12
#define MIN_LIFESPAN 5

void main() {
    
    FILE *seedFile;
    int seedValue, lifespanCount, childPID, childExitCode;

    seedFile = fopen(getcwd() + "seed.txt", "r");
    if (seedFile == NULL) {
        printf("Could not read file. Exiting.\n");
        exit(1);
    }
    fscanf(seedFile, "%d", &seedValue);
    fclose(seedFile);

    srand(seedValue);

    printf("Read seed value: %d\n", seedValue);

    lifespanCount = (rand() % (MAX_LIFESPAN - MIN_LIFESPAN + 1)) + MIN_LIFESPAN;

    printf("Random decendant count: %d\n", lifespanCount);

    while (lifespanCount > 0) {
        childPID = fork();
        
        if (childPID == 0) {
            printf("\t[Child, PID: %d]: I was called with descendant count = %d. I'll have %d descendant(s).\n", getpid(), lifespanCount, lifespanCount-1);
        } else {
            printf("[Parent, PID: %d]: I am waiting for PID %d to finish.\n", getpid(), childPID);
            waitpid(childPID, &childExitCode, 0);
            childExitCode = WEXITSTATUS(childExitCode);
            printf("[Parent, PID: %d]: Child %d finished with status code %d. It's now my turn to exit.\n", getpid(), childPID, childExitCode);
            exit(childExitCode + 1);
        }

        lifespanCount -= 1;
    }

    exit(0);
}