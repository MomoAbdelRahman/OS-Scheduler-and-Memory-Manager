#include "headers.h"
#include <math.h>

int count=0;


int main(int argc, char * argv[])
{
    signal(SIGUSR2,handler_all_processes_sent);
    //printf("Scheduling type:%s\nQuantum:%s\n",argv[1],argv[2]);
    initClk();
    for(int i=0;i<256;i++){
        pcb_arr[i].id=0;
    }
    key_t id;
    id = ftok("keyfile",1);
    int msgid, received;
    msgid=msgget(id, 0666|IPC_CREAT);
    if(msgid==-1){
        perror("error in creatting message queue in scheduler");
    }
    struct msgbuff message;
    int scheduling_type=atoi(argv[1]);

    key_t idsp;
    idsp = ftok("keyfile2",getpid());
    int msgidsp, receivedsp;
    msgidsp=msgget(idsp, 0666|IPC_CREAT);
    if(msgidsp==-1){
        perror("error in creating message queue between scheduler and process (in scheduler)");
    }
    struct exitcode messagesp;
    

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
                count++;
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
            receivedsp=msgrcv(msgidsp, &messagesp, sizeof(messagesp.exit), getpid(),IPC_NOWAIT);
            if(receivedsp==-1){
               //perror("couldnt receive\n");
            }
            else{ 
                pcb_arr[messagesp.exit.id]=messagesp.exit;
            }
        }
    }
    //PHPF
    else if(scheduling_type==2){
        signal (SIGUSR1,handler_phpf);
        while(final_process_sent==0 || currently_running_phpf.id!=0 || PHPF_PriQSize!=0){
            received=msgrcv(msgid, &message, sizeof(message.data), 1,IPC_NOWAIT);
            if(received==-1){
                if(continue_process(currently_running_phpf)==-1){printf("Error in continuing process\n");};
            }
            else{
                count++;
                int new_pid=new_process(&message);
                message.data.pid=new_pid;
                if(PHPF_PriQSize==0&&currently_running_phpf.id==0){
                    currently_running_phpf=message.data;
                }
                else{
                    if(currently_running_phpf.priority<=message.data.priority){
                        if(stop_process(message.data)==-1){printf("Error in stopping process\n");};
                        PHPF_enqueue(message.data);
                    }
                    else{
                        //print stopped currently_running_phpf
                        if(stop_process(currently_running_phpf)==-1){printf("Error in stopping process\n");};
                        PHPF_enqueue(currently_running_phpf);
                        currently_running_phpf=message.data;
                    }
                }
                printPHPFQueue();
                //print_pcb();
            }
            receivedsp=msgrcv(msgidsp, &messagesp, sizeof(messagesp.exit), getpid(),IPC_NOWAIT);
            if(receivedsp==-1){
               //perror("couldnt receive\n");
            }
            else{
                pcb_arr[messagesp.exit.id]=messagesp.exit;
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
                count++;
                int new_pid=new_process(&message);
                message.data.pid=new_pid;
                if(currently_running_rr.id==0){
                    currently_running_rr=message.data;
                    //printf("%d\n",currently_running_rr.id);
                    printRRQueue();
                    //print_pcb();
                }
                else{
                    //printf("Enquing in non empty queue at %d\n",RRqueue.rear);
                    RRenqueue(message.data);
                    stop_process(message.data);
                    printRRQueue();
                    //print_pcb();
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
                    receivedsp=msgrcv(msgidsp, &messagesp, sizeof(messagesp.exit), getpid(),IPC_NOWAIT);
                    if(receivedsp==-1){
                        //perror("couldnt receive\n");
                    }
                    else{ 
                        pcb_arr[messagesp.exit.id]=messagesp.exit;
                    }
                }
                
                //sleep(2);
            }
            receivedsp=msgrcv(msgidsp, &messagesp, sizeof(messagesp.exit), getpid(),IPC_NOWAIT);
            if(receivedsp==-1){
            //perror("couldnt receive\n");
            }
            else{ 
                pcb_arr[messagesp.exit.id]=messagesp.exit;
            }

        }
    print_pcb();
    }
    total_waiting=0;
    total_WTA=0;
    float std_wta=0;
    for(int i=0;i<256;i++){
        if(pcb_arr[i].id!=0){
            
            int waiting=pcb_arr[i].finishtime-pcb_arr[i].arrivaltime-pcb_arr[i].total_running_time;
            total_waiting+=waiting;

            int TA_time=pcb_arr[i].finishtime-pcb_arr[i].arrivaltime;
            float WTA;
            if(pcb_arr[i].total_running_time==0){
                WTA=0;
            }
            else{
                WTA=TA_time/pcb_arr[i].total_running_time;
            }
            total_WTA+=WTA;
        }
    }
    float avg_waiting=1.0*total_waiting/count;
    float avg_WTA=1.0*total_WTA/count;

    for(int i=0;i<256;i++){
        if(pcb_arr[i].id!=0){
            int TA_time=pcb_arr[i].finishtime-pcb_arr[i].arrivaltime;
            float WTA;
            if(pcb_arr[i].total_running_time==0){
                WTA=0;
            }
            else{
                WTA=TA_time/pcb_arr[i].total_running_time;
            }
            std_wta+=(WTA-avg_WTA)*(WTA-avg_WTA);
        }
    }

    float utilization=0;
    for(int i=0;i<256;i++){
        if(pcb_arr[i].id!=0){
            utilization+=pcb_arr[i].total_running_time;
        }
    }
    utilization=utilization/getClk();

    std_wta=std_wta/count;
    std_wta=sqrt(std_wta);

    pFile = fopen("perf.txt", "a");
    
    fprintf(pFile,"Average Waiting Time:%.2f\n",avg_waiting);
    fprintf(pFile,"Average WTA:%.2f\n",avg_WTA);
    fprintf(pFile,"Standard Deviation of WTA:%.2f\n",std_wta);
    fprintf(pFile,"CPU Utilization:%.2f\n",utilization);
    //TODO implement the scheduler :)
    //upon termination release the clock resources
    printf("Terminating Scheduler\n");
    //destroyClk(true);
    exit(0);
}
