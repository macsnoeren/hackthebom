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

// Buzzer functionality selection
enum BuzzerFunctions {
   BUZZER_MUTE,
   BUZZER_TICK_A,
   BUZZER_TICK_B,
   BUZZER_LOSE,
   BUZZER_WIN,
};

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

#define TOTAL_NOTES 57

/* Class: Buzzer
 * The buzzer class provides high level function to control the sound.
 */
class Buzzer: public IDriver {
private:
   uint64_t timer; // Used for timing purposes to become non-blocking
   uint32_t  value;
   uint32_t notes[TOTAL_NOTES] = {E*2, E*2, B, C*2, D*2, D*2, C*2, B, A, A, A, C*2, E*2, E*2,
                                  D*2, C*2, B, B, B, C*2, D*2, D*2, E*2, E*2, C*2, C*2, A, A, A,
                                  D*2, D*2, F*2, A*2, A*2, G*2, F*2, E*2, E*2, E*2, C*2, E*2, E*2, 
                                  D*2, C*2, B, B, B, C*2, D*2, D*2, E*2, E*2, C*2, C*2, A, A, A};
   BuzzerFunctions bf;

public:
    Buzzer(): timer(0), value(0), bf(BUZZER_MUTE) {

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
      switch (this->bf) {
         case BUZZER_TICK_A:
            if ( this->timer == 0 ) {
               this->timer = millis;
            } else {
               if ( millis - this->timer > 600 ) {
                  this->beep(100);
                  this->timer = millis;
                  this->bf = BUZZER_TICK_B;
               }
            }
            break;

         case BUZZER_TICK_B:
            if ( this->timer == 0 ) {
               this->timer = millis;
            } else {
               if ( millis - this->timer > 600 ) {
                  this->beep(200);
                  this->timer = millis;
                  this->bf = BUZZER_TICK_A;
               }
            }
            break;

         case BUZZER_LOSE:
            if ( this->timer == 0 ) {
               this->timer = millis;
            } else {
               if ( millis - this->timer > 0 ) {
                  this->beep(C);
                  this->beep(D);
                  this->beep(E);
                  this->beep(F);
                  this->beep(G);
                  this->beep(A);
                  this->beep(B);
                  this->timer = millis;
               }
            }
            break;

         case BUZZER_WIN:
            if ( this->timer == 0 ) {
               this->timer = millis;
            } else {
               if ( millis - this->timer > 200 ) {
                  if ( value >= TOTAL_NOTES ) {
                     value = 0;
                  }
                  on(this->notes[value++]);
                  this->timer = millis;
               }
            }
            break;

         case BUZZER_MUTE:
         default:
            this->off();
      }

      return 0;
   }

   /* Start the ticking bomb sound.
    *  
    * @param None
    * @return None
    */
   void startTicking () {
      this->off();
      this->bf = BUZZER_TICK_A;
   }

   /* Mute the buzzer
    *  
    * @param None
    * @return None
    */
   void mute () {
      this->off();
      this->bf = BUZZER_MUTE;
   }

   /* Start the win music.
    *  
    * @param None
    * @return None
    */
   void startWin () {
      this->off();
      this->bf = BUZZER_WIN;
   }

   /* Start the lose music.
    *  
    * @param None
    * @return None
    */
   void startLose() {
      this->off();
      this->bf = BUZZER_LOSE;
   }

   /* Make sound with the given frequency (default 1kHz).
    *  
    * @param f: frequency between 100 - 40000 Hz
    * @return None
    */
   void on(uint32_t f = 1000) {
      analogWriteFreq(f);
      analogWrite(D8, 128);
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
   void beep(uint32_t f = 1000, unsigned long d = 5) {
      this->on(f);
      delay(d);
      this->off();
      delay(d);
   }

   /* Make a beep sound for correct wire, which is blocking for 100 ms on the given frequency.
    *  
    * @param None
    * @return None
    */
   void beepCorrectWire() {
      this->beep(4000, 100);
   }

   /* Make a beep sound for incorrect, which is blocking for 100 ms on the given frequency.
    *  
    * @param None
    * @return None
    */
   void beepNotCorrectWire() {
      this->beep(500, 100);
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