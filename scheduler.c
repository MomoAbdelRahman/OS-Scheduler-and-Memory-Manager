#include "headers.h"

struct msgbuff{
    long mtype;
    int mtext;
};

int main(int argc, char * argv[])
{
    printf("Scheduling type:%s\nQuantum:%s\n",argv[1],argv[2]);
  
    initClk();
    
    //TODO implement the scheduler :)
    //upon termination release the clock resources
    
    destroyClk(true);
}
