// Avendo come argomenti dei “binari”, si eseguono con exec ciascuno in un sottoprocesso (generando un figlio)

#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
  for(int i = 1; i<argc;i++){
    int isChild = !fork(); //Generate child
    if(isChild){ //Launch executable only inside children
      char * argList[] = {argv[i],NULL}; //Define arguments
      execv(argv[i], argList); //Launch binary
    }
  }
  while(wait(NULL)>0); //Wait for all children
}