/*
 * Using accelerated motion ("linear speed") in nonblocking mode
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include <Arduino.h>

#define HIGHSPEEDPIN 19
#define LOWSPEEDPIN 18

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
// Target RPM for cruise speed
#define RPM 95
#define HIGHSPEED 70
#define LOWSPEED 20
// Acceleration and deceleration values are always in FULL steps / s^2
#define MOTOR_ACCEL 3000
#define MOTOR_DECEL 3000

// Microstepping mode. If you hardwired it to save pins, set to the same value here.
#define MICROSTEPS 16

#define DIR 6
#define STEP 3
#define ENABLE 8
/*
 * Choose one of the sections below that match your board
 */
// #include "DRV8825.h"
// #define MODE0 10
// #define MODE1 11git 
// #define MODE2 12
// DRV8825 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MODE0, MODE1, MODE2);

// #include "DRV8880.h"
// #define M0 10
// #define M1 11
// #define TRQ0 6
// #define TRQ1 7
// DRV8880 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, M0, M1, TRQ0, TRQ1);

// #include "BasicStepperDriver.h" // generic
// BasicStepperDriver stepper(DIR, STEP);

//#include "DRV8834.h"
//#define M0 10
//#define M1 11
//DRV8834 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, M0, M1);

#include "A4988.h"
A4988 stepper(MOTOR_STEPS, DIR, STEP, ENABLE);


void setup() {
    Serial.begin(115200);
    pinMode(LOWSPEEDPIN, INPUT_PULLUP);
    
    pinMode(HIGHSPEEDPIN, INPUT_PULLUP);

    stepper.begin(RPM, MICROSTEPS);
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    
    stepper.setEnableActiveState(LOW);
    // set current level (for DRV8880 only). Valid percent values are 25, 50, 75 or 100.
    // stepper.setCurrent(100);

    /*
     * Set LINEAR_SPEED (accelerated) profile.
     */
    stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
    stepper.enable();
//    stepper.startRotate(5000); 
    stepper.startConstantMove(true);
    Serial.println("START");
    /* startrotate is nonblocking
     * Using non-blocking mode to print out the step intervals.
     * We could have just as easily replace everything below this line with 
     * stepper.rotate(360);
     */
}

void loop() {
  unsigned wait_time = stepper.nextAction();


  static unsigned int debounce;
  static unsigned int lls;
  static unsigned int lhs;
  
  unsigned int ls = digitalRead(LOWSPEEDPIN);
  unsigned int hs = digitalRead(HIGHSPEEDPIN);

  if(ls == lls && hs == lhs){
    debounce++;
    if(debounce > 20){
      debounce = 20;
    }
  }else{
    debounce=0;
  }
  lls = ls;
  lhs = hs;

//  Serial.print(ls);
//  Serial.println(hs);
  if(debounce < 20) return;
  
  if(!ls){
    if(stepper.getRPM() != LOWSPEED){
      Serial.println("LOW SPEED");
      stepper.enable();
      stepper.setRPM(LOWSPEED);
      stepper.startConstantMove(true);
    }
  }else if(!hs){
    if(stepper.getRPM() != HIGHSPEED){
      stepper.enable();
      Serial.println("HIGH SPEED");
      stepper.setRPM(HIGHSPEED);
      stepper.startConstantMove(true);
    }
  }else{
    if(stepper.getRPM() != 0){
      Serial.println("OFF");
      stepper.disable();
      stepper.setRPM(0);
      stepper.startConstantMove(true);
    }
  }
}
