/*
Dati due eseguibili come argomenti del tipo ls e wc si eseguono in due processi
distinti: il primo deve generare uno stdout redirezionato su un file temporaneo,
mentre il secondo deve essere lanciato solo quando il primo ha finito leggendo lo
stesso file come stdin.
Ad esempio ./main ls wc deve avere lo stesso effetto di ls | wc.
*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char ** argv) {
  
  if(argc < 3){
    fprintf(stderr,"Wrong number of arguments\n");
    return 2;
  }
  char * argList[2];
  int tmpFile = open("/tmp/temp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); //Create temporary file
  int f=fork();
  if (f==0) { //Child
    //Create arguments
    argList[0]=argv[1];
    argList[1]=NULL; 
    dup2(tmpFile, 1); //Redirect stdout to tmpFile
    execvp(argList[0],argList);
  } else { //Father
    wait(NULL); //Wait for the only child to have finished
    argList[0]=argv[2];
    argList[1] = NULL;
    
    dup2(tmpFile, 0); //Redirect stdinput to tmpFile
    lseek(tmpFile, 0, SEEK_SET); //Reset I/O pnt

    execvp(argList[0],argList);
  };
}