#pragma once
/**
 * This file is part of HackTheBom.
 * HackTheBom is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation, either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * HackTheBom is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * HackTheBom. If not, see <https://www.gnu.org/licenses/>.
 * 
 * @file       : inclue/button.cpp
 * @author     : Maurice Snoeren (MS)
 * @description: This file implements the driver for the button. 
 * @date       : 16-06-2024
 * @version    : 1.0
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 * @todo       : 
 */
#include <driver.h>
#include <Arduino.h>

// update with counter, so anti-dender is created.

/* Class: Button
 * The button class provides high level function to control the button.
 */
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

   /* The setup method initializes the task. This method should be called once at the startup of the board.
    * When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
    * should be documented in the concrete task that implements this setup.
    *  
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
    */
   uint8_t setup() {
      pinMode(D3, INPUT_PULLUP);      

      Serial.println("Setup Button Ready!");

      return 0;
   }

   /* The loop method handles the main functionality. This loop method shall not contain any blocking function
      calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
      maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
      documented in the concrete task that implements this loop.
    *  
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
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
         if ( millis - this->timer > 5000 ) {
            this->longPressed = true;
         }
      } else {
         this->timer = millis;
      }

       return 0;
   }

   /* Return when the real button has been pressed. Cannot be used to determine whether it is pressed.
    *  
    * @param None
    * @return True when pressed, otherwise false.
    */
   bool buttonPressed() {
      return digitalRead(D3) == LOW;
   }

   /* Implements the pressing logics (anti-dender). Returns true when the button is really pressed.
    *  
    * @param None
    * @return True when really pressed, otherwise false.
    */
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

   /* Return when the button has been pressed for ten seconds.
    *  
    * @param None
    * @return True when long pressed, otherwise false.
    */
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
    *
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
    */
   uint8_t reset() {
      return 0;
   }

   /* Put the task to sleep and if possible in low power consumption mode.
    *
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
    */
   uint8_t sleep() {
      return 0;
   }

   /* Awake the task so it runs again.
    *
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
    */
   uint8_t wakeup() {
      return 0;
   }

};