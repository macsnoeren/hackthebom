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
 * @file       : inclue/timer.cpp
 * @author     : Maurice Snoeren (MS)
 * @description: This file implements the driver for the 7-segments display using the I2C protocol. 
 * @date       : 16-06-2024
 * @version    : 1.0
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 * @todo       : 
 */
#include <driver.h>

#include <HT16K33.h>

// Timer state to implement specific function when this is selected.
enum TimerState {
   START,
   CONFIG,
   COUNTDOWN,
   FINISH,
};

/* Class: Timer
 * The timer class provides high level function to control the display that is connected.
 */
class Timer: public IDriver {
private:
   uint64_t timer; // Timer that is used for timing purposes
   uint8_t state; // State is used to determine which functionality needs to be executed
   HT16K33 seg; // The library that handles the hardware based on the chip HT16K33
   uint32_t totalMinutes; // Total minutes that will be default selected
   uint32_t minutes; // Total minutes left
   uint32_t seconds; // Total seconds left
   bool dash; // Show the dash or not
   

public:
   Timer(): timer(0), state(START), seg(0x70), totalMinutes(30), minutes(0), seconds(0), dash(true) {

   }

   ~Timer() {

   }

   /* The setup method initializes the task. This method should be called once at the startup of the board.
    * When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
    * should be documented in the concrete task that implements this setup.
    *  
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
    */
   uint8_t setup() {
      // Setup the I2C library to connect to the 7-segments display on 400kHz.
      Wire.begin();
      Wire.setClock(400000);
   
      // Setup the HT16K33 library to connect with the display.
      seg.begin();
      seg.displayOn();
      seg.setBrightness(4);
      seg.setDigits(4);

      // Initially show the dashes on the display.
      this->showDashes();

      Serial.println("Setup Timer Ready!");

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
      if ( this->state == COUNTDOWN ) {
         if ( millis - this->timer > 1000 ) {
            if (this->seconds == 0 ) {
               if ( this->minutes == 0 ) {
                  this->state = FINISH;
                  this->showLose();
               } else {
                  this->minutes--;
                  this->seconds = 59;
               }
            } else {
               this->seconds--;
            }
            this->timer = millis;
            this->dash = !this->dash;
            seg.displayTime(this->minutes, this->seconds, this->dash, true);
         }
      }
      
      return 0;
   }

   /* Check whether the timer reacher zero minutes and zero seconds.
    *  
    * @param None
    * @return True when the timer is zero, otherwise false.
    */
   bool isTimerZero () {
      return this->minutes == 0 && this->seconds == 0;
   }

   /* Show LOSE on the display.
    *  
    * @param None
    * @return None
    */
   void showLose() {
      uint8_t x[4] = { SEG_D | SEG_E | SEG_F,
                       SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
                       SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
                       SEG_A | SEG_D | SEG_E | SEG_F | SEG_G };
      seg.displayRaw(x, false);
      this->state = FINISH;
   }

   /* Show YEAH on the display.
    *  
    * @param None
    * @return None
    */
   void showYeah() {
      uint8_t x[4] = { SEG_B | SEG_C | SEG_D | SEG_F| SEG_G,
                       SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
                       SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
                       SEG_B | SEG_C | SEG_E | SEG_F | SEG_G };
      seg.displayRaw(x, false);
      this->state = FINISH;
   }

   /* Show dashes --:-- on the display.
    *  
    * @param None
    * @return None
    */
   void showDashes() {
      uint8_t x[4] = { SEG_G, SEG_G, SEG_G, SEG_G };
      seg.displayRaw(x, true);
      this->state = START;
   }

   /* Blink the display.
    *  
    * @param None
    * @return None
    */
   void blink(bool on) {
      this->seg.setBlink( (on ? 2 : 0) );
   }

   /* Show the time on the display.
    *  
    * @param minutes: 0-99 minutes on the display
    * @param seconds: 0-59 seconds on the display
    * @return None
    */
   void showTime (uint8_t minutes, uint8_t seconds) {
      if ( minutes > 99 ) {
         minutes = 99;
      }

      if ( seconds > 59 ) {
         seconds = 59;
      }

      this->seg.displayTime(minutes, seconds, true, true); // Show colon and leading zero

      this->state = START;
   }

   /* Start the countdown functionality of the timer. It starts the given minutes and counts down.
    *  
    * @param minutes: total minutes that need to be count down.
    * @return None
    */
   void enterCountdown (uint32_t minutes) {
      this->totalMinutes = minutes;
      this->enterCountdown();
   }

   /* Start the countdown functionality of the timer. It starts with the given default minutes and counts down.
    *  
    * @param None
    * @return None
    */
   void enterCountdown () {
      this->minutes = this->totalMinutes;
      this->seconds = 0;
      this->state = COUNTDOWN;
      seg.displayTime(this->minutes, this->seconds, true, this->dash);
      this->timer = millis();
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