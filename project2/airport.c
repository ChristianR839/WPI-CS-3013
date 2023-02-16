#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define SMALL_PLANES 30
#define TOTAL_PLANES 45

sem_t semRunway;
sem_t semRegion0Lock;
sem_t semRegion1Lock;
sem_t semRegion2Lock;
sem_t semRegion3Lock;
sem_t semRegion4Lock;
sem_t semRegion5Lock;
sem_t semRegion6Lock;

int i;

// Define states for all objects
enum PLANE_TYPE { SMALL = 2, LARGE = 3 };
enum PLANE_STATE { IDLE, AWAIT_TAKEOFF, TAKEOFF, FLYING, AWAIT_LANDING, LANDING };

// Region structures
typedef struct __Region {
    int id;
    sem_t* regionLock;
} Region;

Region region0 = { 0, &semRegion0Lock };
Region region1 = { 1, &semRegion1Lock };
Region region2 = { 2, &semRegion2Lock };
Region region3 = { 3, &semRegion3Lock };
Region region4 = { 4, &semRegion4Lock };
Region region5 = { 5, &semRegion5Lock };
Region region6 = { 6, &semRegion6Lock };

Region* regionMap[3][2] = { { &region1, &region2 },
                            { &region3, &region4 },
                            { &region5, &region6 } };

// Plane structures
typedef struct __Plane {
    pthread_t threadID;
    enum PLANE_TYPE type;
    enum PLANE_STATE state;
    Region* regions[3];
    int number;
    int waitTime;
} Plane;

Plane** planeList;

