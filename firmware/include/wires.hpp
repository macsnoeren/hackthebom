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
 * @file       : inclue/wires.cpp
 * @author     : Maurice Snoeren (MS)
 * @description: This file implements the driver for the defusing wires. 
 * @date       : 16-06-2024
 * @version    : 1.0
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 * @todo       : 
 */
#include <driver.h>

#include <Arduino.h>
#include <math.h>

// Enumaration variable to define the order of the wires.
enum WIRE_NUMBER {
  WIRE_1,
  WIRE_2,
  WIRE_3,
  WIRE_4,
  WIRE_5
};

/* Class: Wires
 * The wires class provides high level function to control the defusing wires.
 */
class Wires: public IDriver {
private:
  uint64_t timer; // For timing purposes to become non-blocking
  uint8_t order[5]; // The array that contains the order that the wires needs to be defused
  char code[4]; // The heximal code that is extracted from the defusing order
  uint8_t wires[5]; // The wire value based on the state to see whether a wire is cut or not cut
  uint8_t orderCut[5]; // The order array that is used to follow the cutting of the wires by the user
  uint8_t totalMistakes; // Total mistakes that the user has made
  uint8_t totalWireCuts; // Total wires the user already has cut

  /* Check whether wire number is already in the order array (wire 0 -> 4)
   *  
   * @param None
   * @return True when found, otherwise false.
   */
  bool inWireOrder(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->order[i] == n ) {
        return true;
      }
    }
    return false;
  }

  /* Print on the serial the wire order that is in the order array.
   *  
   * @param None
   * @return None
   */
  void printWireOrder () {
    printf("Wire order: \n");
    for (uint8_t i=0; i < 5; i++ ) {
      printf("%d\n", this->order[i]);
    }
    printf("\n");
  }

  /* Create the heximal code based on the order array. Note the the order array needs
   * to be initialized before this method is called.
   *  
   * @param None
   * @return None
   */
  void createCode () {
    uint32_t c = 0;
    for (uint8_t i=0; i < 5; i++ ) {
      c = c + (this->order[i] << (i*3));
    }
    sprintf(this->code, "%X", c);
    printf("Code: %s\n", this->code);
  }

  /* Create a random order of the wires to defuse the (fake) bomb.
   *  
   * @param None
   * @return None
   */
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

  /* Check whether the wire that is cut is already found in the orderCut array.
   *  
   * @param None
   * @return True when found, otherwise false.
   */
  bool inWireOrderCut(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->orderCut[i] == n ) {
        return true;
      }
    }
    return false;
  }

  /* Add the wire number to the orderCut array. The function checks whether is is correct. When
   * it is not correct, then the mistakes will be increased. 
   *  
   * @param None
   * @return True correct wire is cut, otherwise false and the wrong wire has been cut.
   */
  bool addWireOrderCut(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->orderCut[i] == 0 ) { // Found empty spot, put it in
        if ( this->order[i] == n ) { // correct wire
          printf("CORRECT WIRE\n");
          this->orderCut[i] = n;
          return true;

        } else { // not correct wire
          printf("NOT CORRECT WIRE\n");
          for (uint8_t i=0; i < 5; i++ ) {
            if ( this->order[i] == n ) { // find position of cut
              this->orderCut[i] = n; // put it on the correct position.
            }
          }
          return false;
        }
      }
    }
    return false;
  }

