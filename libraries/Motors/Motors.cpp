#include "Arduino.h"
#include "Motors.h"

//initialise counters at 0
volatile int _counterL = 0;
volatile int _counterR = 0;

void onTickL();
void onTickR();
void wait();

Motors::Motors(int drivepinL, int drivepinR, int tickpinL,
               int tickpinR, int phasepinL, int phasepinR)
{
  pinMode(tickpinL, INPUT);
  pinMode(tickpinR, INPUT);
  attachInterrupt(digitalPinToInterrupt(tickpinL), onTickL, RISING);
  attachInterrupt(digitalPinToInterrupt(tickpinR), onTickR, RISING);
  pinMode(drivepinL, OUTPUT);
  pinMode(drivepinR, OUTPUT);
  pinMode(phasepinL, OUTPUT);
  pinMode(phasepinR, OUTPUT);

  _drivepinL = drivepinL;
  _drivepinR = drivepinR;
  _phasepinL = phasepinL;
  _phasepinR = phasepinR;
  releaseFlag = 0;

  digitalWrite(phasepinL, LOW);
  digitalWrite(phasepinR, LOW);
}


void Motors::oneMotor(int pin, int* counter, int pwm, int tickDelta)
{

  *counter = 0;

  /* if too many turns then stop */

  while (*counter < tickDelta) {
    analogWrite(pin, pwm);
  }
  analogWrite(pin, 0);
}

void Motors::accForward(int start_pwm, int max_pwm, int tickDelta)
{
  accForward(start_pwm, max_pwm, tickDelta, -1);
}


void Motors::accForward(int start_pwm, int max_pwm,
                        int tickDelta, int forwardIRPin)
{
  if(releaseFlag){
    return;
      }

  _counterL = 0;
  _counterR = 0;

  float acc_pwm = start_pwm;

  float deltaTime = 0;
  int prevTime = millis();

  /*check motors are in sync
    if not then stop motor with higher count */
  unsigned long startTime = millis();
  while ((_counterR < tickDelta || _counterL < tickDelta) &&
         (forwardIRPin == -1 || analogRead(forwardIRPin) <= 240))
  {

    if(millis() - startTime > 5000){
      analogWrite(_drivepinL,0);
      analogWrite(_drivepinR,0);
      while(!releaseFlag){
      }
      return;
    }

    int pwm = acc_pwm;
    if (pwm > max_pwm)
    {
      pwm = max_pwm;
    }

    if (_counterL > _counterR)
    {
      analogWrite(_drivepinL, 0);
      analogWrite(_drivepinR, pwm);
    }
    else if (_counterL < _counterR)
    {
      analogWrite(_drivepinL, pwm);
      analogWrite(_drivepinR, 0);
    }
    else
    {
      analogWrite(_drivepinL, pwm);
      analogWrite(_drivepinR, pwm);
    }

    int curTime = millis();
    deltaTime = curTime - prevTime;
    prevTime = curTime;
    if ((_counterL + _counterR) < tickDelta)
    {
      acc_pwm += deltaTime / 2;
    }
    else
    {
      acc_pwm = start_pwm;
    }
  }
  analogWrite(_drivepinL, 0);
  analogWrite(_drivepinR, 0);

}

void Motors::forward(int pwm, int tickDelta)
{
  accForward(pwm, pwm, tickDelta);
}

void Motors::forward(int pwm, int tickDelta, int forwardIRPin)
{
  accForward(pwm, pwm, tickDelta, forwardIRPin);
}

/* 110 ticks to go 90 degrees
 * 220 ticks to go 180 degrees */

void Motors::turnLeft()
{
  /* motors initially set HIGH forward.
     to turn left set L to low and then reset to high when complete */

  digitalWrite(_phasepinL, HIGH);
  forward(60, 105);
  digitalWrite(_phasepinL, LOW);

}

void Motors::turnRight()
{
  /* same as above but change phasepinR */

  digitalWrite(_phasepinR, HIGH);
  forward(60, 105);
  digitalWrite(_phasepinR, LOW);

}

void Motors::turnAroundLeft()
{

  digitalWrite(_phasepinL, HIGH);
  forward(60, 215);
  digitalWrite(_phasepinL, LOW);

}

void Motors::turnAroundRight()
{

  digitalWrite(_phasepinR, HIGH);
  forward(60, 215);
  digitalWrite(_phasepinR, LOW);

}

void wait(int deltaTime)
{
  unsigned long previousMillis = millis();
  unsigned long currentMillis = millis();
  while ((unsigned long)(currentMillis - previousMillis) < deltaTime){
    currentMillis = millis();
  }
}

void Motors::bump(int pwm)
{
  if(releaseFlag){
    return;
  }
  analogWrite(_drivepinL, pwm);
  analogWrite(_drivepinR, pwm);
  int prevCounterL;
  int prevCounterR;
  do
  {
    prevCounterL = _counterL;
    prevCounterR = _counterR;

    wait(100);
  }
  while ((_counterL + _counterR) - (prevCounterL + prevCounterR) > 5);
  analogWrite(_drivepinL, 0);
  analogWrite(_drivepinR, 0);
}

void Motors::wallOrientateFwd()
{
  bump(40);

  digitalWrite(_phasepinL, HIGH);
  digitalWrite(_phasepinR, HIGH);

  forward(60, 16);

  digitalWrite(_phasepinL, LOW);
  digitalWrite(_phasepinR, LOW);
}

void Motors::wallOrientateBkwd()
{
  digitalWrite(_phasepinL, HIGH);
  digitalWrite(_phasepinR, HIGH);

  bump(40);

  digitalWrite(_phasepinL, LOW);
  digitalWrite(_phasepinR, LOW);

  forward(60, 86);
}

//increment count with each turn

void onTickL()
{
  _counterL++;

}

void onTickR()
{
  _counterR++;
}
