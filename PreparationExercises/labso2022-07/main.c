#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define MAX_SIZE 10

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr,"Inserire due parametri argomento");
        exit(0);
    }
    if (mkfifo(argv[1], 0600) == -1)
    {
        fprintf(stderr,"Error in the creation of the pipe");
        exit(2);
    }

    char *path;
    strcpy(path, argv[1]);
    int n = atoi(argv[2]);
    if (!(n >= 0 && n <= 10))
    {
        fprintf(stderr,"Inserire un numero tra 0 e 10");
        exit(3);
    }

    FILE *ptr;
    ptr = fopen(path, "r");
    // FIFOGET//
    char newChar;
    int index = 0;
    while ((!foef(ptr)) && (index < n))
    {
        newChar = fgetc(ptr);
        fprintf(stdout, "%c\n", newChar);
        ++index;
    }

    if (index < n)
    {
        exit(1);
    }
    else
    {
        exit(0);
    }

    // Fiforev <path> <n>
    char buffer[MAX_SIZE];
    fgets(buffer, n + 1, ptr);

    for (int i = strlen(buffer) - 1; i >= 0; i--)
    {
        fprintf(stdout, "%c\n", buffer[i]);
    }
    if (strlen(buffer) < n)
    {
        exit(1);
    }
    else
    {
        exit(0);
    }

    // fifoskp
    char newChar;
    int index = 0;
    while ((!foef(ptr)) && (index < n - 1))
    {
        newChar = fgetc(ptr);
        fprintf(stdout, "%c\n", newChar);
        ++index;
    }

    if (index < n - 1)
    {
        exit(1);
    }
    else
    {
        exit(0);
    }

    // fifoply <path> <n>
    // let's do the last one :)

    char cifre[MAX_SIZE];
    char letters [MAX_SIZE];
    int n_cifre = 0;
    int i_cifre = 0;
    int i_letters = 0;
    while (!feof(ptr) && (n_cifre < n))
    {
        newChar = fgetc(ptr);
        if (newChar >= "0" && newChar <= "9")
        {
            ++n_cifre;
            cifre[i_cifre] = newChar;
            ++i_cifre;
        }
        else {
            letters[i_letters] = newChar;
            ++i_letters;
        }

    }

    for (int j = strlen(letters) - 1; j >= 0; j--) {
        printf("%c\n",letters[j]);
    }

    int sum = 0;
    int tempInt;
    for (int i = 0; i <= strlen(cifre); i++) {
        tempInt = atoi(cifre[i]);
        sum+=tempInt;
        printf("%c+",cifre[i]);
    }
    printf("=%d",sum);

    return 0;
}