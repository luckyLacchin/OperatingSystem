#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 50

//Invert the string. Return the pointer to the new string.
char * stringrev(char * str){
    char tmp[MAX_LEN+1]; //Declare temporary string
    short len = strlen(str); //Save string length
    int ind=len;
    for(int i=0; i<len;i++){ //Cycle through string chars
        tmp[i]=str[--ind]; // Save ind-th string element as the i-th
    }
    tmp[len]=0; //Save termination char
    str = tmp;
    return str;
}

//Find chr in str and return its position if found. Return -1 otherwise
int stringpos(char * str,const char chr){
    short len = strlen(str); //Save string length
    for(int i = 0; i<len; i++){ //Cycle through string chars
        if(str[i]==chr) //Compare chars
            return i;
    }
    return -1;
}
int main(int argc, char ** argv){
    if (argc < 3){
        printf("Error. Correct syntax: %s <string> <char>\n",argv[0]);
        exit(2);
    }
    if (strlen(argv[1])>MAX_LEN){
        printf("String is too long. Max length: %d\n",MAX_LEN);
        exit(3);
    }
    if (strlen(argv[2])>2){
        printf("Char is not valid");
        exit(4);
    }
    printf("Char found in pos %d\nInverted: %s\n",stringpos(argv[1],argv[2][0]),stringrev(argv[1]));
    return 0;
}