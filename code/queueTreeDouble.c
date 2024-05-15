#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>

#define MAX_ARG 10
#define MAX_DEPTH 5
#define MAX_BROADTH 10
#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define DF "\033[0m"

int createFlag=0;
int isChild = 0;
pid_t myParent = 0;

int depth = 0, broadth = 0;
int treeStruct1[MAX_DEPTH];
int treeStruct2[MAX_DEPTH];
pid_t children[MAX_BROADTH]; //Store pid of children

typedef struct msg{
    long mtype;
    char mtext[2];
} Msg_packet;


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
int createChild(int queue,int level,int * treeStruct){
    //Create creation message
    Msg_packet msgCreate;
    msgCreate.mtype=level-1;
    strncpy(msgCreate.mtext,"c",2);

    //Send enough messages
    for(int j = 0; j < treeStruct[level-1];j++){
        msgsnd(queue,&msgCreate,sizeof(msgCreate.mtext),0);
        //Update tree struct
        treeStruct[level]++;
    }
    return 0;
}

//Print tree
int printTree(int queue,int * treeStruct){
    printTab();
    printf("%s[ID %d - Parent: %d] depth %d%s\n",GREEN,getpid(),myParent,depth,DF);
    
    //Define print message
    Msg_packet msgPrint;
    msgPrint.mtype=0;
    strncpy(msgPrint.mtext,"p",2);

    //Send enough messages for every children
    for(int i = 1; i<=MAX_DEPTH;i++){
        msgPrint.mtype=i;
        for(int j = 0; j < treeStruct[i];j++){
            msgsnd(queue,&msgPrint,sizeof(msgPrint.mtext),0);
            //printf("Msg sent | ");
        }
    }
    //printf("%sSent Print messages to all on queue %d %s\n",GREEN,queue,DF);
    return 0;
}

//Kill children
int killChildren(int queue, int level, int * treeStruct){
    Msg_packet msgKill;
    msgKill.mtype=level;
    strncpy(msgKill.mtext,"k",2);
    
    for(int j = 0; j < treeStruct[level];j++){
        msgsnd(queue,&msgKill,sizeof(msgKill.mtext),0);
    }
    
    return 0;
}


//Quit function with additional parameters
void quit(int queue, int * treeStruct,int queue2, int * treeStruct2 ){ /** ADDITIONAL PARAMETERS **/
    killChildren(queue,1,treeStruct);
    killChildren(queue2,1,treeStruct2); /** ADDITIONAL killChildren CALL **/
    printf("%sSent quit message to all on %d. Waiting for their termination%s\n",GREEN,queue,DF);
    while(wait(NULL)>0);
    printf("Terminating program\n");
    exit(0);
}

int main(){
    char buffer[MAX_ARG]; //Buffer for input parameters
    int level = 0;

    /* Create additional queue with different index */
    creat("/tmp/tree", 0777);
    key_t treeKey1 = ftok("/tmp/tree",1);
    key_t treeKey2 = ftok("/tmp/tree",2);
    
    int treeeQueue1 = msgget(treeKey1,0777|IPC_CREAT);
    msgctl(treeeQueue1,IPC_RMID,NULL);
    treeeQueue1 = msgget(treeKey1,0777|IPC_CREAT);
    
    int treeeQueue2 = msgget(treeKey2,0777|IPC_CREAT);
    msgctl(treeeQueue2,IPC_RMID,NULL);
    treeeQueue2 = msgget(treeKey2,0777|IPC_CREAT);

    int queue;
    //Enter infinite loops for parsing commands
    while(1){
        sleep(1);
        printf("\nNext command: ");fflush(stdout);

        //Fetch command from STDIN
        int r = read(STDIN_FILENO, buffer, MAX_ARG-1);
        buffer[r] = 0; //Terminate string
        
        int ind = 0;
        queue = treeeQueue1;
        int * treeStruct = treeStruct1;

        // Handle the tree selection 
        ind = 1;
        if(buffer[0] == '1'){
            queue=treeeQueue1;
            treeStruct = treeStruct1;
            printf("Queue1 %d selected\n",queue);
        }else if(buffer[0] == '2'){
            queue=treeeQueue2;
            treeStruct = treeStruct2;
            printf("Queue2 %d selected\n",queue);
        }else if(buffer[0] == 'q'){
            quit(treeeQueue1,treeStruct1,treeeQueue2,treeStruct2);
        }else{
            perr("Invalid parameter\n");
            continue;
        }
            
        
        
        //Parse command
        switch(buffer[ind]){
            case 'c': //Create child
                level = atoi(buffer+ind+1); //Get level of new child to be created
                if(level == 1){ //Create immediate child
                    if(broadth++ < MAX_BROADTH){
                        printf("Creating child at level %d\n",level);
                        treeStruct[level]++;
                        isChild = fork();
                    }else{  
                        perr("Too many children\n");
                    }
                }else if(level > 1 && level <= MAX_DEPTH){ //Issue creation of grandchild
                    printf("Creating grandchild at level %d\n",level);
                    createChild(queue,level,treeStruct);
                }else{
                    perr("Invalid parameter\n");
                }
                break;
            case 'k': //Kill child/grandchild
                level = atoi(buffer+ind+1); //Get level of child
                
                //Kill children by issuing the termination signal
                if(level > -1 && level <= MAX_DEPTH)
                    killChildren(queue,level,treeStruct);
                else
                    perr("Invalid parameter\n");
                break;
            case 'p':
                printf("Printing Tree:\n");
                printTree(queue,treeStruct);
                break;
            default:
                perr("Invalid parameter\n");
        }
        if(isChild == 0){ //If it's the first child
            myParent = getppid(); //Keep track of who is the parent
            depth++; //Increase depth at which the child is 
            broadth = 0; // Set broadth of own branch
            printf("I'm new child at level %d with id = %d Listening on queue %d\n",depth,getpid(),queue);
            break;
        }else{
            children[broadth-1]=isChild;
        }
    }
    //Child
    Msg_packet msgRcv;
    while(1){
        //Hang for next message
        msgrcv(queue,&msgRcv,sizeof(msgRcv.mtext),depth,0);
        if(strcmp(msgRcv.mtext,"c")==0){ //Create child
            if(broadth++ < MAX_BROADTH){ //Create new child if allowed
                children[broadth-1] = fork(); // Keep track of child
                if (children[broadth-1] == 0){ //Child that is created
                    depth++; //Increase own depth
                    broadth=0; //Initialise broadth
                    myParent = getppid(); //Keep track of parent
                    printf("I'm new child at level %d with id = %d Listening on queue %d\n",depth,getpid(),queue);
                }
            }else{
                perr("Too many children\n");
            }
        }else if(strcmp(msgRcv.mtext,"k")==0){ //Kill
            //Send termination enough termination message (for each child)
            Msg_packet msgKill;
            msgKill.mtype=depth+1;
            strncpy(msgKill.mtext,"k",2);
            for(int i = 0; i<broadth; i++){
                msgsnd(queue,&msgKill,sizeof(msgKill.mtext),0);
            }
            while(wait(NULL)>0);
            printf("[%d] Terminating\n",getpid()); 
            exit(0);
        }else if(strcmp(msgRcv.mtext,"p")==0){ //Print
            printTab();
            printf("%s[ID %d - Parent: %d] depth %d%s\n",GREEN,getpid(),myParent,depth,DF);
        }else{
            printf("[%d]Invalid message read '%s'\n",getpid(),msgRcv.mtext);
        }
        sleep(1); //Prevent process from reading additional message
    }
    
}