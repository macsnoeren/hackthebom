#pragma once

#include <driver.h>

#include <HT16K33.h>

enum TimerState {
   START,
   CONFIG,
   COUNTDOWN,
   FINISH,
};

class Timer: public IDriver {
private:
   uint64_t timer;
   uint8_t state;
   HT16K33 seg;
   uint32_t totalMinutes;
   uint32_t minutes;
   uint32_t seconds;
   bool dash;
   

public:
    Timer(): timer(0), state(START), seg(0x70), totalMinutes(30), minutes(0), seconds(0), dash(true) {

    }

    ~Timer() {

    }

    /* The abstract setup method initializes the task. This method should be called once at the startup of the board.
       When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
       should be documented in the concrete task that implements this setup.
    */
    uint8_t setup() {
      Wire.begin();
      Wire.setClock(100000);
   
      seg.begin();
      seg.displayOn();
      seg.setBrightness(4);
      seg.setDigits(4);

      this->showDashes();

      Serial.println("Setup Timer Ready!");

      return 0;
    }

    /* The abstract loop method handles the main functionality. This loop method shall not contain any blocking function
       calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
       maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
       documented in the concrete task that implements this loop.
    */
    uint8_t loop(uint64_t millis) {

      if ( this->state == COUNTDOWN ) {
         if ( millis - this->timer > 1000 ) {
            if (this->seconds == 0 ) {
               if ( this->minutes == 0 ) {
                  enterFinish();
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

      if ( this->state == FINISH ) {
         showLose();
      }
      
      return 0;
    }

   void showLose() {
      uint8_t x[4] = { SEG_D | SEG_E | SEG_F,
                       SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
                       SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
                       SEG_A | SEG_D | SEG_E | SEG_F | SEG_G };
      seg.displayRaw(x, false);
   }

   void showYeah() {
      uint8_t x[4] = { SEG_B | SEG_C | SEG_D | SEG_F| SEG_G,
                       SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
                       SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
                       SEG_B | SEG_C | SEG_E | SEG_F | SEG_G };
      seg.displayRaw(x, false);
   }

   void showDashes() {
      uint8_t x[4] = { SEG_G, SEG_G, SEG_G, SEG_G };
      seg.displayRaw(x, true);
   }

   void enterConfig () {
      this->seg.setBlink(2);
      this->state = CONFIG;
   }

   void enterCountdown () {
      this->minutes = totalMinutes;
      this->seconds = 0;
      this->state = COUNTDOWN;
      seg.displayTime(this->minutes, this->seconds, true, this->dash);
      this->timer = millis();
   }

   void enterFinish () {
      this->state = FINISH;
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