/*

Creare un programma che accetti come parametro un numero di figli che andranno creati. 
Il processo padre deve instaurare una comunicazione bidirezionale con pipe anonime con i figli e, 
successivamente, accetterà in input le seguenti opzioni:
- in 🡪 sollecita il figlio n (dove n non è l’id quanto l’n-esimo figlio creato) ad inviargli il suo pid
- rn 🡪 sollecita il figlio n (dove n non è l’id quanto l’n-esimo figlio creato) ad inviargli un numero random
- q 🡪 termina il processo e tutti i figli

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_PROC 20 //Number of process that can be created
#define RD 0
#define WR 1
#define MAX_ARG 3 //Number of chars for stdin commands

#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define DF "\033[0m"



// Error printing function
void perr(char * str){
    fprintf(stderr,"?%s%s%s",RED,str,DF);
}

//Termination function
void signalIntHandler(int signo){
    //Issue termination signal to all children
    kill(-getpid(),SIGTERM);

    //Wait children termination
    while(wait(NULL)>0);
    printf("Terminating");

    //Terminate
    exit(0);
}

int main(int argc, char ** argv){
    
    //Get process number from first parameter
    if(argc != 2){
        perr("Wrong number of parameters\n");
        return 1;
    }

    //Convert argument to integer
    int proc = atoi(argv[1]);
    if(proc > 0 && proc <= MAX_PROC){
        printf("Creating %d processes\n",proc);
    }else{
        perr("Wrong number of proc\n");
        return 2;
    }

    int current;
    int isChild, integerFromChild, r;
    
    char command[MAX_ARG+1]; //Buffer for input parameters
    

    //Children ids
    pid_t children[MAX_PROC];
    
    // Input pipes used by the children to talk with the parent
    int input[MAX_PROC][2];
    
    //Output pipes used by the parent to talk with the childrens
    int output[MAX_PROC][2];

    //Cycle over the number of procs
    for(current = 0;current<proc; current++){
        //Create pipes
        pipe(input[current]);
        pipe(output[current]);
        isChild = fork();
        if(isChild != 0){ // Ancestor
            children[current] = isChild;
            close(input[current][WR]); //Close writing end of input pipe
            close(output[current][RD]); //Close read end of output pipe
        }else{ //Child
            close(output[current][WR]); //Close writing end of output pipe (child's input)
            close(input[current][RD]);  //Close reading end of input pipe (child's output)
            break; //Break from the for loop if child. Child will have unique 'current' value
        }
    }

    if(isChild != 0){ //Ancestor
        signal(SIGINT,signalIntHandler); //Override handler to kill all children
        
        //Enter infinite loop
        while(1){
            printf("\nNext command: ");fflush(stdout); //Create prompt for the user
            r = read(STDIN_FILENO, command, MAX_ARG+1); //Read from stdin
            if(command[0] == 'q'){
                // Terminate all children and itself
                signalIntHandler(0);
            }
            if(command[0] != 'r' && command[0] != 'i'){
                perr("Wrong parameter. Allowed are 'r' and 'i' and 'q'\n");
                continue;
            }

            //Get target process
            int process = atoi(command+1); //Convert the 'n' in the command to an integer
            if(process < 0 || process > proc){
                perr("Wrong target\n");
                continue;
            }
            process--; //indexes start from 0, decrease by one
            
            //Instruct child to produce output
            write(output[process][WR],command,1); //Only send first char
            
            //Get message from child
            r = read(input[process][RD],&integerFromChild,sizeof(integerFromChild));
            printf("Child %d told me: '%d'\n",children[process],integerFromChild);
        }
        
    }else{ //Child
        srand(current);   // Initialization, should only be called once.
        while(1){ 
            

            //Read parent instructions
            r = read(output[current][RD],command,1);
            
            command[r] = 0;

            //Check what command the parent has issued
            if(strcmp(command,"r") == 0){
                printf("%sChild %d computing random....%s\n",GREEN,current+1,DF);
                integerFromChild = rand(); // Returns a pseudo-random integer between 0 and RAND_MAX.
                write(input[current][WR],&integerFromChild,sizeof(integerFromChild)); //Send random
            }else if(strcmp(command,"i") == 0){
                printf("%sChild %d sending own pid....%s\n",GREEN,current+1,DF);
                integerFromChild = getpid();
                write(input[current][WR],&integerFromChild,sizeof(integerFromChild)); //Send pid
            }else{
                perr("Command not recognised \n");
            }       
        }
        
    }

}