void* planeSim(void* threadPlane) {

    Plane* plane = (Plane*) threadPlane;

    // Declare any variables
    int regionMapCoords[plane->type*2];
    int sleepTime, waiting = 1, counter = 0;
    int region1Status = 0, region2Status = 0, region3Status = 0;

    while(1) {

        switch(plane->state) {
            case IDLE:

                // Random wait
                plane->waitTime = (rand() % 100) * 10;
                printf("[PLANE %d | IDLE]: Sleeping for %dµs\n", plane->number, plane->waitTime);
                usleep(plane->waitTime);

                plane->state = AWAIT_TAKEOFF;
                break;

            case AWAIT_TAKEOFF:

                // Set random starting region
                regionMapCoords[0] = rand() % 3;  // Row
                regionMapCoords[1] = rand() % 2;  // Col

                if (plane->type == LARGE) {
                    while ( regionMapCoords[0] == 1) {
                        regionMapCoords[0] = rand() % 3;
                    }
                }

                // Use a switch to determine the direction of travel (other regions)
                switch(regionMap[regionMapCoords[0]][regionMapCoords[1]]->id) {
                    case 1:
                        if (plane->type == LARGE) {
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                            // Region 6
                            regionMapCoords[4] = 2;
                            regionMapCoords[5] = 1;
                        } else {  // If SMALL
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                        }
                        break;
                    case 2:
                        if (plane->type == LARGE) {
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                            // Region 5
                            regionMapCoords[4] = 2;
                            regionMapCoords[5] = 0;
                        } else {  // If SMALL
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                        }
                        break;
                    case 3:
                        // SMALL only
                        switch (rand() % 3) {
                            case 0:
                                // Region 2
                                regionMapCoords[2] = 0;
                                regionMapCoords[3] = 1;
                                break;
                            case 1:
                                // Region 4
                                regionMapCoords[2] = 1;
                                regionMapCoords[3] = 1;
                                break;
                            case 2:
                                // Region 5
                                regionMapCoords[2] = 2;
                                regionMapCoords[3] = 0;
                                break;
                        }
                        break;
                    case 4:
                        // SMALL only
                        switch (rand() % 3) {
                            case 0:
                                // Region 1
                                regionMapCoords[2] = 0;
                                regionMapCoords[3] = 0;
                                break;
                            case 1:
                                // Region 3
                                regionMapCoords[2] = 1;
                                regionMapCoords[3] = 0;
                                break;
                            case 2:
                                // Region 6
                                regionMapCoords[2] = 2;
                                regionMapCoords[3] = 1;
                                break;
                        }
                        break;
                    case 5:
                        if (plane->type == LARGE) {
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                            // Region 2
                            regionMapCoords[4] = 0;
                            regionMapCoords[5] = 1;
                        } else {  // If SMALL
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                        }
                        break;
                    case 6:
                        if (plane->type == LARGE) {
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                            // Region 1
                            regionMapCoords[4] = 0;
                            regionMapCoords[5] = 0;
                        } else {  // If SMALL
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                        }
                        break;
                }

                sem_wait(&semRunway);
                plane->regions[0] = regionMap[regionMapCoords[0]][regionMapCoords[1]];
                plane->regions[1] = regionMap[regionMapCoords[2]][regionMapCoords[3]];
                plane->regions[2] = regionMap[regionMapCoords[4]][regionMapCoords[5]];
                sem_post(&semRunway);

                if (plane->type == LARGE) {
                    printf("[PLANE %d | AWAIT_TAKEOFF]: Regions for takeoff: %d, %d, %d\n",
                        plane->number, plane->regions[0]->id, plane->regions[1]->id, plane->regions[2]->id);
                } else {
                    printf("[PLANE %d | AWAIT_TAKEOFF]: Regions for takeoff: %d, %d\n",
                        plane->number, plane->regions[0]->id, plane->regions[1]->id);
                }

                if (plane->regions[0]->regionLock->__align <= 0 || plane->regions[1]->regionLock->__align <= 0) {
                    if (plane->type == LARGE) {
                        if (plane->regions[2]->regionLock->__align <= 0) {
                            printf("[PLANE %d | AWAIT_TAKEOFF]: Awaiting runway availability\n", plane->number);
                        }
                    } else {
                        printf("[PLANE %d | AWAIT_TAKEOFF]: Awaiting runway availability\n", plane->number);
                    }
                }

                while (1) {
                    region1Status = sem_trywait(plane->regions[0]->regionLock);
                    region2Status = sem_trywait(plane->regions[1]->regionLock);
                    
                    if (plane->type == LARGE) {
                        region3Status = sem_trywait(plane->regions[2]->regionLock);
                    }

                    if (plane->type == LARGE) {
                        if (region1Status == 0 && region2Status == 0 && region3Status == 0) {
                            break;
                        } else {
                            if (region1Status == 0) sem_post(plane->regions[0]->regionLock);
                            if (region2Status == 0) sem_post(plane->regions[1]->regionLock);
                            if (region3Status == 0) sem_post(plane->regions[2]->regionLock);
                        }
                    } else {
                        if (region1Status == 0 && region2Status == 0) {
                            break;
                        } else {
                            if (region1Status == 0) sem_post(plane->regions[0]->regionLock);
                            if (region2Status == 0) sem_post(plane->regions[1]->regionLock);
                        }
                    }
                }
                
                plane->state = TAKEOFF;
                break;

            case TAKEOFF:

                printf("[PLANE %d | TAKEOFF]: Taking off...\n", plane->number);

                sleepTime = (rand() % 100) * 10;
                printf("[PLANE %d | TAKEOFF]: Region %d, time %dµs\n",
                    plane->number, plane->regions[0]->id, sleepTime);
                usleep(sleepTime);
                sem_post(plane->regions[0]->regionLock);

                sleepTime = (rand() % 100) * 10;
                printf("[PLANE %d | TAKEOFF]: Region %d, time %dµs\n",
                    plane->number, plane->regions[1]->id, sleepTime);
                usleep(sleepTime);
                sem_post(plane->regions[1]->regionLock);

                if (plane->type == LARGE) {
                    sleepTime = (rand() % 100) * 10;
                    printf("[PLANE %d | TAKEOFF]: Region %d, time %dµs\n",
                        plane->number, plane->regions[2]->id, sleepTime);
                    usleep(sleepTime);
                    sem_post(plane->regions[2]->regionLock);
                }

                plane->state = FLYING;
                break;

            case FLYING:

                // Random wait
                plane->waitTime = (rand() % 100) * 10;
                printf("[PLANE %d | FLYING]: Sleeping for %dµs\n", plane->number, plane->waitTime);
                usleep(plane->waitTime);

                plane->state = AWAIT_LANDING;
                break;

            case AWAIT_LANDING:

                // Set random starting region
                regionMapCoords[0] = rand() % 3;  // Row
                regionMapCoords[1] = rand() % 2;  // Col

                if (plane->type == LARGE) {
                    while ( regionMapCoords[0] == 1) {
                        regionMapCoords[0] = rand() % 3;
                    }
                }

                // Use a switch to determine the direction of travel (other regions)
                switch(regionMap[regionMapCoords[0]][regionMapCoords[1]]->id) {
                    case 1:
                        if (plane->type == LARGE) {
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                            // Region 6
                            regionMapCoords[4] = 2;
                            regionMapCoords[5] = 1;
                        } else {  // If SMALL
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                        }
                        break;
                    case 2:
                        if (plane->type == LARGE) {
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                            // Region 5
                            regionMapCoords[4] = 2;
                            regionMapCoords[5] = 0;
                        } else {  // If SMALL
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                        }
                        break;
                    case 3:
                        // SMALL only
                        switch (rand() % 3) {
                            case 0:
                                // Region 2
                                regionMapCoords[2] = 0;
                                regionMapCoords[3] = 1;
                                break;
                            case 1:
                                // Region 4
                                regionMapCoords[2] = 1;
                                regionMapCoords[3] = 1;
                                break;
                            case 2:
                                // Region 5
                                regionMapCoords[2] = 2;
                                regionMapCoords[3] = 0;
                                break;
                        }
                        break;
                    case 4:
                        // SMALL only
                        switch (rand() % 3) {
                            case 0:
                                // Region 1
                                regionMapCoords[2] = 0;
                                regionMapCoords[3] = 0;
                                break;
                            case 1:
                                // Region 3
                                regionMapCoords[2] = 1;
                                regionMapCoords[3] = 0;
                                break;
                            case 2:
                                // Region 6
                                regionMapCoords[2] = 2;
                                regionMapCoords[3] = 1;
                                break;
                        }
                        break;
                    case 5:
                        if (plane->type == LARGE) {
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                            // Region 2
                            regionMapCoords[4] = 0;
                            regionMapCoords[5] = 1;
                        } else {  // If SMALL
                            // Region 3
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 0;
                        }
                        break;
                    case 6:
                        if (plane->type == LARGE) {
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                            // Region 1
                            regionMapCoords[4] = 0;
                            regionMapCoords[5] = 0;
                        } else {  // If SMALL
                            // Region 4
                            regionMapCoords[2] = 1;
                            regionMapCoords[3] = 1;
                        }
                        break;
                }

                sem_wait(&semRunway);
                plane->regions[0] = regionMap[regionMapCoords[0]][regionMapCoords[1]];
                plane->regions[1] = regionMap[regionMapCoords[2]][regionMapCoords[3]];
                plane->regions[2] = regionMap[regionMapCoords[4]][regionMapCoords[5]];
                sem_post(&semRunway);

                if (plane->type == LARGE) {
                    printf("[PLANE %d | AWAIT_LANDING]: Regions for landing: %d, %d, %d\n",
                        plane->number, plane->regions[0]->id, plane->regions[1]->id, plane->regions[2]->id);
                } else {
                    printf("[PLANE %d | AWAIT_LANDING]: Regions for landing: %d, %d\n",
                        plane->number, plane->regions[0]->id, plane->regions[1]->id);
                }

                if (plane->regions[0]->regionLock->__align <= 0 || plane->regions[1]->regionLock->__align <= 0) {
                    if (plane->type == LARGE) {
                        if (plane->regions[2]->regionLock->__align <= 0) {
                            printf("[PLANE %d | AWAIT_LANDING]: Awaiting runway availability\n", plane->number);
                        }
                    } else {
                        printf("[PLANE %d | AWAIT_LANDING]: Awaiting runway availability\n", plane->number);
                    }
                }

                while (1) {
                    region1Status = sem_trywait(plane->regions[0]->regionLock);
                    region2Status = sem_trywait(plane->regions[1]->regionLock);
                    
                    if (plane->type == LARGE) {
                        region3Status = sem_trywait(plane->regions[2]->regionLock);
                    }

                    if (plane->type == LARGE) {
                        if (region1Status == 0 && region2Status == 0 && region3Status == 0) {
                            break;
                        } else {
                            if (region1Status == 0) sem_post(plane->regions[0]->regionLock);
                            if (region2Status == 0) sem_post(plane->regions[1]->regionLock);
                            if (region3Status == 0) sem_post(plane->regions[2]->regionLock);
                        }
                    } else {
                        if (region1Status == 0 && region2Status == 0) {
                            break;
                        } else {
                            if (region1Status == 0) sem_post(plane->regions[0]->regionLock);
                            if (region2Status == 0) sem_post(plane->regions[1]->regionLock);
                        }
                    }
                }

                plane->state = LANDING;
                break;

            case LANDING:

                printf("[PLANE %d | LANDING]: Landing...\n", plane->number);

                sleepTime = (rand() % 100) * 10;
                printf("[PLANE %d | LANDING]: Region %d, time %dµs\n",
                    plane->number, plane->regions[0]->id, sleepTime);
                usleep(sleepTime);
                sem_post(plane->regions[0]->regionLock);

                sleepTime = (rand() % 100) * 10;
                printf("[PLANE %d | LANDING]: Region %d, time %dµs\n",
                    plane->number, plane->regions[1]->id, sleepTime);
                usleep(sleepTime);
                sem_post(plane->regions[1]->regionLock);

                if (plane->type == LARGE) {
                    sleepTime = (rand() % 100) * 10;
                    printf("[PLANE %d | LANDING]: Region %d, time %dµs\n",
                        plane->number, plane->regions[2]->id, sleepTime);
                    usleep(sleepTime);
                    sem_post(plane->regions[2]->regionLock);
                }

                printf("[PLANE %d]: Landed successfully\n", plane->number);

                plane->state = IDLE;
                break;
        }

        if (plane->state == IDLE) break;
    }
}


