#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int starttime;
int processedtime=0;

bool running;
int main(int agrc, char * argv[])
{
    initClk();
    
    
    
    //When Process is Interrupted
    
    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        if(running){
            processedtime=processedtime+getClk()-starttime;
        }
        else if(!running){
            //Wait for signal
            starttime=getClk();
        }

        // remainingtime = ??;
    }
    
    //destroyClk(false);
    
    return 0;
}
