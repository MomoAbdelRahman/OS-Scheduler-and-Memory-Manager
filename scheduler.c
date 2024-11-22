#include "headers.h"





int main(int argc, char * argv[])
{
    printf("Scheduling type:%s\nQuantum:%s\n",argv[1],argv[2]);
    
    initClk();
    key_t id;
    id = ftok("keyfile",1);
    int msgid, received;
    msgid=msgget(id, 0666|IPC_CREAT);
    if(msgid==-1){
        perror("error in creatting message queue in scheduler");
    }
    struct msgbuff message;
    int scheduling_type=atoi(argv[1]);
    if(scheduling_type==1){
        //SJF
        signal (SIGUSR1,handler_sjf);
        while(1){
        received=msgrcv(msgid, &message, sizeof(message.data), 1,IPC_NOWAIT);
        if(received==-1){
            //printf("Continuing Process");
            continue_process(currently_running_sjf);
            //printf("Didnt Receive Data from Process generator");
        }
        else{
            //printf("received: %d\n",message.data.id);
            int new_pid=new_process(&message);
            message.data.pid=new_pid;
            if(sjf_queueSize==0&&currently_running_sjf.id==0){
                currently_running_sjf=message.data;
            }
            else{
                sjf_enqueue(message.data);
                stop_process(message.data);
            }
            printsjfQueue();
        }

        
    }
    }
    else if(scheduling_type==2){
        //PHPF
        signal (SIGUSR1,handler_phpf);
        while(1){
        received=msgrcv(msgid, &message, sizeof(message.data), 1,IPC_NOWAIT);
        if(received==-1){
            continue_process(currently_running_phpf);
        }
        else{
            int new_pid=new_process(&message);
            message.data.pid=new_pid;
            if(PHPF_PriQSize==0&&currently_running_phpf.id==0){
                currently_running_phpf=message.data;
            }
            else{
                PHPF_enqueue(message.data);
                stop_process(message.data);
                stop_process(currently_running_phpf);
                PHPF_enqueue(currently_running_phpf);
                currently_running_phpf=removeHighestPriority();
                continue_process(currently_running_phpf);
            }
            printPHPFQueue();
        }
       
    }
    }
    else if(scheduling_type==3){
        //RR
        int quantum=atoi(argv[2]);
        printf("%d\n",RRqueue.front);
        signal (SIGUSR1,handler_rr);
        while(1){
            received=msgrcv(msgid, &message, sizeof(message.data), 1,IPC_NOWAIT);


            if(received==-1){
                //continue_process(currently_running_rr);
            }
            else{
                int new_pid=new_process(&message);
                message.data.pid=new_pid;
                if(RRisEmpty()&&currently_running_rr.id==0){
                    currently_running_rr=message.data;
                    printf("%d\n",currently_running_rr.id);
                }
                else{
                    RRenqueue(message.data);
                    stop_process(message.data);
                    printf("etlam:%d\n",message.data.id);
                }
            }
            if(currently_running_rr.id!=0){
                RRenqueue(currently_running_rr);
                stop_process(currently_running_rr);
                currently_running_rr=RRdequeue();
                continue_process(currently_running_rr);
                for(int i=0;i<quantum;i++){
                    if(dead){
                        dead=false;
                        break;
                    }
                    else{
                        sleep(1);
                    }
                }
                printRRQueue();
                sleep(2);
            }
        }

    
    //TODO implement the scheduler :)
    //upon termination release the clock resources

    //destroyClk(true);
    }
}
