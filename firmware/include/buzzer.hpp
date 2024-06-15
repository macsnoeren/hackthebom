#pragma once

#include <driver.h>
#include <Arduino.h>

enum Notes : uint32_t { // Octave 3
   C  = 131,
   CS = 139,
   D  = 147,
   DS = 156,
   E  = 165,
   F  = 175,
   FS = 185,
   G  = 196,
   GS = 208,
   A  = 220,
   AS = 233,
   B  = 247, 
};

class Buzzer: public IDriver {
private:
   uint64_t timer;
   uint8_t state;
   uint32_t  value; // 100 -> 40.000
   uint32_t notes[8] = {C, D, E, F, G, A, B, C*2};

public:
    Buzzer(): timer(0), state(0), value(100) {

    }

    ~Buzzer() {

    }

    /* The abstract setup method initializes the task. This method should be called once at the startup of the board.
       When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
       should be documented in the concrete task that implements this setup.
    */
    uint8_t setup() {
      pinMode(D8, OUTPUT);
      digitalWrite(D8, HIGH);

      Serial.println("Setup Buzzer Ready!");

      return 0;
    }

    /* The abstract loop method handles the main functionality. This loop method shall not contain any blocking function
       calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
       maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
       documented in the concrete task that implements this loop.
    */
    uint8_t loop(uint64_t millis) {
      if ( this->timer == 0 ) {
         this->timer = millis;
      
      } else {
         //if ( millis - this->timer > 1000 ) {
             //this->beep(C);
             //this->beep(D);
             //this->beep(E);
         //}

         if ( millis - this->timer > 2000 ) {
           this->timer = millis;
         }

      }

      
      return 0;
    }

   void on(uint32_t f = 1000) {
      analogWriteFreq(f);
      analogWrite(D8, 1);
   }

   void off() {
      analogWrite(D8, 0);
   }

   void beep(uint32_t f = 1000) {
      this->on(f);
      delay(5);
      this->off();
      delay(5);
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