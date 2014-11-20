import SimpleOpenNI.*;

SimpleOpenNI kinect;

final int WIDTH = 640;

void setup()
{
  size(WIDTH,480);
  kinect = new SimpleOpenNI(this);
  
  kinect.enableDepth();
}

void draw()
{
  kinect.update();
  PImage depthImage = kinect.depthImage();
  image(depthImage,0,0);
}
    
    
void mousePressed()
{
  int[] depthValues = kinect.depthMap();
  int clickPosition = mouseX + (mouseY * WIDTH);

  int millimeters = depthValues[clickPosition];
  float inches = millimeters / 25.4;
  
  println(inches + "in -- " + millimeters + "mm");

}

