#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0
#define PARAMS 4
typedef char bool;
const char * valid[] = {"-h","-m","-n","--help"};

//Return TRUE is parameter is valid, FALSE otherwise
bool isValid(char * arg){
    for (int i = 0; i<PARAMS;i++){ // Cycle through all of the valid parameters
        //Compare the i-th element of the valid list with the parameter
        if(strcmp(arg,valid[i]) == 0){
            return TRUE;
        }
    }
    return FALSE;
}
int main(int argc, char ** argv){
    char * paramList[argc-1]; //Define list of stored parameters
    int ind = 0;
    for(int i = 1; i<argc; i++){ //Cycle through parameters
        if(isValid(argv[i])){ //Check if parameter is valid
            paramList[ind++]=argv[i]; //Save valid parameter
        }else{
            printf("'%s' is a invalid parameter\n",argv[i]);
            exit(2);
        }
    }
    //Print list of recognised parameters
    printf("Recognised parameters:\n");
    for (int i = 0; i< ind; i++){
        printf("%s\n",paramList[i]);
    }
    return 0;
}