
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>



extern int errno;

#define WR 1
#define RD 0

typedef struct {
    long mtype;
    char mtext[255];
} MsgQueue;

//This is the worker handler for SIGUSR1 and SIGUSR2
void signalHandlerWorker(int signo, siginfo_t * info, void * ctx){
    //Send the signal back to the sender
    printf("Received signal %d from %d. Sending it back\n ",signo,info->si_pid);
    kill(info->si_pid,signo);
}

//In order to synchronize the reading of the file, we need to know when a process has finished reading.
//We will use SIGUSR2 for this purpose. Each worker will send a SIGUSR2 to the father once it has finished reading.
//The father will wait for the readOver variable to be set to the pid of the worker that has finished reading
pid_t readOver = 0; 
void signalHandlerFather(int signo, siginfo_t * info, void * ctx){
    printf("Received SIGUSR2 from %d. It has finished reading.\n",info->si_pid);
    readOver = info->si_pid;  //Use the pid of the worker that has finished reading
}

//This is the handler for SIGWINCH. Once we received it, we set a flag and proceed with reading the file
int canReadFile = 0;
void signalHandlerWinch(int signo){
    canReadFile = 1;
    printf("Received SIGWINCH\n");
}




int main(int argc, char ** argv){

    /************ ARGUMENT CHECKS ***********/    
    if(argc != 4){
        fprintf(stderr,"Numero argomenti errato");
        return 2;
    }

    //Get number of workers
    int workers = strtol(argv[1],NULL,10); //We could have used atoi, but strtol is safer
    if(workers < 1 || workers > 10){
        fprintf(stderr,"Numero lavoratori errato");
        return 3;
    }

    //Try to open the file
    FILE * file = fopen(argv[2],"r"); //We open only in read mode. If the file does not exist, fopen will return NULL
    if(file == NULL){
        fprintf(stderr,"Impossibile aprire il file");
        return 4;
    }

    //REtrieve the input pid
    pid_t pidInput = strtol(argv[3],NULL,10); //We could have used atoi, but strtol is safer
    
    if(
        errno == ERANGE || //Make sure that strtol did not fail
        kill(pidInput,0) == -1 //use a 0 signal to check if the pid is valid. This will return -1 if the pid is invalid and will not send any signal
    ){
        fprintf(stderr,"PID non valido");
        return 5;
    }

    pid_t father = getpid(); //We need the pid of the main process

    //Get the queue right away: it will be shared
    key_t queueKey = ftok(argv[2], father); //We use the file name and the father pid to generate the key. 
    if(queueKey == -1){
        fprintf(stderr,"Impossibile creare la chiave");
        return 5;
    }
    int queueId = msgget(queueKey ,0777 | IPC_CREAT); //We create the queue with full privileges using 0777 and IPC_CREAT. 
    if(queueId == -1){
        fprintf(stderr,"Impossibile creare la coda");
        return 5;
    }else{
        printf("Coda creata con chiave %d e id %d\n",queueKey,queueId);
    }

    /************ WORKERS CREATION ******************/
    //Create an array where to store the pids (it could be allocated dynamically)
    pid_t workersPid[10];

    int pipes[10][2]; //We are going to use pipes to communicate with the workers
    int curIndex = 0;

    for(short i = 0; i < workers; i++){ //Cycle to create the workers
        pipe(pipes[i]); //For every worker we create a pipe
        workersPid[i] = fork(); //We create the worker
        if(workersPid[i]==0){ //We are the child
            //Save the current index for accessing the right pipe
            curIndex = i;

            //Close pipe in write side
            close(pipes[i][WR]);
            //Break from the cycle if we are the worker so that only the father will continue creating workers
            break;
        }else{ //We are the father
            //Close reading pipe
            close(pipes[i][RD]);

            //Now the father only will continue and create more workers
        } 
    }
    /****************  END OF WORKERS CREATION ********/


    /************** FATHER OPERATIONS *************/
    if(getpid() == father){ //We are the father

        //Create a sigaction to handle the SIGUSR2 signal. This will be used internally by our workers
        //To communicate with the father (synchronisation!). It is a design choice!
        struct sigaction sa; 
        sa.sa_sigaction = signalHandlerFather; //We need the senders pid, so we use sigaction
        sa.sa_flags = SA_SIGINFO; 
        sigemptyset(&sa.sa_mask); 
        sigaction(SIGUSR2,&sa,NULL);

        //Assign a signal handler to SIGWINCH. This will initiate the reading of the file
        struct sigaction saWinch;
        saWinch.sa_handler = signalHandlerWinch; //We can use the normal handler
        sigemptyset(&saWinch.sa_mask);
        saWinch.sa_flags = 0;
        sigaction(SIGWINCH,&saWinch,NULL);
        
        //Go to sleep until signalled
        int turn = 0;
        while(1){
            pause(); //IT will be waken up by any signal, such as SIGWINCH
            if(canReadFile) break; //Receiving SIGWINCH sets this flag to 1
        }

        //Now we can read the file
        printf("Start reading the file!\n");

        //Enter a infinite loop to read the file. We will break when the file is over
        while(1){
            //Read one line and send it to the worker
            char buffer[255];
            int read = fscanf(file,"%s",buffer);

            //Check if we read something
            if(read > 0){
                printf("Sending '%s' to %d\n",buffer,workersPid[turn%workers]);

                //Use the right pipe to send the data to a certain worker. We use the modulo operator to cycle through the workers
                //Thus ensuring an even distribution of the data
                write(pipes[turn%workers][WR],buffer,strlen(buffer)+1);
                //Now we are going to wait for a signal from worker
            }
            //Check if we reached the end of the file
            else if(feof(file)){
                //File is over
                printf("File is over. Notify children\n");
                for(short i = 0; i<workers; i++){
                    //We write '0' to every worker to make them terminate.
                    //This is a design choice, and it could be done in many other ways. It might have some drawbacks
                    //If one line of the file is '0'. To avoid this we could do some additional operations
                    write(pipes[i][WR],"0",2);

                    //We close the pipe
                    close(pipes[i][WR]);
                }
                //Wait termination of all children
                while(wait(NULL)>0);
                return 0;
            }else{
                fprintf(stderr,"Errore nella lettura del file\n");
                return 6;
            }
            //We are going to wait for a signal from the worker. Until then we wait. ReadOver contains the pid of the worker that sent the signal
            while(!readOver) pause();   
            printf("Received signal from %d\n",readOver);

            //It would be best to check if we were expecting the signal from the last worker we contacted.
            if(readOver != workersPid[turn%workers]) return 5;
            
            //Go to next worker
            turn++;
            
            //Reset flag
            readOver = 0;

            //Wait one second before sending the next line
            sleep(1);
            
        }

        
    /************** WORKERS OPERATIONS *************/
    }else{
        //Send sigterm
        printf("Sending sigterm to %d\n",pidInput);
        kill(pidInput,15);
        
        //Set sigaction
        struct sigaction sa1,sa2; 
        sa1.sa_sigaction = signalHandlerWorker; 
        sa2.sa_sigaction = signalHandlerWorker; 
        sa1.sa_flags = SA_RESETHAND | SA_SIGINFO; //Reset handler for SIGUSR1 so that after the first signal we will not handle it again and exit
        sa2.sa_flags = SA_SIGINFO;
        sigemptyset(&sa1.sa_mask); 
        sigemptyset(&sa2.sa_mask); 
        sigaction(SIGUSR1,&sa1,NULL);
        sigaction(SIGUSR2,&sa2,NULL);


        //Message to be sent on queue. We use our pid as mtype
        MsgQueue lineToBeSent = {getpid(),""};

        char buffer[255];
        int outcome = 0;
        while(1){
            //It blocks the program on the pipe read. If a signal is received we 
            //will be interrupted, we will handle the signal and then we will
            //continue the loop.
            outcome = read(pipes[curIndex][RD],buffer,255);
            if(outcome == -1){
                fprintf(stderr,"Errore nella read");
            }else if(strcmp(buffer,"0") != 0){
                printf("I'm %d and I can send the message\n",getpid());

                //Copy the message from the pipe into the message queue
                strncpy(lineToBeSent.mtext,buffer,255);

                //Send the message
                msgsnd(queueId,&lineToBeSent,sizeof(lineToBeSent.mtext),0);
                
                printf("Sent '%s' from %d\n",lineToBeSent.mtext,getpid());

                //Notify the father that we are done. We choose to use SIGUSR2.
                kill(father,SIGUSR2);
            }else{
                printf("We have received '0', let's terminate\n");
                //We have to terminate!
                //We close the pipe and return
                close(pipes[curIndex][RD]);
                return 0;
            }
        }
    }
}

//PARAMETRI MAX
//permessi coda
//kill solo i lavoratori