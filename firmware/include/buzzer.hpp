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
 * @file       : inclue/buzzer.cpp
 * @author     : Maurice Snoeren (MS)
 * @description: This file implements the driver for the buzzer. 
 * @date       : 16-06-2024
 * @version    : 1.0
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 * @todo       : - Write a music engine to play music
 */
#include <driver.h>
#include <Arduino.h>

// Enumeration for notes to use music
enum Notes : uint32_t { // Octave 3 - https://forum.professionalcomposers.com/t/note-frequency-chart-free-guide/506
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

/* Class: Buzzer
 * The buzzer class provides high level function to control the sound.
 */
class Buzzer: public IDriver {
private:
   uint64_t timer; // Used for timing purposes to become non-blocking
   uint32_t  value; // 100 -> 40.000
   uint32_t notes[8] = {C, D, E, F, G, A, B, C*2};

public:
    Buzzer(): timer(0), value(100) {

    }

    ~Buzzer() {

    }

   /* The setup method initializes the task. This method should be called once at the startup of the board.
    * When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
    * should be documented in the concrete task that implements this setup.
    *  
    * @param None
    * @return Zero is successfull and non-zero when an error occurred.
    */
   uint8_t setup() {
      pinMode(D8, OUTPUT);
      digitalWrite(D8, HIGH);

      Serial.println("Setup Buzzer Ready!");

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

   /* Make sound with the given frequency (default 1kHz).
    *  
    * @param f: frequency between 100 - 40000 Hz
    * @return None
    */
   void on(uint32_t f = 1000) {
      analogWriteFreq(f);
      analogWrite(D8, 1);
   }

   /* Disable the sound (mute).
    *  
    * @param None
    * @return None
    */   
   void off() {
      analogWrite(D8, 0);
   }

   /* Make a beep sound, which is blocking for 10 ms on the given frequency.
    *  
    * @param f: frequency between 100 - 40000 Hz
    * @return None
    */
   void beep(uint32_t f = 1000) {
      this->on(f);
      delay(5);
      this->off();
      delay(5);
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