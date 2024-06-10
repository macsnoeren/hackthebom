// Start up: Select program (short selection, long enter), select timer (short selection, long enter), (short) start assignment
// Buzzer +: D8 -: GND
// Timer: VCC: 5V, GND: GND, SCL: D1, SDA: D2
// SWITCH: +: 3v3, -: D0
// 

#include <Arduino.h>
#include <String.h>
#include <math.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <driver.h>
#include <timer.hpp>
#include <buzzer.hpp>
#include <button.hpp>
#include <wires.hpp>

#define STRING_LEN 128
#define NUMBER_LEN 32

String SSID = "HTB-" + String(system_get_chip_id());
String PASSWORD;

void handleRoot();
void handleAdmin();
void handleNotFound();

Timer timer;
Buzzer buzzer;
Button button;
Wires wires;
IDriver *drivers[] = { (IDriver*) &timer,
                       (IDriver*) &buzzer,
                       (IDriver*) &button,
                       (IDriver*) &wires,
                     };

ESP8266WebServer server(80);

enum MAIN_STATES {
  SELECT_TIME,
  READY,
  GAME,
  WIN,
  LOSE,
  END,
};
MAIN_STATES stateMain = SELECT_TIME;
uint8_t totalTime = 30;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>HTB</title>
 </head>
 <body>
  <H1>Hack The Bom</H1>
 </body>
</html>)rawliteral";

const char admin_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>HTB</title>
 </head>
 <body>
  <H1>Admin Hack The Bom</H1>
  <H2>Code: %s</H2>
 </body>
</html>)rawliteral";

void setup() {
  Serial.begin(9600);

  EEPROM.begin(512);
  uint8_t es = EEPROM.read(0);
  printf("EEPROM(0): '%u'\n", es);
  if ( es != 0x55 ) {
    PASSWORD = "";
    EEPROM.write(0, 0x55);
    for ( uint8_t i=0; i < 20; i++ ) {
      uint8_t r = random(0, 10);
      PASSWORD = PASSWORD + String(r);
      printf("%d\n", r);
      EEPROM.write(i+1, r);
    }
    EEPROM.commit();
  } else {
    PASSWORD = "";
    for ( uint8_t i=0; i < 20; i++ ) {
      PASSWORD = PASSWORD + String(EEPROM.read(i+1));
    }
  }

  // Call the setup to initialize the main drivers.
  for ( IDriver *driver: drivers ) {
      driver->setup();
  }

  printf("%s / %s\n", SSID.c_str(), PASSWORD.c_str());

  WiFi.softAP(SSID, PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/admin", handleAdmin);
  server.onNotFound(handleNotFound);
  server.begin();

  timer.blink(true);
  timer.showTime(totalTime, 0);

  Serial.println("Ready.");
}

void loop() {
  for ( IDriver *driver: drivers ) {
      driver->loop(millis());
  }
  
  switch (stateMain) {
    case SELECT_TIME:
      if ( button.isPressed() ) {
        totalTime = (totalTime + 5) % 100;
        timer.showTime(totalTime, 0);
        delay(500);
      }
      if ( button.isLongPressed() ) {
        stateMain = READY;
        timer.blink(false);
        delay(1000);
        button.isPressed(); // reset
      }
    break;

    case READY:
      if ( button.isPressed() ) { 
        stateMain = GAME;
        timer.enterCountdown(totalTime);
      }
    break;

    case GAME:
     if ( wires.isWin() ) {
      stateMain = WIN;
     }
    if ( wires.isLose() ) {
      stateMain = LOSE;
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
    break;

    default:
      stateMain = SELECT_TIME;
  };

  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleAdmin() {
  char html[1000];
  sprintf(html, admin_html, wires.getCode());
  server.send(200, "text/html", html);
  timer.enterCountdown();
}

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
