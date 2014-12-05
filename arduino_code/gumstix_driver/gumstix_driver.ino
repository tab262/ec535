#include <TimerOne.h>


//comment this out to see the demodulated waveform
//it is useful for debugging purpose.
#define MODULATED 1
int DELAY = 0; 
const int IR_PIN = 3;

const int THROTTLE_INCDEC_PIN = 13;
const int THROTTLE_RATE_PIN = 12;
const int YAW_INCDEC_PIN = 11;
const int YAW_RATE_PIN = 10;
const int PITCH_INCDEC_PIN = 9;
const int PITCH_RATE_PIN = 8;

const int TEST_PIN = 4;


const unsigned long DURATION = 180000l;
const int HEADER_DURATION = 2000;
const int HIGH_DURATION = 380;
const int ZERO_LOW_DURATION = 220;
const int ONE_LOW_DURATION = 600;
const byte ROTATION_STATIONARY = 60;
const byte CAL_BYTE = 52;

//cmd variables
int throttle, yaw, pitch;

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
    if (b > 0) sendOne(); 
    else sendZero();
  }

  for (int i = 7; i >=0; i--)
  {
    b = ((63 + forwardBackward) & (1 << i)) >> i;   
    if (b > 0) sendOne(); 
    else sendZero();
  }

  for (int i = 7; i >=0; i--)
  {
    b = (throttle & (1 << i)) >> i;   
    if (b > 0) sendOne(); 
    else sendZero();
  }

  for (int i = 7; i >=0; i--)
  {
    b = (CAL_BYTE & (1 << i)) >> i;   
    if (b > 0) sendOne(); 
    else sendZero();
  }

}

void setup()
{
  // Setup pin for outputting IR signal
  pinMode(IR_PIN, OUTPUT);
  digitalWrite(IR_PIN, LOW);

  // These pins server as control inputs. It will interpret
  // the signals as 'binary' inputs to perform certain commands
  pinMode(THROTTLE_INCDEC_PIN, INPUT);
  pinMode(THROTTLE_RATE_PIN, INPUT);
  pinMode(YAW_INCDEC_PIN, INPUT);
  pinMode(YAW_RATE_PIN, INPUT);
  pinMode(PITCH_INCDEC_PIN, INPUT);
  pinMode(PITCH_RATE_PIN, INPUT);

  // The test is simply used to have a digital 1 output to test with
  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, HIGH);

  // Setting the initial cmd values
  throttle = 0;
  pitch = 0;
  yaw = 0;

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
    The Gumstix communicates with the Arduino via a parellel communciation standard.
   For each of the three control signals, two pins (bits) specify the desired
   command.  
   
   X_INCDEC:  1 = Increase current value
   0 = Decrease current value
   X_RATE:    1 = Times 5 Multiplier
   0 = Times 1 Multiplier
   */

  //set the throttle
  if(digitalRead(THROTTLE_INCDEC_PIN)){
    if(digitalRead(THROTTLE_RATE_PIN)){
      throttle += 5;
    }
    else{
      throttle += 1;
    }
  }
  else{
    if(digitalRead(THROTTLE_RATE_PIN)){
      throttle -= 5;
    }
    else{
      throttle -= 1;
    }
  }

  //set the pitch
  if(digitalRead(PITCH_INCDEC_PIN)){
    if(digitalRead(PITCH_RATE_PIN)){
      pitch += 5;
    }
    else{
      pitch += 1;
    }
  }
  else{
    if(digitalRead(PITCH_RATE_PIN)){
      pitch -= 5;
    }
    else{
      pitch -= 1;
    }
  }

  //set the yaw
  if(digitalRead(YAW_INCDEC_PIN)){
    if(digitalRead(YAW_RATE_PIN)){
      yaw += 5;
    }
    else{
      yaw += 1;
    }
  }
  else{
    if(digitalRead(YAW_RATE_PIN)){
      yaw -= 5;
    }
    else{
      yaw -= 1;
    }
  }

  sendCommand(throttle, yaw, pitch);
  Serial.println("TIMER");


}

