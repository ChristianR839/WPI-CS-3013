#include <stdlib.h>
#include <stdio.h>

#define CHILD_COUNT 5

void main() {
    
    FILE *seedFile;
    int i;
    int seedValue, childPID, childExitCode;

    seedFile = fopen(getcwd() + "seed.txt", "r");
    if (seedFile == NULL) {
        printf("Could not read file. Exiting.\n");
        exit(1);
    }
    fscanf(seedFile, "%d", &seedValue);
    fclose(seedFile);

    srand(seedValue);

    printf("Read seed value: %d\n", seedValue);

    int childValues[CHILD_COUNT];

    for (i = 0; i < CHILD_COUNT; i++) {
        childValues[i] = rand() % 6;
    }

    for (i = 0; i < CHILD_COUNT; i++) {

        int changeDirectorySuccess;

        switch (childValues[i]) {
            case 0:
                changeDirectorySuccess = chdir("/home");
                break;
            case 1:
                changeDirectorySuccess = chdir("/proc");
                break;
            case 2:
                changeDirectorySuccess = chdir("proc/sys");
                break;
            case 3:
                changeDirectorySuccess = chdir("/usr");
                break;
            case 4:
                changeDirectorySuccess = chdir("/usr/bin");
                break;
            case 5:
                changeDirectorySuccess = chdir("/bin");
                break;
        }

        char cwd[64];
        getcwd(cwd, sizeof(cwd));

        if (changeDirectorySuccess == 0) {
            printf("Selection #%d: %s [SUCCESS]\n", i+1, cwd);
        } else {
            printf("Selection #%d: %s [FAILURE]\n", i+1, cwd);
        }

        childPID = fork();
        
        if (childPID == 0) {
            printf("\t[Child, PID: %d]: Executing 'ls -tr' command...\n", getpid());

            char *cmd = "ls";
            char *args[3];
            args[0] = "ls";
            args[1] = "-tr";
            args[2] = NULL;

            execvp(cmd, args);

            break;
        } else {
            printf("[Parent]: I am waiting for PID %d to finish.\n", childPID);
            waitpid(childPID, &childExitCode, 0);
            childExitCode = WEXITSTATUS(childExitCode);
            printf("[Parent]: Child %d finished with status code %d. Onward!\n", childPID, childExitCode);
        }
    }
}