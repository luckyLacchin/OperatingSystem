//let's start with it :)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define MAX_SIZE 255
#define MAX_CHILDREN 10 //non so dove trovare questa info

int fatherPid;
FILE *file;
int n_children = 0;
int children [MAX_CHILDREN];

void clientHandler (int signo) {
    fprintf(stdout, "[server:%d]",getpid());
    if (signo == SIGUSR1) {
        children[n_children] = fork();
        if (children[n_children] == 0) { //vuol dire che siamo nei figli
            n_children = 0;
            for (int i = 0; i < MAX_CHILDREN; i++) {
                children[i] = -1;
            }//reset di tutto per stare a posto
        }   
        else {
            fprintf(file,"+%d",children[n_children]); //questo pid!=0 perché sono nel processo padre
            fprintf(stdout,"[server]+%d",children[n_children]);
            ++n_children;//è diverso o no tra i vari processi??? Da guardare molto bene!!!
        }
    }else if (signo == SIGUSR2) {
        if (n_children == 0) {
            //vuol dire che non ho figli
            fprintf(file,"0"); //questo pid!=0 perché sono nel processo padre
            fprintf(stdout,"[server]0"); 
        }
        else {
            --n_children;
            kill(SIGKILL, children[n_children]);
            fprintf(file,"-%d",children[n_children]); //questo pid!=0 perché sono nel processo padre
            fprintf(stdout,"[server]-%d",children[n_children]); 
        }
        //io dovrei creare un array con tutti i children e killarli uno ad uno, il problema è che non so il limite massimo di children, dovrei fare un array dinamico che non posso fare...
    }else if (signo == SIGINT) {
        fprintf(file,"%d",n_children);
        exit(0); //bisogna uscire
    }
}

int main (int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Inserire solo 2 argomenti <server> o <client> e path file");
        exit(1);
    }
    char mode [MAX_SIZE];
    strcpy(mode,argv[1]);
    file = fopen(argv[2],"w+"); //lo apro in lettura e scrittura
    fatherPid = getpid();
    char buf [MAX_SIZE];
    int outcome = 0; //i think that it is useful to keep a varaiable for the different outcomes
    int internalCount = 0;

    if (file == NULL) {//it means that the file doesn't exist
        fprintf(stderr, "File doesn't exist, write a valid path");
        exit(3);
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
        children[i] = -1;
    }
    errno = 0;
    if(strcmp(mode,"server") == 0) {
        //do something
        // int fputs(const char *restrict s, FILE *restrict stream);
        fprintf(file,"%d\n",fatherPid);
        struct sigaction sa;
        sa.sa_flags = 0;
        sa.sa_handler = clientHandler;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2 , &sa, NULL);
        sigaction(SIGINT, &sa, NULL);

        while(1);

    }
    else if (strcmp(mode,"client") == 0) {
        //char *fgets(char *restrict s, int n, FILE *restrict stream);
        
        do {
            fgets(buf,sizeof(buf),file);
        }while (errno != 0); //se è diverso vuol dire che ho avuto un errore nella lettura
        printf("[client] server: %s",buf);

        while (1) {
            int c = getchar();

            if (c == '+') {
                kill(SIGUSR1,fatherPid);
                if (internalCount < 10)
                    ++internalCount;
                printf("[client] %d",internalCount);
            }
            else if (c == '-') {
                kill(SIGUSR2,fatherPid);
                if (internalCount > 0)
                    --internalCount;
                printf("[client] %d",internalCount);
            }
            else if (c == '\n') {
                for (int i = 0; i < internalCount; i++) {
                    kill(SIGUSR2,fatherPid);
                    printf("[client] %d",--internalCount);
                    sleep(1);
                }
                kill(SIGINT,getpid()); //e poi un segnale SIGINT e quindi terminare
            }
            
        }

        //Il processo termina con SIGINT (ad es. con “CTRL+C” da terminale) --> è il comportamente di default
    }
    else {
        fprintf(stderr, "Inserire come secondo parametro o <server> o <client>");
        exit(2);
    }



    return 0;
}