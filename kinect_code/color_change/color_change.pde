import SimpleOpenNI.*;

SimpleOpenNI kinect;
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
  
  
  kinect.enableDepth();
  kinect.enableRGB();
}

void draw()
{
   kinect.update();
   image(kinect.depthImage(),0,0);
   image(kinect.rgbImage(),640,0);
}
