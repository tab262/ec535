import SimpleOpenNI.*;
import processing.serial.*;
Serial myPort;
SimpleOpenNI kinect;

int closestValue;
int closestX;
int closestY;
int DELAY = 0;
final int WIDTH = 640;
final int HEIGHT = 480;
void setup()
{
  size(WIDTH, HEIGHT);
  kinect = new SimpleOpenNI(this);
  kinect.enableDepth();
  
  String portName = Serial.list()[5];
  println(portName);
  myPort = new Serial(this,portName,115200);
}

void draw()
{
  closestValue = 8000;
  kinect.update();
  int[] DV = kinect.depthMap();
 
  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){
      //int i = x + y * 640;
      //int CDV = DV[x + y * 640];
      if(DV[x + y * 640] > 0 && DV[x + y * 640] < closestValue){
        closestValue = DV[x + y * 640];
        closestX = x;
        closestY = y;
      }
    }
  }
 
  image(kinect.depthImage(),0,0);
  fill(255,0,0);
  ellipse(closestX,closestY,15,15);
  //println(closestX + ", " + closestY + ", " + closestValue);
  
  //float val = (1.0*closestY)/HEIGHT * 255;
  //int val2 = (int)val;
  //println(val2 + "--" + closestY);
  if(DELAY % 10 == 0){
  if(closestY < (HEIGHT/2)){
    myPort.write('1');
  }else{
    myPort.write('0');
  }
  }
  DELAY = DELAY + 1;
}
