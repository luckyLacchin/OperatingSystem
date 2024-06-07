//iniziamo a fare sti due coglioni

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

//msgsnd(qid, &msg, sizeof(msg.mtext),IPC_NOWAIT)
//int msgsnd(int msqid, const void msgp[.msgsz], size_t msgsz,int msgflg);

#define MAX_BUFFER 255

int nextCommand = 1;

typedef struct {
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
}msgbuf;

typedef struct {
    int signo;
    pid_t pid;
} killArgs;

typedef struct {
    msgbuf msg;
    int queue;
} queueArgs;

typedef struct {
    char name[MAX_BUFFER];
    char word[MAX_BUFFER];
} fifoArgs;

void *ThreadKill (void *param) {
    killArgs *killPayLoad = (killArgs *) param;

    if(killPayLoad->signo != -2) { 
        kill(killPayLoad -> pid,killPayLoad -> signo);
        nextCommand = 0;
        killPayLoad->signo = -2;
    }
}
void * ThreadQueue (void *param) {
    queueArgs *queuePayLoad = (queueArgs *) param;

    if(queuePayLoad->queue != -2) {
        msgsnd(queuePayLoad->queue,&queuePayLoad->msg,sizeof(queuePayLoad->msg.mtext),0);
        queuePayLoad->queue = -2;
        nextCommand = 0;
    }

    
}
void * ThreadFifo (void *param) {

    fifoArgs *fifoPayLoad = (fifoArgs *) param;

    if(strcmp(fifoPayLoad->word,"") != 0) {
        mkfifo(fifoPayLoad->name,S_IWUSR | S_IRUSR);
        int fd = open(fifoPayLoad->name,O_APPEND);
        write(fd,fifoPayLoad->word,sizeof(fifoPayLoad->word));
        strcpy(fifoPayLoad->word, "");
        nextCommand = 0;
    }
}

void myHandler (int signo) {

    if (signo == SIGUSR1) { // non credo serva questo controllo, ma lo mettiamo lo stesso per sicurezza!
        nextCommand = 1;
    }

}

int main (int argc, char *argv[]) {

    int n = argc;
    if(n != 2) {
        fprintf(stderr, "Inserire /path/to/file.txt");
    }
    char path [MAX_BUFFER];
    strcpy(path,argv[1]);
    FILE *file = fopen(path,"r");
    char buffer [3][MAX_BUFFER];

    //potrei usare errno, ma prima devo fare la sua init
    //errno =  0;
    /*
    while (!feof(file)) {
        fscanf(file, "%s %s %s", buffer[0], buffer[1], buffer[2]);
 
        if (strcmp(buffer[0], "kill") == 0) {
            pid_t id = atoi(buffer[2]); //i have an error for the vs compiler not in the container!
            int signo = atoi (buffer[1]);
            kill(id,signo);
        }
        else if (strcmp(buffer[0], "queue") == 0) {
            int key = ftok(path,1);
            int queue = msgget(key,IPC_CREAT);
            msgbuf msg;
            msg.mtype = atoi(buffer[1]);
            strcpy(msg.mtext, buffer[2]);
            msgsnd(queue,&msg,sizeof(msg.mtext),0);
        }
        else if (strcmp(buffer[1], "fifo") == 0) {
            mkfifo(buffer[1],S_IWUSR | S_IRUSR);
            int fd = open(buffer[1],O_APPEND);
            write(fd,buffer[2],sizeof(buffer[2]));
        }
    }
    int pthread_create(pthread_t *restrict thread,const pthread_attr_t *restrict attr,void *(*start_routine)(void *),void *restrict arg);
    */
    pthread_t th1, th2, th3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    killArgs killPayLoad;
    queueArgs queuePayLoad;
    fifoArgs fifoPayLoad;

    sigset_t attr_mask;
    sigemptyset(&attr_mask);
    sigaddset(&attr_mask,SIGUSR1);

    //int pthread_attr_setsigmask_np(pthread_attr_t *attr,const sigset_t *sigmask);
    thread_attr_setsigmask_np(&attr,&attr_mask);

    pthread_create(&th1, &attr, ThreadKill, (void *) &killPayLoad);
    pthread_create(&th2, &attr, ThreadQueue, (void *) &queuePayLoad);
    pthread_create(&th3, &attr, ThreadFifo, (void *) &fifoPayLoad);

    //pthread_attr_destroy(&attr) non so se effettivamente serva
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = myHandler;
    sigaction(SIGUSR1,&sa, NULL);

    while (!feof(file)) {
        if (nextCommand == 1) {
            fscanf(file, "%s %s %s", buffer[0], buffer[1], buffer[2]);
    
            if (strcmp(buffer[0], "kill") == 0) {
                killPayLoad.pid = atoi(buffer[2]); //i have an error for the vs compiler not in the container!
                killPayLoad.signo = atoi (buffer[1]);
            }
            else if (strcmp(buffer[0], "queue") == 0) {
                int key = ftok(path,1);
                int queue = msgget(key,IPC_CREAT);
                queuePayLoad.msg.mtype = atoi(buffer[1]);
                strcpy(queuePayLoad.msg.mtext, buffer[2]);
            }
            else if (strcmp(buffer[1], "fifo") == 0) {
                strcpy(fifoPayLoad.name, buffer[1]);
                strcpy(fifoPayLoad.word, buffer[2]);
            }
        }
    }

    fclose(file);    

    return 0;   
}