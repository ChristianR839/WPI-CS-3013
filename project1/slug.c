#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SECONDS 6
#define MIN_SECONDS 2

void main(int argc, char *argv[]) {
    FILE *seedFile;
    char seedFileName[15] = "seed_slug_";
    int i;
    int seedValue, slugSeconds, slugCoinFlip;

    strcat(seedFileName, argv[1]);
    strcat(seedFileName, ".txt");

    seedFile = fopen(seedFileName, "r");
    if (seedFile == NULL) {
        printf("Could not read file. Exiting.\n");
        exit(1);
    }
    fscanf(seedFile, "%d", &seedValue);
    fclose(seedFile);

    srand(seedValue);

    printf("[Slug PID: %d] Read seed value: %d\n", getpid(), seedValue);

    slugSeconds = (rand() % (MAX_SECONDS - MIN_SECONDS + 1)) + MIN_SECONDS;
    slugCoinFlip = rand() % 2;

    printf("[Slug PID: %d] Delay time is %d seconds. Coin flip: %d.\n", getpid(), slugSeconds, slugCoinFlip);
    printf("[Slug PID: %d] I'll get the job done. Eventually...\n", getpid());

    sleep(slugSeconds);

    char *cmd;
    char *args[4];

    switch (slugCoinFlip) {
        case 0:
            printf("[Slug PID: %d] Break time is over! I am running the 'last -i -x' command.\n", getpid());
            
            cmd = "last";
            args[0] = "last";
            args[1] = "-i";
            args[2] = "-x";
            args[3] = NULL;

            break;
        case 1:
            printf("[Slug PID: %d] Break time is over! I am running the 'id --group' command.\n", getpid());
            
            cmd = "id";
            args[0] = "id";
            args[1] = "--group";
            args[2] = NULL;

            break;
    }

    execvp(cmd, args);
}