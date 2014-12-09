  #include <TimerOne.h>
 
//comment this out to see the demodulated waveform
//it is useful for debugging purpose.
#define MODULATED 1
int DELAY = 0; 
const int IR_PIN = 3;
//const int test_pin = 5;
const unsigned long DURATION = 180000l;
const int HEADER_DURATION = 2000;
const int HIGH_DURATION = 380;
const int ZERO_LOW_DURATION = 220;
const int ONE_LOW_DURATION = 600;
const byte ROTATION_STATIONARY = 60;
const byte CAL_BYTE = 52;
 
int Throttle, LeftRight, FwdBack,Incr;
 
void sendHeader()
{
  #ifndef MODULATED
    digitalWrite(IR_PIN, HIGH);
  #else
    TCCR2A |= _BV(COM2B1);
  #endif
   
  delayMicroseconds(HEADER_DURATION);
   
  #ifndef MODULATED
    digitalWrite(IR_PIN, LOW);
  #else
    TCCR2A &= ~_BV(COM2B1);
  #endif
   
  delayMicroseconds(HEADER_DURATION);
   
  #ifndef MODULATED
    digitalWrite(IR_PIN, HIGH);
  #else
    TCCR2A |= _BV(COM2B1);
  #endif
   
  delayMicroseconds(HIGH_DURATION);
   
  #ifndef MODULATED
    digitalWrite(IR_PIN, LOW);
  #else
    TCCR2A &= ~_BV(COM2B1);
  #endif
}
 
void sendZero()
{
  delayMicroseconds(ZERO_LOW_DURATION);
 
  #ifndef MODULATED
    digitalWrite(IR_PIN, HIGH);
  #else 
    TCCR2A |= _BV(COM2B1);
  #endif
   
  delayMicroseconds(HIGH_DURATION);
   
  #ifndef MODULATED
    digitalWrite(IR_PIN, LOW);
  #else
    TCCR2A &= ~_BV(COM2B1);
  #endif
}
 
void sendOne()
{
  delayMicroseconds(ONE_LOW_DURATION);
   
  #ifndef MODULATED
    digitalWrite(IR_PIN, HIGH);
  #else
    TCCR2A |= _BV(COM2B1);
  #endif
   
  delayMicroseconds(HIGH_DURATION);
   
  #ifndef MODULATED
    digitalWrite(IR_PIN, LOW); 
  #else
    TCCR2A &= ~_BV(COM2B1);
  #endif
}
 
void sendCommand(int throttle, int leftRight, int forwardBackward)
{
  byte b;
 
  sendHeader();
  //Serial.println(throttle); 
  for (int i = 7; i >=0; i--)
  {
    b = ((ROTATION_STATIONARY + leftRight) & (1 << i)) >> i;   
    if (b > 0) sendOne(); else sendZero();
  }
   
  for (int i = 7; i >=0; i--)
  {
    b = ((63 + forwardBackward) & (1 << i)) >> i;   
    if (b > 0) sendOne(); else sendZero();
  }
   
  for (int i = 7; i >=0; i--)
  {
    b = (throttle & (1 << i)) >> i;   
    if (b > 0) sendOne(); else sendZero();
  }
   
  for (int i = 7; i >=0; i--)
  {
    b = (CAL_BYTE & (1 << i)) >> i;   
    if (b > 0) sendOne(); else sendZero();
  }
  
}
 
void setup()
{
  pinMode(IR_PIN, OUTPUT);
  //pinMode(test_pin, OUTPUT);
  digitalWrite(IR_PIN, LOW);
  //digitalWrite(test_pin, HIGH);
  Throttle = 0;
  Incr = 5;
  //setup interrupt interval: 180ms 
  Timer1.initialize(DURATION);
  Timer1.attachInterrupt(timerISR);
  
  Serial.begin(115200);
  //setup PWM: f=38Khz PWM=0.5 
  byte v = 8000 / 38;
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = v;
  OCR2B = v / 2;
}
 
void loop()
{ 
    
}

void timerISR()
{
  //read control values from potentiometers
  /*
  Throttle = Throttle + Incr;
//  if(Throttle > 130){
  //  Incr = -1;
  //}
  
  if(Throttle < 120){
    Incr = 1; 
  }
  */
  if(Serial.available() > 0){
    
    byte inByte = Serial.read();
    Serial.println(inByte + "|" + Throttle);
   
    if(inByte == '1'){
      if(Throttle < 200){
        Throttle = Throttle + 5;
      }
    }else{
      if(Throttle > 0){
        Throttle = Throttle - 5;
      }  
    }
    Serial.println(Throttle);
  }
  //Serial.read();
  //DELAY = DELAY + 1;

/*
  if(Serial.available() > 0){
    byte inByte = Serial.read();
    Serial.println(Throttle);

    if(inByte == 's'){
       FwdBack = FwdBack + Incr; 
    }else if(inByte == 'w'){
       FwdBack = FwdBack - Incr; 
    }else if(inByte == 'q'){
      Throttle = Throttle - Incr;
    }else if(inByte == 'e'){
      Throttle = Throttle + Incr;
    }else if(inByte == 'd'){
      LeftRight = LeftRight -Incr;
    }else if(inByte == 'a'){
      LeftRight = LeftRight + Incr;
    }else if(inByte == 'r'){
       Throttle = 0;
       FwdBack = 0;
       LeftRight = 0; 
    }

  }
*/

  //Serial.println(Throttle);
  //LeftRight = 0;//analogRead(1);
  //FwdBack = 0;//analogRead(2);
  //Serial.println(Throttle); 
  //Throttle = Throttle / 4; //convert to 0 to 255
  //LeftRight = LeftRight / 8 - 64; //convert to -64 to 63
  //FwdBack = FwdBack / 4 - 128; //convert to -128 to 127
  

  sendCommand(Throttle, LeftRight, FwdBack);
  //Serial.println(Throttle);
  

}


