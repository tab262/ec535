import SimpleOpenNI.*;
SimpleOpenNI kinect;
// Frame
PImage currentFrame;
color trackColor;
 
void setup()
{
 size(640, 480);
 kinect = new SimpleOpenNI(this);
 kinect.enableRGB();
 
 trackColor = color (255,255,255);
 smooth ();
 
 currentFrame = createImage (640,480, RGB);
 
}
 
void draw()
{
 kinect.update();
 
 currentFrame = kinect.rgbImage ();
 image(currentFrame,0,0);
 
 currentFrame.loadPixels();
 
 // Before we begin searching, the "world record" for closest color is set to a high number that is easy for the first pixel to beat.
 float worldRecord = 500;
 
 // XY coordinate of closest color
 int closestX = 0;
 int closestY = 0;
 
 // Begin loop to walk through every pixel
 for (int x = 0; x < currentFrame.width; x ++ ) {
   for (int y = 0; y < currentFrame.height; y ++ ) {
     int loc = x + y*currentFrame.width;
     // What is current color
     color currentColor = currentFrame.pixels[loc];
     float r1 = red(currentColor);
     float g1 = green(currentColor);
     float b1 = blue(currentColor);
     float r2 = red(trackColor);
     float g2 = green(trackColor);
     float b2 = blue(trackColor);
 
      // Using euclidean distance to compare colors
     float d = dist(r1,g1,b1,r2,g2,b2); // We are using the dist( ) function to compare the current color with the color we are tracking.
 
      // If current color is more similar to tracked color than
      // closest color, save current location and current difference
     if (d < worldRecord) {
       worldRecord = d;
       closestX = x;
       closestY = y;
     }
   }
 }
 
// We only consider the color found if its color distance is less than 10.
 // This threshold of 10 is arbitrary and you can adjust this number depending on how accurate you require the tracking to be.
 if (worldRecord < 5) {
  // Draw a circle at the tracked pixel
  fill(trackColor);
  strokeWeight(4.0);
  stroke(0);
  ellipse(closestX,closestY,16,16);
  println(closestX + ", " + closestY);
 }
}

void mousePressed(){
 color c = get(mouseX, mouseY);
 println("r: " + red(c) + " g: " + green(c) + " b: " + blue(c));
 
// Save color where the mouse is clicked in trackColor variable
 int loc = mouseX + mouseY*(currentFrame.width);
 println (loc);
 
 trackColor = currentFrame.pixels[loc];
}

