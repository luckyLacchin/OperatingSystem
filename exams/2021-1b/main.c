#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PATH_MAXLEN 60
#define CHILDREN_MAX 10
#define PIDLEN 8

#define WRITE 1
#define READ 0

int keyFD;
int logFD;
int isChild = 0;
int queue;

typedef struct msg{
    long mtype;
    char mtext[2];
} Msg_packet;

void childFun();
void myHandler();

int main(int argc, char* argv[]){

    if(argc != 3){
        fprintf(stderr, "Invalid input\n");
        exit(1);
    }
    struct stat st;
    if(stat(argv[1],&st)){
        fprintf(stderr, "Path does not exist\n");
        exit(2);
    }

    pid_t pid = getpid();
    char pathInfo[PATH_MAXLEN];
    char pathKey[PATH_MAXLEN];
    char pathLog[PATH_MAXLEN];
    char tmpArg[PIDLEN];
    int n;
    int r = 0;
    key_t keyQueue;

    strcpy(pathInfo, argv[1]);
    strcat(pathInfo, "/info/\0");
    pathKey[0] = 0;
    pathLog[0] = 0;
    if(mkdir(pathInfo,0755)){
        fprintf(stderr, "Cannot create info directory\n"); //fallisce anche se esiste già la cartella
        exit(3);
    }
    strcpy(pathKey, pathInfo);
    strcat(pathKey, "key.txt\0");
    r = sprintf(tmpArg, "%d", (int)pid);
    tmpArg[r] = '\n';
    tmpArg[r+1] = '\0';
    keyFD = open(pathKey, O_CREAT | O_RDWR);
    chmod(pathKey, 0755);
    write(keyFD, tmpArg, strlen(tmpArg));

    keyQueue = ftok(pathKey, 32);

    queue = msgget(keyQueue, 0755 | IPC_CREAT);
    msgctl(queue, IPC_RMID, NULL);
    queue = msgget(keyQueue, 0755 | IPC_CREAT);

    Msg_packet msg;
    msg.mtype = 1;
    sprintf(tmpArg, "%d", (int)getpid());
    strcpy(msg.mtext, tmpArg);

    msgsnd(queue,&msg,sizeof(msg.mtext),0);
    
    n = atoi(argv[2]);

    for(int i = 0; i < n; i++){
        isChild = !fork();
        if(isChild){

            strcpy(pathLog, pathInfo);
            sprintf(tmpArg, "%d", (int)getpid());
            strcat(pathLog, tmpArg);
            strcat(pathLog, ".txt\0");
            //printf("%s\n", pathLog);
            logFD = open(pathLog, O_CREAT | O_APPEND | O_RDWR);
            chmod(pathLog, 0755);
            printf("%d ", getpid());
            fflush(stdout);
            childFun();

            break;

        } else{
            sleep(1);
        }
    }
    printf("\n");

    //while(1);

    return 0;
}

void childFun(){

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = myHandler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    while(1);
}

void myHandler(int sig){
    char tmpArg[PIDLEN];
    if(sig == SIGUSR1){
        write(logFD, "SIGUSR1\n", 8);
        //printf("test SIGUSR1 pid: %d\n", getpid());
        //fflush(stdout);
    } else if(sig == SIGUSR2){
        Msg_packet msgUsr;
        msgUsr.mtype = 1;
        sprintf(tmpArg, "%d", (int)getpid());
        strcpy(msgUsr.mtext, tmpArg);

        msgsnd(queue,&msgUsr,sizeof(msgUsr.mtext),0);
    }
}