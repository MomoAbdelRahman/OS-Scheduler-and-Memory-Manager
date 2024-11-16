#include "headers.h"

void clearResources(int);

struct processData 
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};

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
            i++;
        }
    }

    fclose(fptr);
}

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    struct processData processes[256];
    read_file(processes);
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Hello lovely user, what scheduling algorithm would you like to use?\n");
    printf("Choose (1) for Short Job First (SJF)\n");
    printf("Choose (2) for Preemptive Highest Priority First (PHPF)\n");
    printf("Choose (3) for Round Robin (RR)\n");
    int scheduling_type,quantum;
    scanf("%d",&scheduling_type);
    if(scheduling_type==3){
        printf("How long would you like your quantum to be? (Must be an integer)\n");
        scanf("%d",&quantum);
    }
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}


