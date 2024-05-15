/* lengthof.c */

#include <stdio.h>

int main(int argc, char **argv) {
  int code=0; 
  int len=0;
  char *p;
  if (argc!=2) { //Check number of arguments
    fprintf(stderr,"Usage: %s <stringa>\n", argv[0]);
    code=2;
  } else {
    p=argv[1]; //Copy pointer to first argument
    while (*p != 0 ){ //Check if character is termination char
      p++; //Move to next char
      len++; //Increase length count
    };
    printf("%d\n", len);
  };
  return code;
}
