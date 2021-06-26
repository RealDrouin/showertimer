// Smart Shower Version 1.3  with ATTiny85 ,SleepMode ,Internal Clock 1MHZ
//
// Shower Timer Setting between 7 min - 15 min , increment by 1 min.
//
// Lock Timer Setting between 0 min - 30 min, increment by 5 min.
//
// Pause Function 3 min max after continue cycle.
//
// Made By Real Drouin Junior
// Date 28-jan-2018
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+
//************************************************************
//      Connection For Programming Attiny 25/45/85
//                         +-\/-+
// Pin D10 on Nano or Uno 1|    |8  Vcc
//                        2|    |7  Pin D13 on Nano or Uno
//                        3|    |6  Pin D12 on Nano or Uno
//                   GND  4|    |5  Pin D11 on Nano or Uno
//                         +----+
// Add Led (HeartBeat) with resistor 220ohm at pin D9 Nano or Uno
// Add 10uf Capacitor at pin Rst and Gnd on Nano or Uno
// Program Nano or Uno With ArduinoISP.
//************************************************************

#include <avr/sleep.h>   
#include <avr/power.h>
ISR (PCINT0_vect)
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) 
{
  // do something interesting here
}

const byte ledPin = 1;        // Led on Pin 6
const byte switchPin = 0;     // Switch on Pin 5
boolean switchStatus = HIGH;
const byte openvalve = 2;    // pin 7
const byte closevalve = 3;   // pin 2
byte timer = 6;  // timer shower enable default 7min
byte s = 0;      // sec counter
byte m = 0;      // minute counter
byte wait = 0;   // lock timer default 0min
byte flash = 0;
boolean pause = 0;
byte pausedelay = 0; // pause 3min max. after continu cycle
const byte driverenable = 4; // pin 3 to pin 1 of ic l293d driver

//*******************************************************************
void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(openvalve, OUTPUT);
  pinMode(closevalve, OUTPUT);
  pinMode(driverenable, OUTPUT);
  digitalWrite (driverenable, LOW);
  digitalWrite (openvalve, LOW);
  digitalWrite (closevalve, LOW);
  digitalWrite (ledPin, HIGH);

  // pin change interrupt (example for D0)
  PCMSK  |= bit (PCINT0);  // want pin D0 / pin 5
  GIFR   |= bit (PCIF);    // clear any outstanding interrupts
  GIMSK  |= bit (PCIE);    // enable pin change interrupts
  adc_disable(); // ADC uses ~320uA
  delay(1000);

  on();  // Open Valve When Reset

  while (switchStatus == HIGH)
  {
    switchStatus = digitalRead(switchPin);
  }

  digitalWrite (ledPin, LOW);
  delay(2000);

  while (s < 50)
  {
    s = s + 1;
    delay (100);
    digitalWrite (ledPin, HIGH);
    switchStatus = digitalRead(switchPin);
    delay (100);
    digitalWrite (ledPin, LOW);

    if (switchStatus == LOW)
    {
      delay (1000);
      s = 0;
      timer = timer + 1;

      if (timer > 15)
      {
        timer = 7;
      }

      while (flash < timer)
      {
        delay (250);
        digitalWrite (ledPin, HIGH);
        delay (250);
        digitalWrite (ledPin, LOW);
        flash = flash + 1;
      }
      delay (1000);
      flash = 0;
    }

    if (timer == 6) {
      timer = 7;
    }
  }

  // Lock Timer Setting 0 min - 30 min, increment by 5 min.

  s = 0;
  digitalWrite (ledPin, HIGH);
  while (switchStatus == HIGH)
  {
    switchStatus = digitalRead(switchPin);
  }

  digitalWrite (ledPin, LOW);
  delay(2000);

  while (s < 50)
  {
    s = s + 1;
    delay (100);
    digitalWrite (ledPin, HIGH);
    switchStatus = digitalRead(switchPin);
    delay (100);
    digitalWrite (ledPin, LOW);

    if (switchStatus == LOW)
    {
      delay (1000);
      s = 0;

      wait = wait + 5;

      if (wait > 30)
      {
        wait = 5;
      }

      while (flash < (wait / 5))
      {
        delay (250);
        digitalWrite (ledPin, HIGH);
        delay (250);
        digitalWrite (ledPin, LOW);
        flash = flash + 1;
      }
      delay (1000);
      flash = 0;
    }
  }

  // END Setting.

  digitalWrite (ledPin, HIGH);
  delay(1000);
  digitalWrite (ledPin, LOW);
  s = 0;
  timer = timer - 1;
  off();
}

