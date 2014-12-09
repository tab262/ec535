import processing.serial.*;
Serial bluetoothPort;
void setup()
{
  println(Serial.list());
  println(Serial.list()[5]);
  String portName = Serial.list()[5];
  bluetoothPort = new Serial(this,portName,9600);
  //bluetoothPort.write(65);
  
  
}

void draw()
{
  int i = 0;
  int x = 12;
  int y = 30;
  int z = 45;
  String values = str(x) + " " + str(y) + " " + str(z) + "\n";
  while(i < 20){
    bluetoothPort.write(values);

    i++;
  }
  bluetoothPort.write(10);
  
  
  
}
/*
  void keyPressed() {
     if(key == 'a'){
       println("Sending...");
       bluetoothPort.write(0x6f);
     }  
  }*/
  //println("writing...");
  //bluetoothPort.write(65);

