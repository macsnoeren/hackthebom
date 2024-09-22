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
 * @file       : main.cpp
 * @author     : Maurice Snoeren (MS)
 * @description: Main of the HackTheBom firmware application. HackTheBom is an
 *               escape room game that is used within Avans Hogeschool to teach
 *               students cyber security skills and knowledge. When the firmware
 *               starts, the teacher is able to select first the game that will
 *               be played. Enter the game is done using a long button press. 
 *               Then the time that the students get is selected and again with a
 *               long button press it is selected. To start the game the button
 *               is pressed again. The timer will start and the students need to
 *               solve the puzzle to defuse the (fake) bomb.
 *               The hardware:
 *               - Passive buzzer connected to D8 and GND.
 *               - 7-segment display connected to SCL (D1), SDA (D2), 5V and GND.
 *               - Button connected to D3.
 *               - 5 defuse wires connected to 1 (A0), 2 (D0), 3 (D5), 4 (D6), 5 (D7)
 *                 Wire connections: - A0 --- 3V3
 *                                   - D0 --- GND (external pull-up 100K to 3V3)
 *                                   - D5/D6/D7 --- GND (select internal pull-up)
 * @date       : 22-09-2024
 * @version    : 1.1
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 *               22-09-2024 (MS): Added Game 2 for the first year students including game selection.
 * @todo       : 
 */
#include <Arduino.h>
#include <String.h>
#include <math.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <driver.h>
#include <website.hpp>
#include <timer.hpp>
#include <buzzer.hpp>
#include <button.hpp>
#include <wires.hpp>

// Name of the device based on the ESP8622 chip ID used as SSID.
String SSID = "HTB-" + String(system_get_chip_id());

// Wi-Fi AP password and will be set later.
String PASSWORD = "";

int channel = random(1, 13);

uint8_t GAME_SELECTION = 0;

// Forward declaration of the different routes for the webpages.
void handleRoot();
void handleAdmin();
void handleCode();
void handleNotFound();
String webDefusingCode = "";
uint8_t webDefusingCodeTrials = 0;

// Instantiate the used drivers that handle the hardware appropiately.
Timer timer;
Buzzer buzzer;
Button button;
Wires wires(&buzzer);
IDriver *drivers[] = { (IDriver*) &timer,
                       (IDriver*) &buzzer,
                       (IDriver*) &button,
                       (IDriver*) &wires,
                     };

// Instantiate the webserver to provide webpages to the user.
ESP8266WebServer server(80);

// Implementation of a very simple finite state machine (FSM) using an enumaration type.
enum MAIN_STATES {
  SELECT_GAME, // Game selection state
  SELECT_TIME, // Time selection state
  READY,       // Ready to start, game and time selected
  GAME_1,        // Game is started
  GAME_2,        // Game is started
  WIN,         // The user won!
  LOSE,        // The user lost!
  END,         // End
};

// FSM state variable that indicate in which state we are in.
MAIN_STATES stateMain = SELECT_GAME;

// Total time that students get as default value in minutes (50 minutes).
uint8_t totalTimeDefault = 50;

/**
 * Setup that is used to execute the setup of the hardware and software before
 * the loop starts.
 *
 * @param None
 * @return None
 */
void setup() {
  Serial.begin(115200); // Setup the serial connection for debugging

  /* Create a random number that will be used as Wi-Fi password. This number
   * will be stored in the EEPROM of the ESP8622.
   */ 
  EEPROM.begin(512);
  if ( EEPROM.read(0) != 0x55 ) { // If the first byte is equal to 0x55, there is a number present.

    for ( uint8_t i=0; i < 20; i++ ) { // Create a random password
      uint8_t r = random(0, 10);
      PASSWORD = PASSWORD + String(r);
      EEPROM.write(i+1, r); // Store it in the EEPROM
    }
    EEPROM.write(0, 0x55); // Everything is stored, so set the first byte to 0x55
    EEPROM.commit();

  } else {
    for ( uint8_t i=0; i < 20; i++ ) {
      PASSWORD = PASSWORD + String(EEPROM.read(i+1)); // Read the value from the EEPROM
    }
  }

  // Setup the drivers that are active.
  for ( IDriver *driver: drivers ) {
      driver->setup();
  }

  // Print the name of the device and the password.
  printf("%s / %s\n", SSID.c_str(), PASSWORD.c_str());

  // Setup the Wi-Fi Access Point (AP)
  WiFi.softAP(SSID, PASSWORD, channel, 0, 1); // one client allowed!
  Serial.printf("AP IP address (channel %d): %s\n", channel, WiFi.softAPIP().toString().c_str());

  // Setup the routes to the webpages
  server.on("/", handleRoot);
  server.on("/admin", handleAdmin);
  server.on("/code", handleCode);
  server.onNotFound(handleNotFound);
  server.begin();

  // First state is blinking and show the default time.
  timer.blink(true);
  timer.showGameSelection(GAME_SELECTION+1);
//  timer.showTime(totalTimeDefault, 0);

  // Test the buzzer!
  buzzer.beep();

  // Setup is ready and print it to the serial.
  Serial.println("Ready.");
}

