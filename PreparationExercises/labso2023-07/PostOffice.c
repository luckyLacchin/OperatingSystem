#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>
#include <stdbool.h>
//let's start with it!

#define MAX_SIZE 10
#define RD 0
#define WR 1
#define MAX_BUFFER 255

typedef struct {
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
}msgqueue;


void childHandler (int signo);
void fatherHandler (int signo);

bool first = false;
char path[MAX_BUFFER];
bool sendMsg = false;

int main (int argc, char *argv[]) {
    
    //i'm gonna use open to see if it exists
    //what does it mean that the pid is not valid?!
    fprintf(stdout, "what");
    pid_t isChild;
    int n = strtol(argv[1],NULL,10);
    int workers [MAX_SIZE];
    int pipes [MAX_SIZE] [2];
    int n_index = -1;
    pid_t father_pid = getpid();
    char buffer [MAX_BUFFER];
    int turn = 0;
    if (errno == ERANGE) {
        fprintf(stderr, "There was an error in the conversion of the input-n integer.");
    }
    else if (n < 1 && n > 10) {
        fprintf(stderr, "Write a number between 1 and 10.");
    }
    //else is corrrct
    printf("ciao2");
    FILE *file = fopen(argv[2],"r"); //i think that i've just to read it 
    if (file == NULL) {
        //there was an error open the file
        perror("Error opening the file, it doesn't exist");
    }
    strcpy(path,argv[2]);
    int father = strtol(argv[3],NULL,10);
    if (errno == ERANGE) {
        fprintf(stderr, "There was an error in the conversion of the father pid");
    }

    key_t key = ftok(path,father_pid);
    int queue = msgget(key,0777);
    printf("%d\n",key);
    
    msgqueue msg;
    for (int i = 0; i < n; i++) {
        workers[i] = fork();
        pipe(pipes[i]);
        if(workers[i]!= 0) {
            close(pipes[i][RD]);
            printf("ciao1");
            /*
            char buffer [MAX_BUFFER];
            sprintf(buffer, "%d",i)
            write(pipes[i][WR],buffer,sizeof(buffer));*/
        }
        else {
            //starting the child process
            printf("ciao");
            kill(SIGTERM,father);
            close(pipes[i][WR]); //i'm gonna write just fron the father process
            //read(pipes[i]...)
            n_index = i; //index of the pipe, without using the pipe immediately
            break; //perché devo uscire dal for
        }
    }
    //i think that i've to create a pipe to communicate is i

    if(getpid() == father_pid) { //here we do stuff only for the father

        //let's start with the point 4!
        struct sigaction saFather;
        sigemptyset(&saFather.sa_mask);
        saFather.sa_flags = SA_SIGINFO;
        saFather.sa_handler = fatherHandler;
        sigaction(SIGWINCH ,&saFather,NULL); //idk if i should add it also for the children

        while(1) {
            pause(); //waiting for a signal
            if(sendMsg) {
                while(!feof(file)) {
                    fgets(buffer,MAX_BUFFER,file);
                    write(pipes[0][WR],buffer,sizeof(buffer));
                    /*
                    strncpy(msg.mtext,buffer,strlen(buffer)-1); //così dovrei tagliare /n
                    msg.mtype = 0;
                    msgsnd(queue,msg,sizeof(msg),0);
                    */
                   ++turn;
                   sleep(1);
                }

                for (int i = 0; i<n;i++) {
                    kill(workers[i],SIGKILL); //con sigkill dovrei killarli direttamente
                }
                break;
            }
        }
    }

    else {

        struct sigaction saChild;
        sigemptyset(&saChild.sa_mask);
        saChild.sa_flags = SA_SIGINFO;
        saChild.sa_handler = fatherHandler;
        sigaction(SIGUSR1 ,&saChild,NULL);
        sigaction(SIGUSR2 ,&saChild,NULL);
        
        while(!first) {
            //i have to read the pipe
            read(pipes[n_index][RD],buffer,sizeof(buffer));
            strncpy(msg.mtext,buffer,strlen(buffer)-1); //così dovrei tagliare /n
            msg.mtype = 0;
            msgsnd(queue,&msg,sizeof(msg),getpid());
        }

    }


    



    return 0;
}

void childHandler (int signo) {

    if (signo == SIGUSR1 && !first) {
        kill(SIGUSR1,0); //idk how to set the pid, i have to modify the handler a bit
        first = true; //like this the while should stop
    }
    else if (signo == SIGUSR2) {
        kill(SIGUSR2,0);
    }
}

void fatherHandler (int signo) {

    if(signo == SIGWINCH ) {
        sendMsg = true;

    }
}

//credo che manchi l'ultimo punto ma son cotto...