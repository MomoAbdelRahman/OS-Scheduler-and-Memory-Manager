#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>



typedef short bool;
#define true 1
#define false 1

#define PROCESS_EXEC "process.out"

#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}

struct PCB{
    int id;
    char* state;
    int arrivaltime;
    int total_running_time;
    int remaining_time;
    int priority;
    int processedtime;
};

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

int final_process_sent=0;

float total_WTA;
int total_waiting;

void initializer(){
    total_WTA=0;
    total_waiting=0;
}

struct exitcode{
    struct PCB exit;
    int mtype;
};



void handler_all_processes_sent(int signum){
    final_process_sent=1; 
    signal(SIGUSR2,handler_all_processes_sent);
}

struct processData 
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    pid_t pid;
};

struct msgbuff{
    long mtype;
    struct processData data;
};


///////////////////////////////////////////////////Process Control/////////////////////////////////////////////
int new_process(struct msgbuff* m1){
    //printf("Scheduler PID: %d\n",getpid());
    int pid=fork();
    if(pid==-1){
        perror("Error in fork");
        return -1;
    }
    else if(pid==0){
        char id[3];
        char arrival_time[5];
        char priority[3];
        char running_time[6];
        sprintf(id, "%d", m1->data.id);
        sprintf(arrival_time, "%d", m1->data.arrivaltime);
        sprintf(priority,"%d",m1->data.priority);
        sprintf(running_time,"%d",m1->data.runningtime);
        printf("Copied");
        m1->data.pid=getpid();
        //printf("New Process PID:%d\n",m1->data.pid);
        char* arr[]={PROCESS_EXEC,id,arrival_time,priority,running_time, NULL};
        int proccess_init=execv(PROCESS_EXEC,arr);
        if(proccess_init==-1){
            perror("Error in execv for new process");
            return -1;
        }
        return getpid();
    }
}




int stop_process(struct processData p1){
    kill(p1.pid,SIGSTOP);   
}

int continue_process(struct processData p1){
    kill(p1.pid,SIGCONT);
}


FILE * pFile;
FILE* perf;
FILE* waiting_times_buffer;

//////////////////////////////////////////////Circular Queue//////////////////////////////////////////////////////

struct CircularQueue {
    int front;
    int rear;
    struct processData rrprocesses[256];
};

struct processData currently_running_rr={0};

struct CircularQueue RRqueue={-1,-1};

// Initialize the circular queue


bool RRisEmpty() {
    return((RRqueue.front==-1)&&(RRqueue.rear==-1));
}

bool RRisFull() {
    return ((RRqueue.rear + 1) % 256) == RRqueue.front;
}

void RRenqueue(struct processData process) {
    if(RRisEmpty()){
        //printf("emptyqueue");
        RRqueue.front=0;
        RRqueue.rear=0;
    }
    else if (!RRisFull()) {
        //printf("Case 2");
        RRqueue.rear = (RRqueue.rear + 1) % 256;
    }
   // printf("Enquing at: %d\n",RRqueue.rear);
    RRqueue.rrprocesses[RRqueue.rear] = process;
}

struct processData RRdequeue() {
    struct processData process={0}; 
    if(!RRisEmpty()){
        if(RRqueue.front==RRqueue.rear){
            //printf("Dequing at %d",RRqueue.front);
            process= RRqueue.rrprocesses[RRqueue.front];
            RRqueue.front=-1;
            RRqueue.rear=-1;
        }
        else if(!RRisEmpty()){
            process = RRqueue.rrprocesses[RRqueue.front];
            RRqueue.front = (RRqueue.front + 1) % 256;
        }
    }
    
    return process;
}

struct processData RRpeek() {
    if (!RRisEmpty()) {
        struct processData process = RRqueue.rrprocesses[RRqueue.front];
        return process;
    }
    struct processData emptyProcess = {0}; // Return an empty process if queue is empty
    return emptyProcess;
}

int dead=0;



void printRRQueue() {

    printf("Queue Size:%d Current Clock: %d\n",RRqueue.front > RRqueue.rear ? (256 - RRqueue.front + RRqueue.rear + 1) : (RRqueue.rear - RRqueue.front + 1), getClk());
    printf("Current Process Queue:\n");
    printf("ID\tArrival Time\tRunning Time\tPriority\n");

    printf("Currently Running\n%d\t%d\t\t%d\t\t%d\n", 
            currently_running_rr.id, 
            currently_running_rr.arrivaltime, 
            currently_running_rr.runningtime, 
            currently_running_rr.priority);
    printf("Waiting:\n");
    if(RRisEmpty()){}
    else{
        if(RRqueue.rear>=RRqueue.front){
            for(int i=RRqueue.front;i<=RRqueue.rear;i++){
                printf("%d\t%d\t\t%d\t\t%d\n", 
                RRqueue.rrprocesses[i].id, 
                RRqueue.rrprocesses[i].arrivaltime, 
                RRqueue.rrprocesses[i].runningtime, 
                RRqueue.rrprocesses[i].priority);
            }
        }
        else{
            for(int i=RRqueue.front;i<256;i++){
                printf("%d\t%d\t\t%d\t\t%d\n", 
                RRqueue.rrprocesses[i].id, 
                RRqueue.rrprocesses[i].arrivaltime, 
                RRqueue.rrprocesses[i].runningtime, 
                RRqueue.rrprocesses[i].priority);
            }
            for (int i=0; i<=RRqueue.rear;i++){
                printf("%d\t%d\t\t%d\t\t%d\n", 
                RRqueue.rrprocesses[i].id, 
                RRqueue.rrprocesses[i].arrivaltime, 
                RRqueue.rrprocesses[i].runningtime, 
                RRqueue.rrprocesses[i].priority);
            }
        }
    }
    printf("\n\n");
}

