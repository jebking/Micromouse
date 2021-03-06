/********************************************************
 * PID RelayOutput Example
 * Same as basic example, except that this time, the output
 * is going to a digital pin which (we presume) is controlling
 * a relay.  the pid is designed to Output an analog value,
 * but the relay can only be On/Off.
 *
 * To connect them together we use "time proportioning
 * control"  it's essentially a really slow version of PWM.
 * first we decide on a window size (5000mS say.) we then
 * set the pid to adjust its output between 0 and that window
 * size.  lastly, we add some logic that translates the PID
 * output into "Relay On Time" with the remainder of the
 * window being "Relay Off Time"
 *
 * This version, uses SetOutputMapping method, not only to cut off
 * low and high output values, but also map the original range 0-255
 * to our time window (0-windowSize) which is based on my experience
 * and couple of tests, working better than original example, when the
 * window is much larger than maxOutput.
 * Also, it counts on default SetMode(AUTOMATIC) setting, which is moved
 * to constructor as it's by default what we want.
 ********************************************************/

#include <PID_v1.h>

#define PIN_INPUT 0
#define RELAY_PIN 6

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 5000;
unsigned long windowStartTime;

void setup()
{
  windowStartTime = millis();

  // Set RELAY_PIN as output
  pinMode(RELAY_PIN,OUTPUT);
  
  //initialize the variables we're linked to
  Setpoint = 100;

  //tell the PID to map between 0 and the full window size
  myPID.SetOutputMapping(0, WindowSize);

}

void loop()
{
  Input = analogRead(PIN_INPUT);
  myPID.Compute();

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
  if (millis() - windowStartTime > WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  
  //output is already between 0 and WindowSize.
  if (Output < millis() - windowStartTime) digitalWrite(RELAY_PIN, HIGH);
  else digitalWrite(RELAY_PIN, LOW);

}



