#include "headers.h"

/* Modify this file as needed*/
struct PCB{
    int id;
    char* state;
    int arrivaltime;
    int total_running_time;
    int remaining_time;
    int priority;
    int processedtime;
};

struct PCB process_control;

struct exitcode{
    struct PCB exit;
    int mtype;
};


bool running=0;
int main(int agrc, char * argv[])
{
    process_control.processedtime=0;
    initClk();
    process_control.id=atoi(argv[1]);
    process_control.arrivaltime=atoi(argv[2]);
    process_control.priority=atoi(argv[3]);
    process_control.total_running_time=atoi(argv[4]);
    //printf("New process forked\n");
    //TODO it needs to get the remaining time from somewhere
    process_control.remaining_time = process_control.total_running_time;
    int current;
    
    pFile = fopen("log.txt", "a");
    fprintf(pFile,"At time %d process %d started | Arrival time:%d | Total Runtime:%d | Remaining Time:%d | Waiting Time:%d\n",getClk(),process_control.id,process_control.arrivaltime,process_control.total_running_time, process_control.remaining_time,getClk()-process_control.arrivaltime-process_control.processedtime);
    fclose(pFile);
    while (process_control.remaining_time > 0)
    {
        current=getClk();
        sleep(1);
        process_control.processedtime=process_control.processedtime+getClk()-current;
        current=getClk();
        process_control.remaining_time = process_control.total_running_time-process_control.processedtime;
    }
    pFile = fopen("log.txt", "a");
    int TA_time=getClk()-process_control.arrivaltime;
    float WTA;
    if(process_control.total_running_time==0){
        WTA=0;
    }
    else{
        WTA=TA_time/process_control.total_running_time;
    }
    int waiting_time=getClk()-process_control.arrivaltime-process_control.processedtime;
    fprintf(pFile,"At time %d process %d finished | Arrival time:%d | Total Runtime:%d | Remaining Time:%d | Waiting Time:%d | TA Time:%d | WTA: %.2f\n",getClk(),process_control.id,process_control.arrivaltime,process_control.total_running_time, process_control.remaining_time,waiting_time, TA_time,WTA);
    fclose(pFile);
    printf("Waiting Time:%d\n",waiting_time);
    //total_waiting=total_waiting+waiting_time;
    //printf("Total Waiting Time:%d\n",total_waiting);
    //printf("Waiting TIme:%d",getClk()-process_control.arrivaltime-process_control.processedtime);
    kill(getppid(),SIGUSR1);
    exit(0);
    
    destroyClk(false);
    
    return 0;
}
