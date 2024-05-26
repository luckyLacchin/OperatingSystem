//This is a possible solution to the problem. It is not the only one, and it might not be the best one
//It leverages local variables to communicate with the threads, avoiding the use of global variables

#include <stdio.h>
#include <pthread.h> 
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>

#define MAX_BUF 255

// Define the various types. We will use local variables to communicate with the threads
// The data will be stored in the payload, i.e. a struct containing the data and a flag.
// The flag will be used to check if the thread has to do perform the action, and it will
// be set by the incoming SIGUSR1.
typedef struct{
    pid_t dest;
    int signo;
} PayloadKill; //Payload for the kill thread

typedef struct{
    long mtype;
    char mtext[MAX_BUF];
} Msg; // Message for the queue

typedef struct{
    Msg message;
    char * queueName;
} PayloadQueue; // Payload for the queue thread

typedef struct{
    char fifoName[MAX_BUF];
    char msg[MAX_BUF];
    char canSend;
} PayloadFifo; // Payload for the fifo thread


//Handler for the kill command
void * threadKill(void * arg){
    //Create pointer to payload received from argument
    PayloadKill * payload = (PayloadKill *)arg;
    //Cycle until program termination
    while(1){
        //Use the destination as a flag to check if the thread has to send a signal
        if(payload->dest != 0){

            //Send signal
            kill(payload->dest,payload->signo);

            //Change flag back to 0
            payload->dest = 0;
        }
        sleep(1);
    }
}

//Handler for the queue command
void * threadQueue(void * arg){
    
    //Create pointer to payload received from argument
    PayloadQueue * payload = (PayloadQueue *)arg;

    //Create the queue using the queueName from the argument
    key_t qkey = ftok(payload->queueName,1);

    //Create queue only if it doesn't exist, otherwise open it
    int queue = msgget(qkey, 0777 | IPC_CREAT);

    
    
    //Cycle until program termination
    while(1){

        //Use the message type as a flag to check if the thread has to send a message
        if(payload->message.mtype != -1){

            //Send message
            msgsnd(queue,&payload->message,sizeof(payload->message.mtext),0);
            //Reset flag
            payload->message.mtype = -1;
        }
        sleep(1);
    }
}

//Handler for the fifo command
void * threadFifo(void * arg){
    //Create pointer to payload received from argument
    PayloadFifo * payload = (PayloadFifo *)arg;

    //Cycle until program termination
    while(1){

        //Check if the thread has to send a message
        if(payload->canSend == 1){

            //Create a new fifo with the name from the payload
            int created = mkfifo(payload->fifoName, S_IRUSR | S_IWUSR);
            
            //Open the fifo
            int fd = open(payload->fifoName,O_WRONLY);

            //Write on the fifo (this will block until it is read)
            write(fd,payload->msg,strlen(payload->msg));

            //REset flag
            payload->canSend = 0;

            //Close the fifo
            close(fd);
        }
        sleep(1);
    }
}

//Global variable to check if we have received a SIGUSR1
char nextCommand = 1;

//Signal handler for SIGUSR1
void handler(int signo, siginfo_t * info, void * empty){
    
    //Set the global flag to 1
    nextCommand = 1;
}


int main(int argc, char ** argv){
    //Check if the number of arguments is correct
    if(argc != 2){
        fprintf(stderr,"Errore negli argomenti\n");
        return 1;
    }
    
    //Open file with file stream in read mode
    FILE * file = fopen(argv[1],"r");
    if(file == NULL){
        fprintf(stderr,"Errore nell'apertura del file\n");
        return 2;
    }
    
    //Create a sigacction struct to handle SIGUSR1
    struct sigaction sa;
	sa.sa_sigaction = handler;

    //Empty the mask for the handler
	sigemptyset(&sa.sa_mask); 
    //Set the flag to use the siginfo_t struct
	sa.sa_flags = SA_SIGINFO;

    //Assign the handler to SIGUSR1
    sigaction(SIGUSR1,&sa,NULL);

    //Create the payloads for the threads. Initialise them with the flag set to 0 or -1
    PayloadKill PayloadKill = {0,0};
    PayloadQueue payloadQueue = {{-1,""},argv[1]};
    PayloadFifo payloadFifo = {"","",0};

    //Create the threads ids
    pthread_t th_k, th_q, th_f;

    //Create a new signal mask for the threads, so that they will block SIGUSR1
    sigset_t setBlocked;
    sigemptyset(&setBlocked);
    sigaddset(&setBlocked,SIGUSR1);
    
    //Create the attributes for the threads
    pthread_attr_t attrk;
    //Initialise the attributes and set the signal mask
    pthread_attr_init(&attrk);
    pthread_attr_setsigmask_np(&attrk,&setBlocked);

    //Create the threads. They will keep running until the program is terminated
    //WE pass them the address of the local variables (payloads), which we will
    //use to communicate with them
    pthread_create(&th_k,&attrk,threadKill,(void *)&PayloadKill);
    pthread_create(&th_q,&attrk,threadQueue,(void *)&payloadQueue);
    pthread_create(&th_f,&attrk,threadFifo,(void *)&payloadFifo);
    
    //Create a buffer to store the three commands from the file
    char buf[3][MAX_BUF];
    
    //Cyccle the file until the end
    while(!feof(file)){
        //If we have received a SIGUSR1, parse the next command
        if(nextCommand == 1){
            //Parse a line and store the 3 string in the buffers
            int matched = fscanf(file,"%s %s %s",buf[0],buf[1],buf[2]);

            //Check which command was parsed and set the payload accordingly
            if(strcmp(buf[0],"kill") == 0){
                PayloadKill.signo = atoi(buf[1]);
                PayloadKill.dest = atoi(buf[2]); //This will function as the flag
                
            }
            if(strcmp(buf[0],"queue") == 0){
                payloadQueue.message.mtype = atoi(buf[1]); //This will function as the flag
                strncpy(payloadQueue.message.mtext,buf[2],sizeof(payloadQueue.message.mtext));
            }
            if(strcmp(buf[0],"fifo") == 0){
                strncpy(payloadFifo.fifoName,buf[1],sizeof(payloadFifo.fifoName));
                strncpy(payloadFifo.msg,buf[2],sizeof(payloadFifo.msg));
                payloadFifo.canSend = 1; //This will function as the flag
            }
            //Reset the flag
            nextCommand = 0;
        }   
        sleep(1);
    }
    //Close the file 
    fclose(file);
    return 0; //This will terminate the program and the threads
    

}