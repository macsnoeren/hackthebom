#pragma once

#include <driver.h>
#include <Arduino.h>

class Buzzer: public IDriver {
private:
   uint64_t timer;
   uint8_t state;   

public:
    Buzzer(): timer(0), state(0) {

    }

    ~Buzzer() {

    }

    /* The abstract setup method initializes the task. This method should be called once at the startup of the board.
       When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
       should be documented in the concrete task that implements this setup.
    */
    uint8_t setup() {
      pinMode(D4, OUTPUT);
      digitalWrite(D4, HIGH);
      

      Serial.println("Setup Buzzer Ready!");

      return 0;
    }

    /* The abstract loop method handles the main functionality. This loop method shall not contain any blocking function
       calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
       maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
       documented in the concrete task that implements this loop.
    */
    uint8_t loop(uint64_t millis) {
      
      return 0;
    }

   void on() {
      digitalWrite(D4, HIGH);
   }

   void off() {
      digitalWrite(D4, LOW);
   }

   void beep() {
      digitalWrite(D4, LOW);
      delay(500);
      digitalWrite(D4, HIGH);
      delay(500);
   }

    /* The abstract reset function resets the task. If successfull the method returns 0, otherwise it returns an error
       number.
    */
    uint8_t reset() {
      return 0;
    }

    /* Put the task to sleep and if possible in low power consumption mode. */
    uint8_t sleep() {
      return 0;
    }

    /* Awake the task so it runs again. */
    uint8_t wakeup() {
      return 0;
    }

};