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
 * @date       : 27-03-2026
 * @version    : 1.1
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 *               27-03-2026 (MS): Improved code and documentation.
 * @todo       : 
 */
#include <driver.h>

#include <Arduino.h>
#include <math.h>

#include <buzzer.hpp>

// Enumaration variable to define the order of the wires.
enum WIRE_NUMBER {
  WIRE_1,
  WIRE_2,
  WIRE_3,
  WIRE_4,
  WIRE_5
};

/**
 * @class Wires
 * @brief Beheert de status en ontmantelingslogica van de fysieke draden van de bom.
 * 
 * Deze klasse implementeert de IDriver interface en handelt het inlezen van de draden,
 * de softwarematige ontstoring (debounce) en de volgorde van doorknippen af.
 */
class Wires: public IDriver {
private:
  uint64_t timer;           ///< Timer voor non-blocking updates (anti-dender).
  uint8_t order[5];         ///< De juiste volgorde waarin draden doorgeknipt moeten worden.
  char code[5];             ///< Hexadecimale code gegenereerd uit de volgorde (verhoogd naar 5 voor null-terminator).
  uint8_t wires[5];         ///< Accumulator voor ontstoring per draad.
  uint8_t orderCut[5];      ///< Bijgehouden volgorde van doorgeknipte draden.
  uint8_t totalMistakes;    ///< Aantal fouten gemaakt door de gebruiker.
  uint8_t totalWireCuts;    ///< Totaal aantal draden dat momenteel is doorgeknipt.
  Buzzer* buzzer;           ///< Referentie naar de buzzer voor feedback.

  /**
   * @brief Controleert of een draadnummer al in de gegenereerde volgorde staat.
   * @param n Het draadnummer (1-5).
   * @return True als de draad al aanwezig is, anders false.
   */
  bool inWireOrder(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->order[i] == n ) {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Print de gegenereerde draadvolgorde naar de seriële monitor voor debugging.
   */
  void printWireOrder () {
    printf("Wire order: \n");
    for (uint8_t i=0; i < 5; i++ ) {
      printf("%d\n", this->order[i]);
    }
    printf("\n");
  }

  /**
   * @brief Genereert een unieke hex-code gebaseerd op de ontmantelingsvolgorde.
   * De code wordt gebruikt in de web-interface.
   */
  void createCode () {
    uint32_t c = 0;
    for (uint8_t i=0; i < 5; i++ ) {
      c = c + (this->order[i] << (i*3));
    }
    // 15 bits max (5 * 3 bits) past in 4 hex karakters + \0. Buffer moet 5 groot zijn.
    snprintf(this->code, sizeof(this->code), "%X", c);
    printf("Code: %s\n", this->code);
  }

  /**
   * @brief Genereert een willekeurige volgorde voor de 5 draden.
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

  /**
   * @brief Controleert of een specifieke draad al als "doorgeknipt" is geregistreerd.
   * @param n Draadnummer.
   * @return True als de draad al is doorgeknipt.
   */
  bool inWireOrderCut(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->orderCut[i] == n ) {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Registreert een nieuwe draadknip en valideert of dit de juiste draad was.
   * Bij een foutieve draad wordt de ticking-snelheid van de buzzer verhoogd.
   * @param n Het nummer van de doorgeknipte draad.
   * @return True als de knip correct was volgens de volgorde.
   */
  bool addWireOrderCut(uint8_t n) {
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->orderCut[i] == 0 ) { // Found empty spot, put it in
        if ( this->order[i] == n ) { // correct wire
          printf("CORRECT WIRE\n");
          this->buzzer->beepCorrectWire();
          this->orderCut[i] = n;
          return true;

        } else { // not correct wire
          printf("NOT CORRECT WIRE\n");
          this->buzzer->beepNotCorrectWire();
          this->buzzer->startTicking(300); // double speed
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
  /**
   * @brief Constructor voor de Wires klasse.
   * @param buzzer Pointer naar de Buzzer instantie voor audio feedback.
   */
  Wires(Buzzer* buzzer): timer(0), totalMistakes(0), totalWireCuts(0), buzzer(buzzer) {
    for ( uint8_t i=0; i < 5; i++ ) { // Initialize the arrays
      this->order[i] = 0;
      this->wires[i] = 0;
      this->orderCut[i] = 0;
    }
  }

  ~Wires() {

  }

  /**
   * @brief Initialiseert de hardware pinnen en genereert de random volgorde.
   * Wordt aangeroepen tijdens boot of reset van het spel.
   * @return 0 bij succes.
   */
  uint8_t setup() {
    this->timer = 0;
    this->totalMistakes = 0;
    this->totalWireCuts = 0;
    for ( uint8_t i=0; i < 5; i++ ) { // Initialize the arrays
      this->order[i] = 0;
      this->wires[i] = 0;
      this->orderCut[i] = 0;
    }

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

  /**
   * @brief Leest de actuele elektrische status van een specifieke draad.
   * @param n Draadnummer (1-5).
   * @return True als de draad verbonden is, false als deze onderbroken is.
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

  /**
   * @brief Debug functie om de status van alle draden naar de seriële poort te schrijven.
   */
  void printWires () {
    printf("Wires: \n");
    for ( uint8_t i=0 ; i < 5; i++ ) {
      printf("- %d => (%d, %d)\n", i+1, this->stateWire(i+1), this->wires[i]);
    }
  }

  /**
   * @brief Hoofd-loop voor de draden. Verzorgt softwarematige ontstoring en checkt op nieuwe onderbrekingen.
   * Een draad wordt als "doorgeknipt" beschouwd als de accumulator boven de 128 uitkomt.
   * 
   * @param millis De huidige systeem-tijd in milliseconden.
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

  /**
   * @brief Geeft het totaal aantal correct of foutief doorgeknipte draden terug.
   * @return Aantal doorgeknipte draden.
   */
  uint8_t totalWiresCut() {
    uint8_t total = 0;
    for (uint8_t i=0; i < 5; i++ ) {
      if ( this->orderCut[i] != 0 ) {
        total++;
      }
    }
    return total;
  }

  /**
   * @brief Geeft de gegenereerde deactivatiecode terug.
   * @return Pointer naar de hex-string.
   */
  char* getCode() {
    return this->code;
  }

  /**
   * @brief Controleert of de gebruiker heeft gewonnen (alle draden door met max 1 fout).
   * @return True als gewonnen.
   */
  bool isWin () { // one or zero mistakes
    return (this->totalWireCuts == 5 && this->totalMistakes < 2);
  }

  /**
   * @brief Controleert of de gebruiker heeft verloren (te veel fouten).
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