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
 * @description: This file implements the logic for Game 02 (Web Hacking). 
 * @date       : 27-03-2026
 * @version    : 1.1
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 *               27-03-2026 (MS): Improved code and documentation.
 * @todo       : 
 */
#include <driver.h>
#include <Arduino.h>
#include <timer.hpp>
#include <buzzer.hpp>

class Game02: public IDriver {
private:
  Timer*  _timer;
  Buzzer* _buzzer;
  bool    _active;
  bool    _win;
  bool    _lose;
  String  _webDefusingCode;
  uint8_t _webDefusingCodeTrials;

public:
  Game02(Timer* timer, Buzzer* buzzer) : 
    _timer(timer), _buzzer(buzzer), _active(false), _win(false), _lose(false), 
    _webDefusingCode(""), _webDefusingCodeTrials(0) {
  }

  ~Game02() {
  }

  uint8_t setup() {
    return 0;
  }

  /**
   * @brief Start het web hacking spel.
   * @param minutes De beschikbare tijd in minuten.
   */
  void start(uint8_t minutes) {
    this->_timer->enterCountdown(minutes);
    this->_active = true;
    this->_win    = false;
    this->_lose   = false;
    this->_webDefusingCode = "";
    this->_webDefusingCodeTrials = 0;
  }

  /**
   * @brief Verwerkt de ingevoerde code vanuit de webinterface.
   * @param code De ontvangen code string.
   */
  void handleWebCode(String code) {
    if (!this->_active || code.equals("")) return;

    this->_webDefusingCode = code;
    this->_webDefusingCodeTrials++;
    this->_buzzer->startTicking(300); // Versnel de buzzer bij elke poging
  }

  uint8_t loop(uint64_t millis) {
    if (!this->_active) return 0;

    if (this->_webDefusingCode.equals("BC84")) {
      this->_win    = true;
      this->_active = false;
    }

    if (this->_timer->isTimerZero() || this->_webDefusingCodeTrials > 3) {
      this->_lose   = true;
      this->_active = false;
    }

    return 0;
  }

  bool isWin()  { return this->_win; }
  bool isLose() { return this->_lose; }

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