//Avendo come argomenti dei “binari”, si eseguono con exec ciascuno in un sottoprocesso (generando un figlio),
// in più salvando i flussi di stdout e stderr in un unico file.

#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
int main(int argc, char ** argv){
  //Open/create output file
  int outfile = open("/tmp/out.txt",O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  dup2(outfile,1); //Redirect stdout to outputfile
  dup2(outfile,2); //Redirect stderr to outputfile
  for(int i = 1; i<argc;i++){
    int isChild = !fork(); //Generate child
    if(isChild){ //Launch executable only inside children
      char * argList[] = {argv[i],NULL}; //Define arguments
      execv(argv[i], argList); //Launch binary
    }
  }
  while(wait(NULL)>0); //Wait for all children
  close(outfile);
}