// Main
void main(void) {

    FILE *seedFile;
    int seedValue;

    seedFile = fopen("seed.txt", "r");
    if (seedFile == NULL) {
        printf("Could not read file. Exiting.\n");
        exit(1);
    }
    fscanf(seedFile, "%d", &seedValue);
    fclose(seedFile);

    srand(seedValue);

    printf("Random seed value: %d\n", seedValue);

    // Init semaphores
    sem_init(&semRunway, 0, 1);
    sem_init(&semRegion0Lock, 0, 1);
    sem_init(&semRegion1Lock, 0, 1);
    sem_init(&semRegion2Lock, 0, 1);
    sem_init(&semRegion3Lock, 0, 1);
    sem_init(&semRegion4Lock, 0, 1);
    sem_init(&semRegion5Lock, 0, 1);
    sem_init(&semRegion6Lock, 0, 1);

    // Create planeList
    planeList = (Plane**) malloc(TOTAL_PLANES * sizeof(Plane*));
    for (i = 0; i < TOTAL_PLANES; i++) {
        Plane* plane = (Plane*) malloc(sizeof(Plane));
        plane->number = i;

        if (i < SMALL_PLANES) {
            plane->type = SMALL;
        } else {
            plane->type = LARGE;
        }

        planeList[i] = plane;
    }

    // Create threads
    for (i = 0; i < TOTAL_PLANES; i++) {
        pthread_create(&(planeList[i]->threadID), NULL, planeSim, planeList[i]);
    }

    // Join threads
    for (i = 0; i < TOTAL_PLANES; i++) {
        pthread_join(planeList[i]->threadID, NULL);
    }
}