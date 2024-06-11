#pragma once

#include <driver.h>

#include <Arduino.h>
#include <math.h>

enum WIRE_NUMBER {
  WIRE_1,
  WIRE_2,
  WIRE_3,
  WIRE_4,
  WIRE_5
};

class Wires: public IDriver {
private:
   uint64_t timer;
   uint8_t state;
   uint8_t order[5];
   char code[4];
   uint8_t totalMistakes;

  void resetWireOrder() {
    for (uint8_t i=0; i < 5; i++ ) {
      this->order[i] = 0;
    }
  }

  bool inWireOrder(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->order[i] == n ) {
        return true;
      }
    }
    return false;
  }

  void printWireOrder () {
    printf("Wire order: \n");
    for (uint8_t i=0; i < 5; i++ ) {
      printf("%d\n", this->order[i]);
    }
    printf("\n");
  }

  void createCode () {
    uint32_t c = 0;
    for (uint8_t i=0; i < 5; i++ ) {
      c = c + (this->order[i] << (i*3));
    }
    sprintf(this->code, "%X", c);
    printf("Code: %s\n", this->code);
  }

  void createRandomWireOrder() {
    //printf("Create random order\n");
    for (uint8_t i=0; i < 5; i++ ) {
      uint8_t n = (uint8_t) random(1, 6);
      while ( this->inWireOrder(n) ) {
        n = (uint8_t) random(1, 6);
      }
      //printf("Found number: %d\n", n);
      this->order[i] = n;
    }
    this->createCode();
  }

public:
    Wires(): timer(0), state(0), totalMistakes(0) {

    }

    ~Wires() {

    }

    /* The abstract setup method initializes the task. This method should be called once at the startup of the board.
       When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
       should be documented in the concrete task that implements this setup.
    */
    uint8_t setup() {
      this->createRandomWireOrder();

      // GND( D0, D5, D6, D7 ), 3V3( D8 )
      // D0 external pullup.
      pinMode(D0, INPUT_PULLUP);
      pinMode(D5, INPUT_PULLUP);
      pinMode(D6, INPUT_PULLUP);
      pinMode(D7, INPUT_PULLUP);
      pinMode(D8, INPUT); // outside pulldown

      Serial.println("Setup Wires Ready!");

      return 0;
    }

    bool stateWire (uint8_t n) {
      switch (n) {
        case 1:
          return digitalRead(D0) == LOW;
          break;
        case 2:
          return digitalRead(D5) == LOW;
          break;
        case 3:
          return digitalRead(D6) == LOW;
          break;
        case 4:
          return digitalRead(D7) == LOW;
          break;
        case 5:
          return digitalRead(D8) == HIGH;
          break;
      }
      printf("Wire ERROR\n");
      return false;
    }

    void printWires () {
      printf("Wires: \n");
      for ( uint8_t i=0 ; i < 5; i++ ) {
        printf("- %d => %d\n", i+1, this->stateWire(i+1));
      }
    }

    /* The abstract loop method handles the main functionality. This loop method shall not contain any blocking function
       calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
       maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
       documented in the concrete task that implements this loop.
    */
    uint8_t loop(uint64_t millis) {
      if ( this->timer == 0 ) {
        this->timer = millis;
      }

      if ( millis - this->timer > 1000 ) {
        this->printWires();
        this->timer = millis;
      }

      return 0;
    }

    bool pressed() {
      return digitalRead(D3) == LOW;
    }

    char* getCode() {
      return this->code;
    }

    bool isWin () {
      return false;
    }

    bool isLose () {
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