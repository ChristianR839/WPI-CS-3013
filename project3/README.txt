Christian Rua
Project 3

----------------------------------------------------------------

Workloads:

1. [3, 3, 3, 3, 3]
For workload 1, the response time and the wait time had to be equivalent when running a RR policy. To do this, I made evey job require 3 units to run, which would mean that they would never have to do any additional waiting after they executed once (time slice was 3 units).

2. [500, 1, 1, 1, 1, 1, 1, 1, 1, 1]
For workload 2, I made it so that the first job was a lot longer than the rest of the jobs (499 unit difference). Because of this, the turnaround time using a FIFO policy is going to be offset by a significant amount, when compared to the SJF, which will put the longest job last. It took a bit of work to determin that 500 and nine 1s was a combination that would result in an approximately 10x difference.

3. [3, 3, 3, 3, 3]
For workload 3, I used the same jobs as workload 1 because they accomplish the task. Each of the measurements remains the same between policies because there is no need to rearrange the order (SJF) or split the jobs into multiple pieces (RR).

4. [1, 1, 1, 1, 900]
For workload 4, I decided that I wanted to have a few small jobs then one really large one because I needed each job to begin very quickly, but one of them to have a turnaround time so large that it moves the average over 100 units.

5. [3, 9, 12]
For workload 5, I did a lot of trial-and-error. I recognized that the job lengths of the first two had to add up to 15 (counting job 1 twice, which I didn't realize until after doing a handful of tests), so some testing got me 3 (given) and 9. Then, I tested a bunch of numbers until 12 got me the desired turnaround time.

----------------------------------------------------------------

For all three policies, I made use of a linked list of jobs, each represented as a struct with the following format:

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

This list is set up by reading input file before any of the policies run.


FIFO:
This policy was the most straightforward. I run each job in the order received and save the necessary data to calculate the analysis afterwards.

SJF:
This policy involves a sorting algorithm that re-arranges the list from least-to-greatest length. Then, it runs just like FIFO, in the order "received" and the data necessary for analysis is saved.

RR:
The most complicated policy, this one requires a restructuring of the job list into a circular list who's tail points to its head. Then, I cycle through the jobs, completing x units of each (where x is the time split) until all jobs have been complete. Throughout this process, I am collecting data for later analysis.

----------------------------------------------------------------

Additional information:
There are a few relics of older ideas throughout my code that remain unused. I don't have the available time to scrub through the program and clean it up. One such relic is the pthread_t that is in the Job struct, as I used to make use of threads in an earlier version of this project.