public:
  Wires(): timer(0), totalMistakes(0), totalWireCuts(0) {
    for ( uint8_t i=0; i < 5; i++ ) { // Initialize the arrays
      this->order[i] = 0;
      this->wires[i] = 0;
      this->orderCut[i] = 0;
    }
  }

  ~Wires() {

  }

  /* The setup method initializes the task. This method should be called once at the startup of the board.
   * When the setup is successfull, the method returns 0, otherwise it returns an error number. The error numbers 
   * should be documented in the concrete task that implements this setup.
   *  
   * @param None
   * @return Zero is successfull and non-zero when an error occurred.
   */
  uint8_t setup() {
    this->createRandomWireOrder();

    // GND( D0, D5, D6, D7 ), 3V3( A0 )
    // D0 external pullup.
    pinMode(D0, INPUT_PULLUP);
    pinMode(D5, INPUT_PULLUP);
    pinMode(D6, INPUT_PULLUP);
    pinMode(D7, INPUT_PULLUP);

    Serial.println("Setup Wires Ready!");

    return 0;
  }

  /* Check whether wire number is already in the order array (wire 0 -> 4)
   *  
   * @param None
   * @return True when found, otherwise false.
   */
  bool stateWire (uint8_t n) {
    switch (n) {
      case 1:
        return analogRead(A0) > 500;
      case 2:
        return digitalRead(D0) == LOW;
        break;
      case 3:
        return digitalRead(D5) == LOW;
        break;
      case 4:
        return digitalRead(D6) == LOW;
        break;
      case 5:
        return digitalRead(D7) == LOW;
        break;
    }
    printf("Wire ERROR\n");
    return false;
  }

  /* Print the state of the wires and the values of the wires.
   *  
   * @param None
   * @return True when found, otherwise false.
   */
  void printWires () {
    printf("Wires: \n");
    for ( uint8_t i=0 ; i < 5; i++ ) {
      printf("- %d => (%d, %d)\n", i+1, this->stateWire(i+1), this->wires[i]);
    }
  }

  /* The loop method handles the main functionality. This loop method shall not contain any blocking function
   * calls. If you require delays, please use timing variable millis. Using this strategy the CPU performance is
   * maximized. If successfull the method returns 0, otherwise it returns an error number. The error numbers should be
   * documented in the concrete task that implements this loop.
   *  
   * @param None
   * @return Zero is successfull and non-zero when an error occurred.
   */
  uint8_t loop(uint64_t millis) {
    if ( this->timer == 0 ) {
      this->timer = millis;
    }

    // low-pass filter to remove high freq of button press (anti-dender), RC=20ms
    if ( (millis - this->timer) > 20 ) {
      for ( uint8_t i=0; i < 5; i++ ) {
        if ( !this->stateWire(i+1) ) {
          if ( this->wires[i] < 255 ) {
            this->wires[i] = this->wires[i] + 1;
          }
        } else {
          if ( this->wires[i] > 0 ) {
            this->wires[i] = this->wires[i] - 1;
          }
        }
      }
      this->timer = millis;
    }

    // Check real wire cutting order
    for ( uint8_t i=0; i < 5; i++ ) {
      if ( this->wires[i] > 128 ) { // Wire is cut
        if ( !this->inWireOrderCut(i+1) ) { // This is the next wire that has been cut
          if ( !this->addWireOrderCut(i+1) ) { // correct wire
            this->totalMistakes++;
          }
          this->totalWireCuts++;
          printf("Wire cut detected: %d\n", i+1);
        }
      }
    }

    // Check

    return 0;
  }

  /* Return the total wires that have been cut.
   *  
   * @param None
   * @return Total wires that have been cut.
   */
  uint8_t totalWiresCut() {
    uint8_t total = 0;
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->orderCut != 0 ) {
        total++;
      }
    }
    return total;
  }

  /* Returns the pointer to the char array containing the heximal code of the wire defusing order.
   *  
   * @param None
   * @return char* to the heximal code.
   */
  char* getCode() {
    return this->code;
  }

  /* Return whether the user has won the game. Note that false does not mean the user has lost.
   *  
   * @param None
   * @return True when won, otherwise false.
   */
  bool isWin () { // one or zero mistakes
    return (this->totalWireCuts == 5 && this->totalMistakes < 2);
  }

  /* Check whether the user has lost the game. Note that false does not mean the user has won.
   *  
   * @param None
   * @return True when lost, otherwise false.
   */
  bool isLose () { // more than one mistake
    return (this->totalMistakes > 1);
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