void handler_rr(int sig_num){
    struct processData removed=currently_running_rr;
    //printf("Process %d terminated at time %d\n",removed.id,getClk());
    currently_running_rr=RRdequeue();
    dead=1;
    printRRQueue();
    signal (SIGUSR1,handler_rr);
}

///////////////////////////////////////////////////////SJF PriQ////////////////////////////////////////////////////
struct processData sjf_priorityQueue[100];

int sjf_queueSize = 0;


void sjf_enqueue(struct processData process) {
    if (sjf_queueSize >= 100) return; // Queue is full

    // Insert process in sorted order based on arrival time
    int i;
    for (i = sjf_queueSize - 1; (i >= 0 && sjf_priorityQueue[i].runningtime > process.runningtime); i--) {
        sjf_priorityQueue[i + 1] = sjf_priorityQueue[i];
    }
    //printf("%d\n",process.id);
    sjf_priorityQueue[i + 1] = process; 
    sjf_queueSize++;
}


// Function to remove the process with the earliest arrival time
struct processData sjf_dequeue() {
    if (sjf_queueSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }

    struct processData earliestProcess = sjf_priorityQueue[0]; // Get the first process
    
    // Shift all elements to the left
    for (int i = 1; i < sjf_queueSize; i++) {
        sjf_priorityQueue[i - 1] = sjf_priorityQueue[i];
    }
    
    sjf_queueSize--; // Decrease the queue size
    return earliestProcess; // Return the first process
}
struct processData sjf_peek() {
    if (sjf_queueSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }
    return sjf_priorityQueue[0]; // Return the first process without removing it
}

struct processData currently_running_sjf;

void printsjfQueue() {

    printf("Queue Size:%d Current Clock: %d\n",sjf_queueSize, getClk());
    printf("Current Process Queue:\n");
    printf("ID\tArrival Time\tRunning Time\tPriority\n");
    
    printf("Currently Running\n%d\t%d\t\t%d\t\t%d\n", 
            currently_running_sjf.id, 
            currently_running_sjf.arrivaltime, 
            currently_running_sjf.runningtime, 
            currently_running_sjf.priority);
    printf("Waiting:\n");
    for (int i = 0; i < sjf_queueSize; i++) {
        printf("%d\t%d\t\t%d\t\t%d\n", 
            sjf_priorityQueue[i].id, 
            sjf_priorityQueue[i].arrivaltime, 
            sjf_priorityQueue[i].runningtime, 
            sjf_priorityQueue[i].priority);
    }
    printf("\n\n");
}

void handler_sjf(int sig_num){
    struct processData removed=currently_running_sjf;
    //printf("Process %d terminated at time %d\n",removed.id,getClk());
    signal (SIGUSR1,handler_sjf);
    currently_running_sjf=sjf_dequeue();
    printsjfQueue();
}



///////////////////////////////PHPF Priority Queue//////////////////////////////////

struct processData PHPF_PriQ[256];

int PHPF_PriQSize = 0; 

struct processData currently_running_phpf;

void PHPF_enqueue(struct processData process) {
    if(process.id!=0){
        if (PHPF_PriQSize >= 256) return; // Queue is full

        // Insert process in sorted order based on arrival time
        int i;
        for (i = PHPF_PriQSize - 1; (i >= 0 && PHPF_PriQ[i].priority > process.priority); i--) {
            PHPF_PriQ[i + 1] = PHPF_PriQ[i];
        }
        PHPF_PriQ[i + 1] = process; 
        PHPF_PriQSize++;
    }
}


// Function to remove the process with the highest priority
struct processData removeHighestPriority() {
    if (PHPF_PriQSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }

    struct processData earliestProcess = PHPF_PriQ[0]; // Get the first process
    
    // Shift all elements to the left
    for (int i = 1; i < PHPF_PriQSize; i++) {
        PHPF_PriQ[i - 1] = PHPF_PriQ[i];
    }
    
    PHPF_PriQSize--; // Decrease the queue size
    return earliestProcess; // Return the first process
}
struct processData peekHighestPriority() {
    if (PHPF_PriQSize == 0) {
        struct processData emptyProcess = {0}; // Return an empty process
        return emptyProcess;
    }
    return PHPF_PriQ[0]; // Return the first process without removing it
}

void printPHPFQueue() {

    printf("Queue Size:%d Current Clock: %d\n",PHPF_PriQSize, getClk());
    printf("Current Process Queue:\n");
    printf("ID\tArrival Time\tRunning Time\tPriority\n");
    
    printf("Currently Running\n%d\t%d\t\t%d\t\t%d\n", 
            currently_running_phpf.id, 
            currently_running_phpf.arrivaltime, 
            currently_running_phpf.runningtime, 
            currently_running_phpf.priority);
    printf("Waiting:\n");
    for (int i = 0; i < PHPF_PriQSize; i++) {
        printf("%d\t%d\t\t%d\t\t%d\n", 
            PHPF_PriQ[i].id, 
            PHPF_PriQ[i].arrivaltime, 
            PHPF_PriQ[i].runningtime, 
            PHPF_PriQ[i].priority);
    }
    printf("\n\n");
}

void handler_phpf(int sig_num){
    struct processData removed=currently_running_phpf;
    //printf("Process %d terminated at time %d\n",removed.id,getClk());
    signal (SIGUSR1,handler_phpf);
    currently_running_phpf=removeHighestPriority();
    printPHPFQueue();
}

