  #include <TimerOne.h>
 
//comment this out to see the demodulated waveform
//it is useful for debugging purpose.
#define MODULATED 1
int DELAY = 0; 
const int IR_PIN = 3;
// Select Pins: 13, 12, 11, 10
const int SELECT0_PIN = 13;
const int SELECT1_PIN = 12;
const int SELECT2_PIN = 11;
const int SELECT3_PIN = 10;
const int SELECT4_PIN = 9;

const int TEST_PIN = 4;


const unsigned long DURATION = 180000l;
const int HEADER_DURATION = 2000;
const int HIGH_DURATION = 380;
const int ZERO_LOW_DURATION = 220;
const int ONE_LOW_DURATION = 600;
const byte ROTATION_STATIONARY = 60;
const byte CAL_BYTE = 52;
 
int Throttle, LeftRight, FwdBack,increment, myDirection, rate;
int val0,val1,val2,val3,val4;

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
  // Setup pin for outputting IR signal
  pinMode(IR_PIN, OUTPUT);
  digitalWrite(IR_PIN, LOW);
  
  // These pins server as control inputs. It will interpret
  // the signals as 'binary' inputs to perform certain commands
  pinMode(SELECT0_PIN, INPUT);
  pinMode(SELECT1_PIN, INPUT);
  pinMode(SELECT2_PIN, INPUT);
  pinMode(SELECT3_PIN, INPUT);
  pinMode(SELECT4_PIN, INPUT);
  
  // The test is simply used to have a digital 1 output to test with
  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, HIGH);
  
  // Setting the initial Throttle to 0
  Throttle = 0;
  increment = 1;
  myDirection = 0;
  
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
    Serial.println("LOOP");
}

void timerISR()
{

  /*
  2 bits: parameter_select:
  0: Throttle
  1: LeftRight
  3: FwdBack
  4: Trim
  
  direction:
  0: Decrease
  1: Increase
  
  rate:
  0: +/- 1
  1: +/- 5
  
  enable:
  0: no
  1: yes
  */
  
  
  int val0,val1,val2,val3,val4;
  val0 = digitalRead(SELECT0_PIN);
  val1 = digitalRead(SELECT1_PIN);
  val2 = digitalRead(SELECT2_PIN);
  val3 = digitalRead(SELECT3_PIN);
  val4 = digitalRead(SELECT4_PIN);

  int parameter_select = val0 + (2*val1);
  
  if(!val4){
     return; 
  }
  
  if(val2){
     myDirection = 1; 
  }else{
    myDirection = -1;
  }
  
  if(val3){
     rate = 5; 
  }else{
     rate = 1; 
  }
  
  increment = myDirection * rate;
  
  
  
  if(parameter_select == 0){
     Throttle += increment; 
  }else if(parameter_select == 1){
     LeftRight += increment; 
  }else if(parameter_select == 2){
     FwdBack += increment; 
  }

  
  
  
 
  sendCommand(Throttle, LeftRight, FwdBack);
  Serial.println("TIMER");
    

}


