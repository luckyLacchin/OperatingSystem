//let's start :)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_BUFFER 255

void repeaterHandler (int signo, siginfo_t *info, void *ctx);
void loggerHandler (int signo, siginfo_t *info, void *ctx);

typedef struct {
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
} msgBuf;


int fd;  //we need it as global variable to use it also in the handler

//mi manca solo il queueChecker

int main (int argc, char *argv[]) {

    if (argc != 2) {
        printf("Inserire un solo parametro");
        exit(0);
    }
    char pathToLogFile [MAX_BUFFER];
    strcpy(pathToLogFile, argv[1]);

    //iniziamo con il Repeater
    struct sigaction rep;
    rep.sa_flags = SA_SIGINFO;
    rep.sa_sigaction = repeaterHandler;
    sigemptyset(&rep.sa_mask);
    sigaction(SIGUSR1, &rep, NULL);
    sigaction(SIGUSR2, &rep, NULL);
    sigaction(SIGINT, &rep, NULL);
    
    struct sigaction log;
    log.sa_flags = SA_SIGINFO;
    log.sa_sigaction = loggerHandler;
    sigemptyset(&log.sa_mask);
    sigaction(SIGUSR1, &log, NULL);
    sigaction(SIGUSR2, &log, NULL);
    
    fd = open(pathToLogFile, O_RDWR | O_APPEND, 0666); //S_IRUSR | S_IWUSR and 0666 should have the same meaning

    key_t key = ftok (pathToLogFile, 1);
    int queue = msgget (key, IPC_CREAT);

    msgBuf msg;
    while (1) {//dobbiamo continuare a vedere se qualcuno ha mandato un messaggio sulla coda
        msgrcv(queue,&msg,sizeof(msg.mtext),0,0); //Se non sono presenti messaggi, la chiamata si blocca in loro attesa. Il flag  IPC_NOWAIT fa fallire la syscall se non sono presenti messaggi

        pid_t pid = atoi(msg.mtext);
        kill(SIGALRM,pid);
    }
    
    return 0;
}

void repeaterHandler (int signo, siginfo_t *info, void *ctx) {
    
    //info->si_pid dovrebbe essere il segnale mandante
    char buffer [MAX_BUFFER];

    if (signo == SIGUSR1) {
        kill(SIGUSR1, info->si_pid);
    }
    else if (signo == SIGUSR2) {
        pid_t isChild = fork();
        if (isChild == 0) {
            //mando il segnale da un altro processo, ovvero il figlio
            kill(SIGUSR1, info->si_pid);
        }
    }
    else if (signo==SIGINT) {
        sprintf(buffer, "stop\n");
        write (fd,buffer, sizeof(buffer));
    }



}

void loggerHandler (int signo, siginfo_t *info, void *ctx) {
    
    //info->si_pid dovrebbe essere il segnale mandante
    char buffer [MAX_BUFFER];
    sleep(3);
    sprintf(buffer, "%d-%d\n",info->si_pid,signo);
    write (fd,buffer, sizeof(buffer)); 

}