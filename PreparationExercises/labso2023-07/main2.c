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
        printf("gestiamo SIGUSR1 inviato da %d e lo inviamo da qui %d",siginfo->si_pid, getpid());
        fflush(stdout);
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
        printf("gestiamo SIGUSR1");
        fflush(stdout);
    }
}

void sigWinchHandler (int signo) {

    goOn = 1;
}

void finishedReading (int signo) {
    //handler per indicare che ha finito di leggere il child
    isReading = 0;
    sleep(1); //sleep tra una scrittura e l'altra
}

int main (int argc, char *argv[]) {

    int thisProcess;
    if (argc != 4) {
        fprintf(stderr, "Inserire ./PostOffice 10 /tmp/ciao.txt 1450 template");
    }
    errno = 0;
    int n =  strtol (argv[1], NULL, 10); //string, something that we don't care about and the base for the conversion, so for us is always 10
    if (errno != 0 || !(n >= 1 && n <= 10)) {
        fprintf(stderr, "Inserire n tra 1 e 10");
        exit(0);
    }
    int pipes [n] [2];
    int children [n];
    pid_t pidInput = strtol(argv[3],NULL,10);
    msgBuf msg;
    pidFather = getpid();
    errno = 0;
    key_t key = ftok(argv[2],pidFather);
    if (errno != 0) {
        printf("There is something wrong in the key\n");
    }
    queue = msgget (key, 0777 | IPC_CREAT); //dopo la prima volta che viene create non viene più creata
    printf("Queue: %d, Key: %d, Path: %s\n", queue,key,argv[2]);
    printf("il pid del padre è: %d\n",pidFather);
    fflush(stdout);
    pid_t isChild;
    /*
    if (errno != 0) {
        perror("E' stato inserito un pid invalido");
        exit(1);
    }
    */
    //adesso dovrei fare un controllo sul path
    FILE *file = fopen(argv[2],"r");
    if (file == NULL) {
        printf("%s",argv[2]);
        perror("Il file non esiste, dare un path valido");
        exit(2);
    }
    strcpy(path, argv[2]);

    for (int i = 0; i < n; i++) {
        pipe(pipes[i]); //fuck i think that the fact i was creating the pipe inside the if was a fucking problem...I have to open the pipe before the fork!!!
        children[i] = fork();
        if (getpid() == pidFather) {
            close (pipes[i][RD]);
            fprintf(stdout, "child pid: %d\n", children[i]);
            fflush(stdout);
        }
        else if (children[i] == 0) { //it means that we are in the child process
            struct sigaction sa;
            sigemptyset(&sa.sa_mask);
            sa.sa_sigaction = childHandler;
            sigaction(SIGUSR1, &sa, NULL);
            sigaction(SIGUSR2, &sa, NULL);
            printf("Invio segnale da %d a %d\n",getpid(),pidInput);
            int res = kill(SIGTERM, pidInput);
            printf ("res = %d, errore: %d\n", res,errno);
            close(pipes[i][WR]);
            thisProcess = i;
            //fprintf(stdout, "%s\n", "siamo nel child");
            fflush(stdout);
            break;//NB: te lo eri completamente scordato
        }
    }

    if (getpid() == pidFather) {
        struct sigaction sa3;
        sigemptyset(&sa3.sa_mask);
        sa3.sa_handler = sigWinchHandler;
        sa3.sa_flags = 0;
        sigaction(SIGWINCH , &sa3, NULL);

        while (goOn == 0) {
            pause();
            break;
        }
    }

    if (getpid() == pidFather) {
        struct sigaction sa2;
        sigemptyset(&sa2.sa_mask);
        sa2.sa_handler = finishedReading; //handler usato per avvisare il father di aver finito di leggere la pipe e aver inviato il msg da parte del figlio
        sigaction(SIGUSR1, &sa2, NULL);
        
        while (!feof(file)) {
            if (isReading == 0) {
                char buf [MAX_BUFFER];
                fgets(buf, sizeof(buf), file);
                //ssize_t write(int fd, const void buf[.count], size_t count);
                printf("almost writing: %s\n", buf);
                int out = write(pipes[turn%n][WR],buf, strlen(buf)+1);
                printf("i have written to %d with out = %d\n ", turn%n, out);
                fflush(stdout);
                ++turn;
                isReading == 1; //serve ad indicare che un child is reading
            }
        
        }

        if (feof(file)) {
            printf("%s\n","siamo qui");
            fflush(stdout);
            for (int i = 0; i < n; i++) {
                //kill(SIGKILL,children[i]); //per killare i children alla fine della lettura di tutto il testo
            }
            //return 0; //non servirebbe in realtà
        }  
    }   
    else {
        while (1) {
        
            //devo cominciare a leggere la pipe, se ricevo qualcosa, la mando nella queue e dopo devo avvisare il pidFather
            //ssize_t read(int fd, void buf[.count], size_t count);
            //int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
            
            msgBuf msg;
            char buf [MAX_BUFFER];
            printf("siamo qui con process %d pipes %d\n", thisProcess, pipes[thisProcess][RD]);
            fflush(stdout);
            int outcome = read(pipes[thisProcess][RD],buf, sizeof(buf)); //in realtà dovrei salvare il curIndex quando sto creando i figli!
            printf("outcome = %d, HO LETTO: %s\n",outcome,buf);
            fflush(stdout);
            strcpy(msg.mtext,buf);
            msg.mtype = getpid();
            outcome=msgsnd(queue,&msg,sizeof(msg.mtext),0);
            printf("messaggio inviato: %s , da child %d con OUTCOME: %d\n", msg.mtext, thisProcess,outcome);
            fflush(stdout);
            kill(SIGUSR1,pidFather);
            
        }
    }

    
    while(1) {
        if (msgrcv(queue,&msg,sizeof(msg.mtext),0,0) != -1) {
            //vuol dire che posso leggere il messaggio, perché c'è nella mia coda
            fprintf(stdout, "MESSAGGIO NELLA CODA: %s\n", msg.mtext); //stampiamo i messaggi riga per riga
        }
    }
    return 0;
}

/*
errorone: nel for devo mettere dei break per i isChild, che disattenzione, fare debugging!
*/