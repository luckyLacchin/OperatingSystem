// let's start this 2nd agony
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_FILES 10
#define BYTE 8

int main(int argc, char *argv[])
{

    if (argc > 11)
    {
        fprintf(stderr, "Inserire al massimo 10 paths");
    }

    int n = argc;
    int children[MAX_FILES];
    int thisProcess = -1;
    int father = getpid();
    int fd; // file descriptor opening the file
    char bufByte[BYTE];
    int outcome = 0;
    for (int i = 1; i < argc; i++)
    {
        // devo creare un sottoprocesso per ciascuno dei file
        children[i] = fork(); // se ottengo 0 vuol dire che sono nel child

        if (children[i] == 0)
        { // vuol dire che sono nel child e mi salvo il suo index :)
            thisProcess = i;

            break;
        }
    }

    if (getpid() == father)
    { 
        while (wait(NULL) > 0); //così aspettiamo tutti i processi figli!!!
        printf("\n");
    }
    else
    {
        fd = open(argv[thisProcess], O_RDONLY);
        // ssize_t read(int fd, void buf[.count], size_t count);
        read(fd, bufByte, sizeof(bufByte));
        if ((bufByte >= "0" && bufByte <= "9") || (bufByte >= "a" && bufByte <= "z") || (bufByte >= "A" && bufByte <= "Z"))
        {
            outcome = printf("Ho letto il seguente byte: %s", bufByte);
        }
        else
        {
            outcome = printf("Ho letto il seguente byte: %s", "-");
        }
        if (outcome < 0)
        {
            fprintf(stderr, "Encounterer error: %s", "?");
        }
        close(fd);
    }

    return 0;
}