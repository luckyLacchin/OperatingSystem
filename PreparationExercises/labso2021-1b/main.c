#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h> // For IPC structures and constants
#include <sys/msg.h> // For message queue structures and constant
#include <signal.h>

#define PATH_MAXLEN 60
#define CHILDREN_MAX 10
#define PDILEN 8

typedef struct Msg_buffer
{
    long mtype;
    char mtext[100];
} msg_buffer;

void childHandler(int sig);
void childFun();

int fdGlobal;
int queueGlobal;

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr, "Il numero di argomenti argv è incoretti. Corretto format: ./app /tmp int \n");
        // exit(1);
    }
    struct stat st;
    if (stat(argv[1], &st))
    {
        fprintf(stderr, "Path does not exist!\n");
        // exit(2);
    }
    char subdir[PATH_MAXLEN] = "/info";
    char path[PATH_MAXLEN];
    char file[PATH_MAXLEN] = "/key.txt";
    char pathInfo[PATH_MAXLEN];
    char tmpPath[PATH_MAXLEN];

    strcpy(path, argv[1]);
    strcat(path, subdir);
    printf("path = %s \n", path);
    if (mkdir(path, 0755) == -1)
    {
        fprintf(stderr, "Cannot create info directory\n");
        // exit(3);
    }
    strcpy(pathInfo, path);
    int index = strlen(pathInfo);
    strcat(path, file);
    int fd = open(path, O_RDWR | O_CREAT);
    printf("file = %s \n", path);
    chmod(path, 0755);
    // before i have to create the text
    char text[PATH_MAXLEN];
    int r = sprintf(text, "%d", (int)getpid());
    text[r] = '\n';
    text[r + 1] = '\0';
    write(fd, text, strlen(text));

    int n = atoi(argv[2]);
    printf("%d\n", n);

    key_t key = ftok(path, 32);
    int queue = msgget(key, IPC_CREAT);
    msgctl(queue, IPC_RMID, NULL); // so i'm sure it wasn't previously created!
    queue = msgget(key, IPC_CREAT);
    queueGlobal = queue;
    msg_buffer msg;
    strcpy(msg.mtext, text);
    msg.mtype = 1;
    printf("msg.mtext = %s", msg.mtext);
    msgsnd(queue, &msg, sizeof(msg.mtext), 0);

    for (int i = 0; i < n; i++)
    {
        int isChild = !fork();
        if (isChild)
        {
            sprintf(file, "%d", (int)getpid());
            strcpy(tmpPath, pathInfo);
            tmpPath[index] = '/';
            strcat(tmpPath, file);
            printf("%d ", (int)getpid());
            fflush(stdout);
            fd = open(tmpPath, O_RDWR | O_CREAT);
            fdGlobal = fd;
            chmod(tmpPath, 0755);

            childFun();
        }
    }
    return 0;
}

void childFun()
{
    struct sigaction sa;
    sa.sa_sigaction = childHandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    while (1)
        ; // perché devo lasciarli attivi!!!
}

void childHandler(int sig)
{
    if (sig == SIGUSR1)
    {
        write(fdGlobal, "SIGUSR1\n", strlen("SIGUSR1\n"));
    } // do something
    else if (sig == SIGUSR2)
    {
        // do something else!
        char message[PATH_MAXLEN];
        sprintf(message, "%d", (int)getpid());
        msg_buffer msg;
        strcpy(msg.mtext, message);
        msg.mtype = 1;
        printf("msg.mtext = %s", msg.mtext);
        msgsnd(queueGlobal, &msg, sizeof(msg.mtext), 0);
    }
}