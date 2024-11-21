#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int id;
int arrivaltime;
int priority;
int running_time;
int starttime;
int processedtime=0;

bool running=0;
int main(int agrc, char * argv[])
{
    initClk();
    id=atoi(argv[1]);
    arrivaltime=atoi(argv[2]);
    priority=atoi(argv[3]);
    running_time=atoi(argv[4]);
    printf("New process forked\n");
    //TODO it needs to get the remaining time from somewhere
    remainingtime = running_time;
    int current;
    while (remainingtime > 0)
    {
        current=getClk();
        sleep(1);
        processedtime=processedtime+getClk()-current;
        current=getClk();
        remainingtime = running_time-processedtime;
    }
    kill(getppid(),SIGUSR1);
    exit(0);

    
    //destroyClk(false);
    
    return 0;
}