//**********************************************
void loop()
{
  s = 0;
  m = 0;
  switchStatus = digitalRead(switchPin);

  if (switchStatus == LOW)
  {
    digitalWrite (ledPin, HIGH);
    delay (2000);
    on();
    start();
    lastminute();
    digitalWrite(ledPin, LOW);
    hold();
  }
  goToSleep ();
}
//**************************************
void on()
{
  digitalWrite(driverenable, HIGH);
  delay(60);
  digitalWrite(openvalve, HIGH);
  delay(50);
  digitalWrite(openvalve, LOW);
  digitalWrite(driverenable, LOW);
}
//***************************************
void off()
{
  digitalWrite(driverenable, HIGH);
  delay(60);
  digitalWrite(closevalve, HIGH);
  delay(50);
  digitalWrite(closevalve, LOW);
  digitalWrite(driverenable, LOW);
}

//**************************************
void start()
{
  s = 0;
  m = 0;

  while (m < timer)
  {
    switchStatus = digitalRead(switchPin);

    if (switchStatus == LOW && pause == 0)
    {
      off();
      digitalWrite (ledPin, HIGH);
      pause = 1;
      delay(2000);

      while (pause == 1)
      {
        delay(1000);
        pausedelay ++;

        switchStatus = digitalRead(switchPin);
        if (switchStatus == LOW && pause == 1) {
          on();
          pause = 0;
          pausedelay = 0;
        }

        if (pausedelay == 180) {
          on();
          pause = 0;
        }
      }
    }

    digitalWrite (ledPin, HIGH);
    delay(30);
    digitalWrite (ledPin, LOW);
    delay(970);
    s = s + 1;
    if (s >= 60)
    {
      s = 0;
      m = m + 1;
    }
  }
}

//**************************************
void lastminute()
{
  s = 0;
  m = 0;

  off();
  delay(2000);
  on();

  while (s < 60)
  {
    switchStatus = digitalRead(switchPin);

    if (switchStatus == LOW && pause == 0)
    {
      off();
      digitalWrite (ledPin, HIGH);
      pause = 1;
      delay(2000);

      while (pause == 1)
      {
        delay(1000);
        pausedelay ++;

        switchStatus = digitalRead(switchPin);
        if (switchStatus == LOW && pause == 1) {
          on();
          pause = 0;
          pausedelay = 0;
        }

        if (pausedelay == 180) {
          on();
          pause = 0;
        }
      }
    }

    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(800);
    s = s + 1;
  }
  digitalWrite (ledPin, LOW);

  off();
  pause = 0;
  pausedelay = 0;
}

//*********************************************
void hold()
{
  delay(5000);
  s = 0;
  m = 0;
  while (m < wait)
  {
    delay(4970);
    digitalWrite (ledPin, HIGH);
    delay(30);
    digitalWrite (ledPin, LOW);
    s = s + 5;
    if (s >= 60)
    {
      s = 0;
      m = m + 1;
    }
  }
}

//**********************************************
void goToSleep ()
{
  digitalWrite (ledPin, LOW);
  delay(200);
  digitalWrite (ledPin, HIGH);
  delay (50);
  digitalWrite (ledPin, LOW);
  delay(200);
  digitalWrite (ledPin, HIGH);
  delay (50);
  digitalWrite (ledPin, LOW);

  ADCSRA = 0;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  power_all_disable ();
  sleep_enable();
  sleep_cpu();
  sleep_disable();
  power_all_enable();
  adc_disable(); // ADC uses ~320uA
}
