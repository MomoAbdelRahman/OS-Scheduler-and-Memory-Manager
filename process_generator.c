#include "headers.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#define SCHEDULER_EXEC "/home/aml-ismail/Desktop/OS_Starter_Code/scheduler.out"
#define CLK_EXEC "/home/aml-ismail/Desktop/OS_Starter_Code/clk.out"
void clearResources(int);




struct processData 
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};

struct msgbuff{
    long mtype;
    int mtext;
};

struct processData priorityQueue[100];

int queueSize = 0; 


void insertProcess(struct processData process) {
    if (queueSize >= 100) return; // Queue is full

    // Insert process in sorted order based on arrival time
    int i;
    for (i = queueSize - 1; (i >= 0 && priorityQueue[i].arrivaltime > process.arrivaltime); i--) {
        priorityQueue[i + 1] = priorityQueue[i];
    }
    priorityQueue[i + 1] = process; 
    queueSize++;
}


// Function to remove the process with the earliest arrival time
struct processData removeEarliestArrival() {
    if (queueSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }
    return priorityQueue[--queueSize]; // Remove and return the first process
}

void read_file(struct processData processes[]){
    FILE* fptr;
    int i=0;
    fptr=fopen("processes.txt","r");
    

    char process_parameters[256];
    
    if(fptr!=NULL){
        fgets(process_parameters, 256, fptr);
        while(fgets(process_parameters, 256, fptr)){
            sscanf(process_parameters, "%d\t%d\t%d\t%d",&processes[i].id,&processes[i].arrivaltime,
            &processes[i].runningtime,&processes[i].priority);
            insertProcess(processes[i]);
            i++;
        }
    }

    fclose(fptr);
}

void printQueue() {
    printf("Current Process Queue:\n");
    printf("ID\tArrival Time\tRunning Time\tPriority\n");
    for (int i = 0; i < queueSize; i++) {
        printf("%d\t%d\t\t%d\t\t%d\n", 
            priorityQueue[i].id, 
            priorityQueue[i].arrivaltime, 
            priorityQueue[i].runningtime, 
            priorityQueue[i].priority);
    }
}

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization

    // 1. Read the input files.
    struct processData processes[256];
    read_file(processes);
    printQueue();
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int scheduling_type, quantum;
    quantum=-1;
    printf("Hello lovely user, what scheduling algorithm would you like to use?\n");
    printf("Choose (1) for Short Job First (SJF)\n");
    printf("Choose (2) for Preemptive Highest Priority First (PHPF)\n");
    printf("Choose (3) for Round Robin (RR)\n");
    scanf("%d",&scheduling_type);
    if(scheduling_type==3){
        printf("How long would you like your quantum to be? (Must be an integer)\n");
        scanf("%d",&quantum);
    }

    // 3. Initiate and create the scheduler and clock processes.

    int spid=fork();
    if(spid == -1){
        perror("Error in fork");
    }

    else if(spid==0){
        // char* arr[]={SCHEDULER_EXEC, scheduling_type, quantum, NULL};
        char strq[20];
        char strs[20];
        sprintf(strq, "%d", quantum);
        sprintf(strs, "%d", scheduling_type);
        char* arr[]={SCHEDULER_EXEC,strs, strq, NULL};
        int scheduler_init=execv(SCHEDULER_EXEC, arr);
        if(scheduler_init==-1){
            perror("Couldnt Execv");
        }
    }

    else{
        int cpid=fork();
        if (cpid==-1){
            perror("Error in second fork");
        }
        else if (cpid==0)
        {
            char* arr2[]={CLK_EXEC,NULL};
            int clk_init=execv(CLK_EXEC,arr2);
            if(clk_init==-1){
                perror("Couldn't Initialize Clock");
            }
        }
        else{

        // 4. Use this function after creating the clock process to initialize clock
        initClk();
        sleep(1);
        
        // To get time use this
        int x = getClk();
        printf("current time is %d\n", x);


        // TODO Generation Main Loop
        // 5. Create a data structure for processes and provide it with its parameters.
        // 6. Send the information to the scheduler at the appropriate time.
        // 7. Clear clock resources
        destroyClk(true);
        }
        
    }


}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}


