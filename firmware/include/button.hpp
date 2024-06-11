#pragma once

#include <driver.h>
#include <Arduino.h>

// update with counter, so anti-dender is created.

class Button: public IDriver {
private:
   uint64_t timer;
   uint8_t value;

   bool pressed;
   bool longPressed;
   bool longPressedRead;

public:
    Button(): timer(0), value(0), longPressed(false), longPressedRead(false) {

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
      // low-pass filter to remove high freq of button press (anti-dender)
      if ( this->buttonPressed() ) {
         if ( this->value < 255 ) {
            this->value = this->value + 1;
         }
      } else {
         if ( this->value > 0 ) {
            this->value = this->value - 1;
         }
      }

      if ( this->pressed ) {
         if ( millis - this->timer > 10000 ) {
            this->longPressed = true;
         }
      } else {
         this->timer = millis;
      }

       return 0;
    }

    bool buttonPressed() {
      return digitalRead(D3) == LOW;
    }

    bool isPressed() {
      if ( this->value > 128 && !this->pressed ) {
         this->pressed = true;
         return true;
      }

      if ( this->value < 128 && this->pressed ) {
         this->pressed = false;
      }

      return false;
    }

    bool isLongPressed() {
      if ( this->longPressed && !this->longPressedRead ) {
         this->longPressedRead = true;
         return true;
      }

      if ( !this->pressed && this->longPressedRead ) {
         this->longPressed = false;
         this->longPressedRead = false;
      }

      return false;
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