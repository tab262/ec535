import SimpleOpenNI.*;

SimpleOpenNI kinect;
final int WIDTH = 640;
final int HEIGHT = 480;
color[] STATIC_COLOR;
int[] START_IMAGE;
int[] STATIC_IMAGE;

void setup()
{
  size(WIDTH*2, HEIGHT);
  kinect = new SimpleOpenNI(this);
  kinect.enableDepth();
  kinect.update();

  kinect.enableDepth();
  kinect.enableRGB();
  kinect.update();
  int index;
  PImage rgbImage = kinect.rgbImage();
  color c = rgbImage.get(0,0);
  STATIC_COLOR = new color[WIDTH*HEIGHT];

  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){   
        index = x + y*WIDTH;
        STATIC_COLOR[index] = rgbImage.get(x,y);   
    }
  }
  println(STATIC_COLOR[0]);
  println(red(STATIC_COLOR[100]) + "|" + green(STATIC_COLOR[100]) + "|" + blue(STATIC_COLOR[100]));
}

void draw()
{
   kinect.update();
   image(kinect.depthImage(),0,0);
   image(kinect.rgbImage(),640,0);
   
   /*
   int average = 0;
   for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){   
        average += rgbImage.get(x,y);   
    }
   }
   
   println("AVERAGE CHANGE: " + average);
   */
}

void mousePressed(){
 color c = get(mouseX, mouseY);
 println(mouseX + ", " + mouseY + ": " + red(c) + "|" + green(c) + "|" + blue(c)); 
}
