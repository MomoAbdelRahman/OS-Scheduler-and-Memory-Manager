#include "headers.h"



int main(int argc, char * argv[])
{
    signal(SIGUSR2,handler_all_processes_sent);
    //printf("Scheduling type:%s\nQuantum:%s\n",argv[1],argv[2]);
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
    

    //SJF
    if(scheduling_type==1){
        signal (SIGUSR1,handler_sjf);
        while(final_process_sent==0 || currently_running_sjf.id!=0 || sjf_queueSize!=0){
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
    //PHPF
    else if(scheduling_type==2){
        signal (SIGUSR1,handler_phpf);
        while(final_process_sent==0 || currently_running_phpf.id!=0 || PHPF_PriQSize!=0){
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
    //RR
    else if(scheduling_type==3){
        int quantum=atoi(argv[2]);
        //printf("%d\n",RRqueue.front);
        signal (SIGUSR1,handler_rr);
        while(final_process_sent==0 || currently_running_rr.id!=0){
            received=msgrcv(msgid, &message, sizeof(message.data), 1,IPC_NOWAIT);

            if(received==-1){
                //continue_process(currently_running_rr);
            }
            else{
                int new_pid=new_process(&message);
                message.data.pid=new_pid;
                if(currently_running_rr.id==0){
                    currently_running_rr=message.data;
                    //printf("%d\n",currently_running_rr.id);
                    printRRQueue();
                }
                else{
                    //printf("Enquing in non empty queue at %d\n",RRqueue.rear);
                    RRenqueue(message.data);
                    stop_process(message.data);
                    printRRQueue();
                    //printf("etlam:%d\n",message.data.id);
                }
            }
            if(currently_running_rr.id!=0){
                stop_process(currently_running_rr);
                RRenqueue(currently_running_rr);
                //printf("Current Rear:%d\n",RRqueue.rear);
                currently_running_rr=RRdequeue();
                continue_process(currently_running_rr);
                //printf("Current Rear:%d\n",RRqueue.rear);
                for(int i=0;i<quantum;i++){
                    //printf("dead: %d\n",dead);
                    if(dead==1){
                        dead=0;
                        break;
                    }
                    else{
                        sleep(1);
                    }
                }
                
                //sleep(2);
            }
            
        }
    }
    //TODO implement the scheduler :)
    //upon termination release the clock resources
    printf("Terminating Scheduler\n");
    //destroyClk(true);
    exit(0);
}
