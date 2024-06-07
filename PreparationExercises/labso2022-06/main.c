//let's start... . . .. . . .. . . una vita in vacanza, una vecchia che ballaa...
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//./coda <name> <action> [<value>] <pid> questo è il template

#define MAX_LENGTH 32

typedef struct {
    long   mtype;       /* Message type. */
    char   mtext[1];    /* Message text. */
} msgBuf;

int main (int argc, char *argv[]) {

    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Inserire 3 o 4 parametri");
        exit(0);
    }
    char name [MAX_LENGTH];;
    errno = 0;
    char action [MAX_LENGTH];
    char value  [MAX_LENTH];
    pid_t pidInput;
    int queue;
    msgBuf msg;
    strcpy (action, argv[2]);
    strcpy (name, argv[1]);

    if (strcmp(action,"new") == 0) {
        key_t key = ftok(name,1);
        queue = msgget (key, 0777 | IPC_CREAT | IPC_EXCL); //it is very important to remember to put the permission bits!!!

        if (queue == -1) { //it means that was already previously created
            queue = msgget (key, 0777 | IPC_CREAT );
            fprintf (stdout, "Queue already create for %d", queue);
        }

    }
    else if (strcmp(action,"put") == 0) {
        if (argc != 5) {
            fprintf(stderr, "With put we need 4 paramters, because we need also the <value>");
            exit(1):
        }
        queue = msgget (key, 0777 | IPC_CREAT);
        strcpy(msg.mtext,argv[3]);
        //int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
        msgsnd(queue,&msg,sizeof(msg.mtext),0); //non so la category, credo che 0 vada bene..

    }else if (strcmp(action,"get") == 0) {
        //ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,int msgflg);
        queue = msgget (key, 0777 | IPC_CREAT );
        msgrcv(queue,&msg,sizeof(msg.mtext),NULL,0);
        fprintf(stdout, "Get stampa %s dopo il comando prec\n", msg.mtext);
    }else if (strcmp(action,"del") == 0) {
        queue = msgget (key, 0777 | IPC_CREAT | IPC_EXCL);
        if (queue != -1) { //vuol dire che esiste già!
            //int msgctl(int msqid, int cmd, struct msqid_ds *buf);
            msgctl(queue,IPC_RMID,NULL); //così dovrei cancellarlo
        }
    }else if (strcmp(action,"emp") == 0) {
        queue = msgget (key, 0777 | IPC_CREAT );
        while (msgrcv(queue,&msg,sizeof(msg.mtext),NULL,IPC_NOWAIT) != -1) {
            //vuol dire che posso leggere il messaggio
            fprintf(stdout, "%s\n", msg.mtext); //stampiamo i messaggi riga per riga
        }
    }

    return 0;
}

//direi di andare avanti domani!