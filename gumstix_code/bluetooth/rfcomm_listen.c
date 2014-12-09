#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define DEBUG 0


int main(int argc, char **argv)
{
  printf("Waiting for connection to open...\n");
  char buffer[256];
  buffer[0] = 'x';
  FILE * pFile;
  //int pFile; 
  int c = 65;
  int Y_NEUTRAL = 230;
  int X_NEUTRAL = 320;
  int x=0,y=0;
  int inc;
  int yaw;

  // Access kernel for writin
  char *kernel_address = "/dev/arduino_comms";
  FILE *kernelFile; 
  kernelFile = fopen(kernel_address,"r+");

  if(kernelFile == NULL){
    printf("Failed to open kernel file\n");
  }else{
    printf("Kernel opened\n");
  }

  do{
    //printf("Opening rfcomms...\n");
    pFile = fopen("/dev/rfcomm0", "r");
    sleep(1);
  }while(pFile == NULL);

  printf("RFCOMMS OPENED\n");
  int i = 0;
  //bytes_read = getline (&my_string, &nbytes, stdin);
  //c = getline(buffer,256,pFile);
  //fgets(buffer,sizeof(buffer),pFile);
  fscanf(pFile,"%d,%d",&x,&y);
  printf("c: %d\n",c);
  //printf("BUFFER: %s\n", buffer);
  //y = atoi(buffer);
  
  printf("init x,y: %d,%d\n",x,y);


  
  while(i++ < 1000){
    //fgets(buffer,sizeof(buffer),pFile);
    int result = fscanf(pFile,"%d,%d",&x,&y);
    if(result > 0){
          
      if(DEBUG) printf("x,y: %d,%d\n", x,y);
      if(y < (Y_NEUTRAL-20)){
	inc = 0;
	if(DEBUG)printf("Throttle: Decrease--");
      }else if (y > (Y_NEUTRAL + 20)){
	inc = 1;
	if(DEBUG)printf("Throttle: Increase--");
      }else{
	inc = -1;
	if(DEBUG)printf("Throttle: No change--");
      }

      if(x < (X_NEUTRAL-20)){
	yaw = 0;
	if(DEBUG)printf("YAW: Decrease\n");
      }else if (x > (X_NEUTRAL + 20)){
	yaw = 1;
	if(DEBUG)printf("YAW: Increase\n");
      }else{
	yaw = 2;
	if(DEBUG)printf("YAW: No change\n");
      }

      sprintf(buffer,"a,%d,%d,%d\n",yaw,0,inc);
    
      //fputs(buffer,kernelFile);

      fwrite(buffer,1,sizeof(buffer),kernelFile);
      
      memset(buffer,0,256);
    }
  }
  
  printf("Closing rfcomms...\n");
  
  fclose(pFile);
  
  return 0;
}

