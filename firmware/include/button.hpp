#pragma once

#include <driver.h>
#include <Arduino.h>

enum BUTTON_STATE {
   ON,
   OFF
};

class Button: public IDriver {
private:
   uint64_t timer;
   uint8_t state;   
   bool buttonPressed;
   bool longPressed;
   bool longPressedStarted;

public:
    Button(): timer(0), state(0), buttonPressed(false), longPressed(false), longPressedStarted(false) {

    }

    ~Button() {

    }

    /* The abstract setup method initializes the task. This method should be called once at the startup of the board.
       When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
       should be documented in the concrete task that implements this setup.
    */
    uint8_t setup() {
      pinMode(D3, INPUT_PULLUP);      

      Serial.println("Setup Button Ready!");

      return 0;
    }

    /* The abstract loop method handles the main functionality. This loop method shall not contain any blocking function
       calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
       maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
       documented in the concrete task that implements this loop.
    */
    uint8_t loop(uint64_t millis) {
      
      if ( !this->longPressedStarted && this->pressed() ) {
         this->longPressedStarted = true;
         this->timer = millis;
      }

      if ( this->longPressedStarted ) {
         if ( millis - this->timer > 10000 ) {
            this->longPressed = true;
            this->longPressedStarted = false;
         }
      }

      if ( !this->pressed() ) {
         if ( this->longPressedStarted ) {
            this->buttonPressed = true;
         }
         this->longPressedStarted = false;
      }

      return 0;
    }

    bool pressed() {
      return digitalRead(D3) == LOW;
    }

    bool isPressed() {
      bool r = this->buttonPressed;
      this->buttonPressed = false;
      return r;
    }

    bool isLongPressed() {
      bool r = this->longPressed;
      this->longPressed = false;
      return r;
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