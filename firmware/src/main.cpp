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
 * @date       : 27-03-2026
 * @version    : 1.2
 * @updates    : 20-02-2024 (MS): Initial code.
 *               16-06-2024 (MS): Created the first release version.
 *               22-09-2024 (MS): Added Game 2 for the first year students including game selection.
 *               27-03-2026 (MS): Improved code and documentation.
 * @todo       : 
 */
#include <Arduino.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include <driver.h>
#include <timer.hpp>
#include <buzzer.hpp>
#include <button.hpp>
#include <wires.hpp>
#include <game_01.hpp>
#include <game_02.hpp>

/// @brief Unieke SSID gebaseerd op de Chip ID.
String SSID = "HTB-" + String(system_get_chip_id());

/// @brief Wi-Fi wachtwoord (wordt geladen uit EEPROM of random gegenereerd).
String PASSWORD = "";

int channel = random(1, 13);
uint8_t GAME_SELECTION = 0;

// Forward declaration of the different routes for the webpages.
void handleRoot();
void handleAdmin();
void handleCode();
void handleStatus();
void handleNotFound();

// Instantieer hardware drivers.
Timer timer;
Buzzer buzzer;
Button button;
Wires wires(&buzzer);
Game01 game01(&timer, &buzzer, &wires);
Game02 game02(&timer, &buzzer);

IDriver *drivers[] = { (IDriver*) &timer,
                       (IDriver*) &buzzer,
                       (IDriver*) &button,
                       (IDriver*) &wires,
                     };

IDriver *games[] = { (IDriver*) &game01,
                     (IDriver*) &game02,
                   };

ESP8266WebServer server(80);

/**
 * @enum MAIN_STATES
 * @brief De staten van de hoofd-state machine van het spel.
 */
enum MAIN_STATES {
  SELECT_GAME, ///< De leraar kiest het speltype.
  SELECT_TIME, ///< De leraar stelt de beschikbare tijd in.
  READY,       ///< Systeem is klaar voor de start (wacht op knopdruk).
  GAME_1,      ///< Game 1 (Draden) is actief.
  GAME_2,      ///< Game 2 (Web hacking) is actief.
  WIN,         ///< De bom is succesvol ontmanteld.
  LOSE,        ///< De bom is afgegaan.
  END,         ///< Einde van het spel, wacht op herstart.
};

// FSM state variable that indicate in which state we are in.
MAIN_STATES stateMain = SELECT_GAME;

// Total time that students get as default value in minutes (50 minutes).
uint8_t totalTimeDefault = 50;

/**
 * @brief Arduino Setup functie.
 * Initialiseert Seriële poort, EEPROM (voor wachtwoord), drivers en de webserver.
 */
void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting HackTheBom Firmware...");

  // Start het bestandssysteem
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }

  /* Create a random number that will be used as Wi-Fi password. This number
   * will be stored in the EEPROM of the ESP8622.
   */ 
  EEPROM.begin(512);
  if ( EEPROM.read(0) != 0x55 ) { 
    // Geen wachtwoord gevonden, genereer een nieuwe van 20 cijfers.
    PASSWORD.reserve(20); 
    for ( uint8_t i=0; i < 20; i++ ) { // Create a random password
      uint8_t r = random(0, 10);
      PASSWORD += String(r);
      EEPROM.write(i+1, r); // Store it in the EEPROM
    }
    EEPROM.write(0, 0x55); // Everything is stored, so set the first byte to 0x55
    EEPROM.commit();

  } else {
    PASSWORD.reserve(20);
    for ( uint8_t i=0; i < 20; i++ ) {
      PASSWORD += String(EEPROM.read(i+1));
    }
  }

  // Start alle hardware drivers.
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
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();

  // First state is blinking and show the default time.
  timer.blink(true);
  timer.showGameSelection(GAME_SELECTION+1);

  // Test the buzzer!
  buzzer.beep();

  // Setup is ready and print it to the serial.
  Serial.println("Ready.");
}

/**
 * @brief Arduino Loop functie.
 * Verwerkt driver updates en beheert de hoofd-state machine (FSM) zonder blocking code.
 */
