import SimpleOpenNI.*;
import processing.serial.*;
Serial myPort;
SimpleOpenNI kinect;

int closestValue;
int closestX = 0;
int closestY = 0;
int DELAY = 0;
final int WIDTH = 640;
final int HEIGHT = 480;
int[] START_FIELD;
int[] START_IMAGE;
int[] STATIC_IMAGE;
void setup()
{
  size(WIDTH, HEIGHT);
  kinect = new SimpleOpenNI(this);
  kinect.enableDepth();
  kinect.update();
  START_FIELD = kinect.depthMap();
  STATIC_IMAGE = new int[WIDTH*HEIGHT];
  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){
      STATIC_IMAGE[x+WIDTH*y] = START_FIELD[x+WIDTH*y];
    }
  }
  
  String portName = Serial.list()[5];
  println(portName);
  myPort = new Serial(this,portName,115200);
}

void draw()
{
  closestValue = 8000;
  kinect.update();
  

  
  int[] DV = kinect.depthMap();
  float max_diff = 0;
  int index;
 //println((START_FIELD[0]  + " " + DV[0]));
  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){
      index = x+y*640;
      //println(START_FIELD[index]);
      float scaled_diff = abs(STATIC_IMAGE[index] - DV[index]) / (STATIC_IMAGE[index]+1);
      if((scaled_diff > max_diff)){
        max_diff = scaled_diff;
        closestX = x;
        closestY = y;
      }
    }
  }
 
  //println(closestX + " " + closestY); 
  image(kinect.depthImage(),0,0);
  fill(255,0,0);
  
  ellipse(closestX,closestY,15,15);

  
}
