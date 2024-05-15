/**
 * The tree is managed through N master processes, where N is the maximum depth.
 * Each master process is a group leader and each children created at depth M will
 * enter the group of master process M. This allows the main process to issue signal
 * only to a certain depth. 
 * NB: the kill command will leave hanging branches (gap in the tree)
 * */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_ARG 50
#define MAX_DEPTH 5
#define MAX_BROADTH 10
#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define DF "\033[0m"

int createFlag=0;
int isChild = 0;
pid_t myParent = 0;

int depth = 0, broadth = 0;

int groups[MAX_DEPTH]; //Master processes

// Error printing function
void perr(char * str){
    fprintf(stderr,"%s%s%s",RED,str,DF);
}

//Tabulation printing function
void printTab(){
    for(int i = 1; i <= depth ; i++){
        printf("\t");
    }
}

//Child creation function
int createChild(short level){
    // Issue a SIGUSR1 to instruct the children at level to create a new child
    
    return kill(-groups[level-1],SIGUSR1);
}

// Handler to initiate the creation of the child
void signalHandlerCreate(int signo){
    // Interrupt while(1) loop by setting the flag for the creation of the child. See end of main
    createFlag = 1;
}

// Signal all the children to print to stdout their info
void signalPrintHandler(int signo){
    printTab();
    printf("%s[ID %d - Parent: %d - Group %d] depth %d%s\n",GREEN,getpid(),myParent,getpgid(0),depth,DF);
}

//Signal handler to terminate the process
void signalTermHandlerChild(int signo){
    printf("[Child terminating]\n");
    exit(0); //This will leave a hanging branch
}
//Signal handler for the master process
void signalIntHandlerMasterParent(int signo){
    while(waitpid(-getpid(),NULL,0)>0);
    printf("[Terminating master parent] \n");
    exit(0);
}

//Quit function
void quit(int signo){
    for(int i = 1; i<MAX_DEPTH; i++){
        kill(-groups[i],SIGTERM); //Terminate children
    }
    while(wait(NULL)>0);
    exit(0);
}

//Kill children at a certain depth
int killChildren(int level){
    //Issue SIGTERM to all children at level depth
    printf("Killing children at level %d\n",level);
    return kill(-groups[level],SIGTERM);
}

//PRint entire tree
void printTree(){
    for(int i = 0; i<MAX_DEPTH; i++){
        //Use SIGUSR2 to signal print
        kill(-groups[i],SIGUSR2);
    }
}

int main(){
    char buffer[MAX_ARG]; //Buffer for input parameters
    int level = 0; //Default level to create child

    printf("Max level is %d\n",MAX_DEPTH-1);
    
    //Initiate the master parent: each child at depth n will belong to the group of Master Parent groups[n].
    groups[0] = getpid(); //Set current process as Master parent level 0
    
    //Create all of the master parents
    for(int i = 1; i < MAX_DEPTH; i++){
        groups[i] = fork();
        if(groups[i]==0) break; 
    }
    
    // Each Master parent becomes group leader
    setpgid(0,getpid());
    
    

    //Block all master parents
    if(groups[0] != getpid()){
        signal(SIGTERM,signalIntHandlerMasterParent); //Ignore TERM since its used to terminate children
        while(1) pause();
    }

    printf("Master processes = ");
    for(int i = 0; i<MAX_DEPTH; i++){
        printf("[%d] ",groups[i]);
    } fflush(stdout);

    //Assign handlers for the various events.
    signal(SIGUSR2,signalPrintHandler);
    signal(SIGUSR1,signalHandlerCreate);
    signal(SIGINT,quit);
    //Enter infinite loops for parsing commands
    while(1){
        sleep(1); //Delay to avoid graphic glitches
        printf("\nNext command: "); fflush(stdout);

        //Fetch command from STDIN
        int r = read(STDIN_FILENO, buffer, MAX_ARG-1);
        buffer[r] = 0; //Terminate string
        char command[MAX_ARG];
        int level = -1;
        sscanf(buffer,"%s %d",command,&level);
        
        //Parse command
        if(strcmp(command,"create") == 0){ //Create child
            if(level == -1 ){
                perr("Invalid parameter\n");
                continue;
            }
            if(level == 1){ //Create immediate child
                if(broadth++ < MAX_BROADTH){
                    printf("Creating child at level %d\n",level);
                    isChild = !fork();
                }else{  
                    perr("Too many children\n");
                }
            }else if(level > 1 && level < MAX_DEPTH){ //Issue creation of grandchild
                printf("Creating grandchild at level %d\n",level);
                //We have to delegate the master process of that level
                createChild(level);
            }
        }else if(strcmp(command,"kill") == 0){ //Kill child/grandchild
            if(level == -1 ){
                perr("Invalid parameter\n");
                continue;
            }
            //Kill children by issuing the termination signal
            if(level > -1 && level < MAX_DEPTH)
                killChildren(level);
            else
                perr("Invalid parameter\n");
        }else if(strcmp(command,"print") == 0){
            printf("Printing Tree:\n");
            printTree();
        }else if(strcmp(command,"quit") == 0){
            printf("Terminating...\n");
            quit(0);
        }else{
            fprintf(stderr,"Invalid parameter\n");
        }


        if(isChild){ //If it's the first child
            signal(SIGTERM,signalTermHandlerChild); //Set termination handler
            myParent = getppid(); //Keep track of who is the parent
            depth++; //Increase depth at which the child is 
            broadth = 0; // Set broadth of own branch 
            setpgid(0,groups[depth]); //Join group of master parent
            printf("I'm new child at level %d with id = %d\n",depth,getpid());
            break;
        }
    }






    //Child
    while(1){
        if(createFlag){ //This flag is set by the signal handler!
            if(broadth++ < MAX_BROADTH){ //Create new child if allowed
                if (!fork()){ //Child that is created
                    depth++; //Increase own depth
                    broadth=0; //Initialise broadth
                    myParent = getppid(); //Keep track of parent
                    setpgid(0,groups[depth]); // Join master parent group
                    printf("I'm new child at level %d with group = %d\n",depth,getpgid(0));
                }
            }else{
                perr("Too many children\n");
            }
            //Reset create flag
            createFlag = 0;
        }
        pause();
    }
    
}