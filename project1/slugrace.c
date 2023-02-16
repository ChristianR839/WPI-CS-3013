#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SLUG_COUNT 4

void main() {
    
    FILE *seedFile;
    int i;
    int seedValue, childPID[SLUG_COUNT], childExitCode;
    struct timespec slugStart[SLUG_COUNT], slugStop[SLUG_COUNT], raceStart, raceStop;

    seedFile = fopen(getcwd() + "seed.txt", "r");
    if (seedFile == NULL) {
        printf("Could not read file. Exiting.\n");
        exit(1);
    }
    fscanf(seedFile, "%d", &seedValue);
    fclose(seedFile);

    srand(seedValue);

    printf("Read seed value: %d\n", seedValue);

    char *args[3];
    char *slugID = malloc(sizeof(char) * 2);

    clock_gettime(CLOCK_REALTIME, &raceStart);

    for (i = 0; i < SLUG_COUNT; i++) {

        sprintf(slugID, "%i", i+1);

        clock_gettime(CLOCK_REALTIME, &slugStart[i]);
        childPID[i] = fork();
        
        if (childPID[i] == 0) {
            args[0] = "./slug";
            args[1] = slugID;
            args[2] = NULL;

            printf("\t[Child, PID: %d]: Executing './slug %d' command...\n", getpid(), i+1);

            execvp(args[0], args);

            exit(0);
        } else {
            printf("[Parent]: I forked off child %d.\n", childPID[i]);
        }
    }

    unsigned int racing = 1;
    int tempPID, finishedCount = 0, racingSlugs[SLUG_COUNT] = {1, 1, 1, 1};
    double elapsedTime;

    while (racing) {
        printf("The race is ongoing. The following children are still racing: ");

        for (int i = 0; i < SLUG_COUNT; i++) {
            if (racingSlugs[i]) {
                printf("%d ", childPID[i]);
            }
        }
        printf("\n");

        for (i = 0; i < SLUG_COUNT; i++) {
            if (racingSlugs[i]) {
                tempPID = waitpid(childPID[i], &childExitCode, WNOHANG);

                if (tempPID > 0) {
                    clock_gettime(CLOCK_REALTIME, &slugStop[i]);
                    elapsedTime = (slugStop[i].tv_sec - slugStart[i].tv_sec);
                    printf("Child %d has crossed the finish line! It took %lf seconds\n", tempPID, elapsedTime);
                    racingSlugs[i] = 0;
                    finishedCount += 1;
                }

                if (finishedCount == SLUG_COUNT) {
                    clock_gettime(CLOCK_REALTIME, &raceStop);
                    elapsedTime = (raceStop.tv_sec - raceStart.tv_sec);
                    printf("The race is over! It took %lf seconds\n", elapsedTime);
                    racing = 0;
                }
            }
        }

        usleep(330000);
    }
}