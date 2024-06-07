//iniziamo a fare questa morte
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX_BUFFER 255
#define RD 0
#define WR 1

typedef struct{
    long   mtype;       /* Message type. */
    char   mtext[1];    /* Message text. */
} msgBuf;

pid_t pidFather;
int goOn = 0;
char path[MAX_BUFFER];
int turn = 0; //serve a tener conto a chi stiamo inviando il messaggio da scrivere
int isReading = 0;
int queue = -1;

void childHandler (int signo, siginfo_t *siginfo, void *csv) {

    if (signo == SIGUSR1) {
        kill(SIGUSR1, siginfo -> si_pid);
        //ogni successivo SIGUSR1 dovrà essere gestito con il comportamento di default
        /*
        struct sigaction sa_default;
        sa_default.sa_handler = SIG_DFL;
        sigemptyset(&sa_default.sa_mask);
        sa_default.sa_flags = 0;
        sigaction(SIGUSR1, &sa_default, NULL);
        */
       signal(SIGUSR1, SIG_DFL);
    }
    else if (signo == SIGUSR2) {
        kill(SIGUSR2, siginfo->si_pid);
    }
}

void sigWinchHandler (int signo) {
    key_t key = ftok(path,pidFather);
    queue = msgget (key, 0777); //dopo la prima volta che viene create non viene più creata
    goOn = 1;
}

void finishedReading (int signo) {
    //handler per indicare che ha finito di leggere il child
    isReading = 0;
    sleep(1); //sleep tra una scrittura e l'altra
}

int main (int argc, char *argv[]) {


    if (argc != 4) {
        fprintf(stderr, "Inserire ./PostOffice 10 /tmp/ciao.txt 1450 template");
    }
    errno = 0;
    int n =  strtol (argv[1], NULL, 10);
    if (errno != 0 || !(n >= 1 && n <= 10)) {
        fprintf(stderr, "Inserire n tra 1 e 10");
        exit(0);
    }
    int workers [n] [2];
    int children [n];
    pid_t pidInput = strtol(argv[3],NULL,10);
    pidFather = getpid();
    if (errno != 0) {
        perror("E' stato inserito un pid invalido");
        exit(1);
    }

    //adesso dovrei fare un controllo sul path
    FILE *file = fopen(argv[1],"r");
    if (file == NULL) {
        perror("Il file non esiste, dare un path valido");
        exit(2);
    }
    strcpy(path, argv[1]);
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigWinchHandler;
    sigaction(SIGWINCH , &sa, NULL);

    for (int i = 0; i < n; i++) {
        pid_t isChild = fork();
        if (getpid() == pidFather) {
            pipe(workers[i]);
            close (workers[i][RD]);
            children[i] = isChild;
        }
        else if (isChild == 0) { //it means that we are in the child process
            struct sigaction sa;
            sigemptyset(&sa.sa_mask);
            sa.sa_sigaction = childHandler;
            sigaction(SIGUSR1, &sa, NULL);
            sigaction(SIGUSR2, &sa, NULL);
            kill(SIGTERM, pidInput);
            close(workers[i][WR]);
        }
    }

    while (goOn == 0) {
        pause();
    }
    //quando finisco il while vuol dire che ho creato la coda

    if (getpid() == pidFather) {
        struct sigaction sa2;
        sigemptyset(&sa2.sa_mask);
        sa.sa_handler = finishedReading; //handler usato per avvisare il father di aver finito di leggere la pipe e aver inviato il msg da parte del figlio
        sigaction(SIGUSR1, &sa, NULL);
        
        while (!feof(file)) {
            if (isReading == 0) {
                char buf [MAX_BUFFER];
                fgets(buf, sizeof(buf), file);
                //ssize_t write(int fd, const void buf[.count], size_t count);
                write(workers[turn%n][WR],buf, sizeof(buf));
                ++turn;
                isReading == 1; //serve ad indicare che un child is reading
            }
        
        }

        if (feof(file)) {
            for (int i = 0; i < n; i++) {
                kill(SIGKILL,children[i]); //per killare i children alla fine della lettura di tutto il testo
            }
            return 0; //non servirebbe in realtà
        }  
    }   
    else {
        while (1) {
            //devo cominciare a leggere la pipe, se ricevo qualcosa, la mando nella queue e dopo devo avvisare il pidFather
            //ssize_t read(int fd, void buf[.count], size_t count);
            //int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
            msgBuf msg;
            char buf [MAX_BUFFER];
            read(workers[turn%n][RD],buf, sizeof(buf));
            strcpy(msg.mtext,buf);
            msg.mtype = getpid();
            msgsnd(queue,&msg,sizeof(msg.mtext),0);
            kill(SIGUSR1,pidFather);

        }
    }


    return 0;
}