void loop() {
  for ( IDriver *driver: drivers ) { // Call the loop functions of the drivers.
      driver->loop(millis());
  }
  
  // Implementation of the FSM by using a switch statement.
  switch (stateMain) {
    case SELECT_GAME:
      if ( button.isPressed() ) { // Wissel tussen Game 1 en Game 2
        GAME_SELECTION = (GAME_SELECTION+1) % 2;
        timer.showGameSelection(GAME_SELECTION+1);
        printf("GAME: %d\n", GAME_SELECTION);
      }

      if ( button.isLongPressed() ) { // Bevestig game keuze
        timer.showTime(totalTimeDefault, 0);
        stateMain = SELECT_TIME;
        buzzer.beep();
      }
    break;

    case SELECT_TIME:
      if ( button.isPressed() ) { // Verhoog tijd in stappen van 5 minuten
        totalTimeDefault = (totalTimeDefault + 5) % 100;
        timer.showTime(totalTimeDefault, 0);
      }

      if ( button.isLongPressed() ) { // Bevestig tijd
        timer.showTime(totalTimeDefault, 0);
        stateMain = READY;
        timer.blink(false);
        buzzer.beep();
      }
    break;

    case READY:
      if ( button.isPressed() ) { // START HET SPEL
        if ( GAME_SELECTION == 0 ) { 
          stateMain = GAME_1; 
          game01.start(totalTimeDefault);
        }
        if ( GAME_SELECTION == 1 ) { 
          WiFi.softAP(SSID, "h4cKTh!5", channel, 0, 1); // Reset it to a fixed password for game 2 wi-fi
          stateMain = GAME_2;
          game02.start(totalTimeDefault);
        }
        // Print the name of the device and the password.
        printf("%s / %s\n", SSID.c_str(), PASSWORD.c_str());
        printf("GAME: %d\n", GAME_SELECTION+1);
      }
    break;

    case GAME_1: // Draden spel (Minor Game)
     if ( game01.isWin() ) {
        stateMain = WIN;
        buzzer.startWin();
     }
     if ( game01.isLose() ) { // Tijd op of te veel fouten
        stateMain = LOSE;
        buzzer.startLose();
     }
    break;

    case GAME_2: // Introduction, year 1 Game
    if ( game02.isWin() ) {
        stateMain = WIN;
        buzzer.startWin();
    }
    if ( game02.isLose() ) {
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
      static uint32_t end_timer = millis();
      if ( end_timer != 0 && millis() - end_timer > 20000 ) {
        buzzer.mute();
        end_timer = 0;
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
  char filename[30];
  sprintf(filename, "/game%02d.html", GAME_SELECTION + 1);

  File file = LittleFS.open(filename, "r");  
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

/**
 * Handles the admin webpage http://<ipaddress>/admin.
 *
 * @param None
 * @return None
 */
void handleAdmin() {
  char filename[30];
  sprintf(filename, "/game%02d-admin.html", GAME_SELECTION + 1);

  File file = LittleFS.open(filename, "r");  
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  } else {
    handleNotFound();
  }
}

/**
 * Handles the code webpage http://<ipaddress>/code.
 *
 * @param None
 * @return None
 */
void handleCode() {
  char filename[30];
  sprintf(filename, "/game%02d-code.html", GAME_SELECTION + 1);

  File file = LittleFS.open(filename, "r");  
  if (file) {
    String content = file.readString();
    file.close();
    
    // Vervang de placeholders door de actuele waarden
    content.replace("{SSID}", SSID);
    content.replace("{CODE}", String(wires.getCode()));

    server.send(200, "text/html", content);
  } else {
    handleNotFound();
  }
}

/**
 * Handles the status request to sync time and game state.
 */
void handleStatus() {
  bool win = (GAME_SELECTION == 0) ? game01.isWin() : game02.isWin();
  bool lose = (GAME_SELECTION == 0) ? game01.isLose() : game02.isLose();
  
  // Zorg dat je Timer klasse de methode getRemainingSeconds() implementeert
  int remaining = timer.getRemainingSeconds();

  String json = "{";
  json += "\"time\":" + String(remaining) + ",";
  json += "\"win\":" + String(win ? "true" : "false") + ",";
  json += "\"lose\":" + String(lose ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
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
