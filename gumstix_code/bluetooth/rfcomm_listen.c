#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>




int main(int argc, char **argv)
{
  printf("Waiting for connection to open...\n");
  char buffer[256];
  buffer[0] = 'x';
  FILE * pFile;
  //int pFile; 
  int c = 65;

  do{
    //printf("Opening rfcomms...\n");
    pFile = fopen("/dev/rfcomm0", "r");
    sleep(1);
  }while(pFile == NULL);

  printf("RFCOMMS OPENED\n");
  int i = 0;
  //bytes_read = getline (&my_string, &nbytes, stdin);
  //c = getline(buffer,256,pFile);
  fgets(buffer,sizeof(buffer),pFile);
  printf("c: %d\n",c);
  printf("BUFFER: %s\n", buffer);
  
  while(i++ < 10){
    fgets(buffer,sizeof(buffer),pFile);
    printf("BUFFER: %s\n", buffer);
  }
  printf("Closing rfcomms...\n");
  
  fclose(pFile);
  
  return 0;
}
