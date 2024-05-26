#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#define MAX_SIZE 255

int newCommand = 0;

struct mymsg
{
    long mtype;    /* Message type. */
    char mtext[1]; /* Message text. */
} queueMsg;

struct killStruct
{
    int sign;
    pid_t pid;
} killPayload;

struct queueStruct
{
    queueMsg msg;
    char queuePath[MAX_SIZE];
} queuePayload;

struct fifoStruct
{
    char fifoPath[MAX_SIZE];
    char word[MAX_SIZE];

} fifoPayload;

// azz i need arguments
void *killThread(void *param)
{
    killPayload *payload = (killPayload *)param;
    while (1)
    {
        if(newCommand == 1) {
            kill(payload.pid, payload.sign);
            newCommand = 0;
        }
    }
}

void *queueThread(void *param)
{
    queuePayload *payload = (queuePayload *)param;

    key_t key = ftok(payload.queuePath); // maybe i should check if the path exists?! Nothing is said

    int queue = msgget(key, 0777 | IPC_CREAT); // i think that we should only put this flag!

    while (1)
    {
        if(newCommand == 1) {
            msgsnd(queue, &payload.queuePath, sizeof(msg), 0);
            newCommand = 0;
        }
    }
}

void *fifoThread(void *param)
{
    fifoPayload *payload = (fifoPayload *)param;
    while (1)
    {
        if(newCommand == 1) {
            mkfifo(payload.fifoPath, 00600);
            int fd = open(payload.fifoPath, O_RDWR);
            write(fd, fifoLoad.word, strlen(fifoLoad.word));
            newCommand = 0;
        }
    }
}

void myHandler(int signo)
{
    newCommand = 1;
}


int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Inserire il <path>");
    }
    char *path;
    strcpy(path, argv[1]);
    FILE *file;
    file = fopen(path, "r");
    char buffer[3][MAX_SIZE];

    queueMsg msg;

    killPayload killLoad = {0, 0};
    queuePayload queueLoad = {NULL, NULL};
    strcpy(queueLoad.queuePath, path);
    fifoPayload fifoLoad = {NULL, NULL};

    struct sigaction sg;
    sg.sa_handler(myHandler);
    sg.sa_flags = 0; //Initialise flags
    sigemptyset(&sg.sa_mask); //Define an empty mask
    sigaction(SIGUSR1,&sa,NULL);

    //Create a new signal mask for the threads, so that they will block SIGUSR1
    sigset_t setBlocked;
    sigemptyset(&setBlocked);
    sigaddset(&setBlocked,SIGUSR1);
    
    //Create the attributes for the threads
    pthread_attr_t attrk;
    //Initialise the attributes and set the signal mask
    pthread_attr_init(&attrk);
    pthread_attr_setsigmask_np(&attrk,&setBlocked);

    pthread_t t_id1, t_id2, t_id3;
    pthread_create(&t_id1, &attrk, killThread, (void *)&killLoad);
    pthread_create(&t_id2, &attrk, queueThread, (void *)&queueLoad);
    pthread_create(&t_id3, &attrk, fifoThread, (void *)&fifoLoad);

    while ((!feof(file)) && (newCommand == 1))
    {
        if(newCommand == 1) {
            fscanf(file, "%s %s %s", buffer[0], buffer[1], buffer[2]);

            if (strcmp(buffer[0], "kill"))
            {

                killLoad.sign = atoi(buffer[1]);
                killLoad.pid = atoi(buffer[2]);
            }
            else if (strcmp(buffer[0], "queue"))
            {
                strcpy(msg.mtext, buffer[2])
                    msg.mtype = buffer[1];
                queueLoad.msg = msg; // it should work properly
            }
            else if (strcmp(buffer[0], "fifo"))
            {
                strcpy(fifoLoad.fifoPath, buffer[1])
                    strcpy(fifoLoad.word, buffer[2])
            }
        }
        else {
            //we don't do anything
        }
    }

    return 0;
}

// adesso devo modificare l'applicazione per usare threads!
// That's why i have to create a struct per threads, for communication! Really interesting!
// let's do the last point! As i thought, i should have put a flag...and remember also to close the files!