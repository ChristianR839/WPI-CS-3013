#include <stdlib.h>
#include <stdio.h>

#define MAX_CHILDREN 13
#define MIN_CHILDREN 8

void main() {
    
    FILE *seedFile;
    int i;
    int seedValue, childCount, childPID, childValue, childExitCode, childWaitTime;

    seedFile = fopen(getcwd() + "seed.txt", "r");
    if (seedFile == NULL) {
        printf("Could not read file. Exiting.\n");
        exit(1);
    }
    fscanf(seedFile, "%d", &seedValue);
    fclose(seedFile);

    srand(seedValue);

    printf("Read seed value: %d\n", seedValue);

    childCount = (rand() % (MAX_CHILDREN - MIN_CHILDREN + 1)) + MIN_CHILDREN;

    printf("Random child count: %d\n", childCount);

    int childValues[childCount];

    for (i = 0; i < childCount; i++) {
        childValues[i] = rand();
    }

    for (i = 0; i < childCount; i++) {
        childPID = fork();
        
        if (childPID == 0) {
            childValue = childValues[i];
            childExitCode = (childValue % 50) + 1;
            childWaitTime = (childValue % 3) + 1;
            printf("\t[Child, PID: %d]: I am the child and I will wait %d seconds and exit with code %d.\n", getpid(), childWaitTime, childExitCode);
            sleep(childWaitTime);
            printf("\t[Child, PID: %d]: Now exiting...\n", getpid());
            exit(childExitCode);
        } else {
            printf("[Parent]: I am waiting for PID %d to finish.\n", childPID);
            waitpid(childPID, &childExitCode, 0);
            childExitCode = WEXITSTATUS(childExitCode);
            printf("[Parent]: Child %d finished with status code %d. Onward!\n", childPID, childExitCode);
        }
    }
}