/**
 * The loop is called continously. Make sure no blocking functions are implemented
 * by the different modules, drivers and other code.
 *
 * @param None
 * @return None
 */
void loop() {
  for ( IDriver *driver: drivers ) { // Call the loop functions of the drivers.
      driver->loop(millis());
  }
  
  // Implementation of the FSM by using a switch statement.
  switch (stateMain) {
    case SELECT_GAME:
      if ( button.isPressed() ) {
        GAME_SELECTION = (GAME_SELECTION+1) % 2;
        timer.showGameSelection(GAME_SELECTION+1);
        printf("GAME: %d\n", GAME_SELECTION);
      }

      if ( button.isLongPressed() ) {
        GAME_SELECTION = GAME_SELECTION-1; // Always one back due to button press
        GAME_SELECTION = GAME_SELECTION % 2;
        timer.showTime(totalTimeDefault, 0);
        stateMain = SELECT_TIME;
        buzzer.beep();
      }
    break;

    case SELECT_TIME:
      if ( button.isPressed() ) {
        totalTimeDefault = (totalTimeDefault + 5) % 100;
        timer.showTime(totalTimeDefault, 0);
      }

      if ( button.isLongPressed() ) {
        totalTimeDefault = (totalTimeDefault - 5) % 100; // Always an extra press - simple solution to fix.
        timer.showTime(totalTimeDefault, 0);
        stateMain = READY;
        timer.blink(false);
        buzzer.beep();
      }
    break;

    case READY:
      if ( button.isPressed() ) { 
        stateMain = GAME_1; // Default
        timer.enterCountdown(totalTimeDefault);
        buzzer.startTicking();
        wires.setup(); // Reset the game

        // Print the name of the device and the password.
        printf("%s / %s\n", SSID.c_str(), PASSWORD.c_str());
        printf("GAME: %d\n", GAME_SELECTION+1);

        if ( GAME_SELECTION == 0 ) { stateMain = GAME_1; }
        if ( GAME_SELECTION == 1 ) { 
          WiFi.softAP(SSID, "h4cKTh!5", channel, 0, 1); // Reset it to a fixed password for game 2 wi-fi
          stateMain = GAME_2;
        }
      }
    break;

    case GAME_1: // Minor Game with wires
     if ( wires.isWin() ) {
      stateMain = WIN;
      buzzer.startWin();
     }
    if ( timer.isTimerZero() || wires.isLose() ) {
      stateMain = LOSE;
      buzzer.startLose();
     }
    break;

    case GAME_2: // Introduction, year 1 Game
    if ( webDefusingCode.equals("BC84") ) { // Hardcoded!
      stateMain = WIN;
      buzzer.startWin();
    }

    if ( timer.isTimerZero() || webDefusingCodeTrials > 3) {
      stateMain = LOSE;
      buzzer.startLose();
     }
    break;

    case WIN:
      timer.showYeah();
      stateMain = END;
    break;

    case LOSE:
      timer.showLose();
      stateMain = END;
    break;

    case END:
      static uint32_t timer = millis();
      if ( timer != 0 && millis() - timer > 20000 ) {
        buzzer.mute();
        timer = 0;
      }
      // Do nothing, so the device needs to be restarted.
    break;

    default:
      stateMain = SELECT_TIME;
  };

  // Call the loop function of the server to handle clients that are connected.
  server.handleClient();
}

/**
 * Handles the root webpage http://<ipaddress>/.
 *
 * @param None
 * @return None
 */
void handleRoot() {
  if ( GAME_SELECTION == 1 ) {
    server.send(200, "text/html", index_html_2);
    webDefusingCode = server.arg("code");
    if ( !webDefusingCode.equals("") ) {
      webDefusingCodeTrials++;
      if ( webDefusingCodeTrials > 0 ) {
        buzzer.startTicking(300); // double speed
      }
    }
    printf("webDefusingCode: %s\n", webDefusingCode.c_str());

  } else { // Default Game 1
    server.send(200, "text/html", index_html);
  }
}

/**
 * Handles the admin webpage http://<ipaddress>/admin.
 *
 * @param None
 * @return None
 */
void handleAdmin() {
  server.send(200, "text/html", admin_html);
}

/**
 * Handles the code webpage http://<ipaddress>/code.
 *
 * @param None
 * @return None
 */
void handleCode() {
  char html[1000];
  sprintf(html, code_html, SSID.c_str(), wires.getCode());
  server.send(200, "text/html", html);
}

/**
 * Handles when a route does not exist.
 *
 * @param None
 * @return None
 */
void handleNotFound () {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/* END OF FILE */
