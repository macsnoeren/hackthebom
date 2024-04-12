#include <Arduino.h>
#include <String.h>

#include <IotWebConf.h>
#include <IotWebConfOptionalGroup.h>

#define STRING_LEN 128
#define NUMBER_LEN 32

String name = "HackTheBom-" + String(system_get_chip_id());
String wifiInitialApPassword = "HackTheBom";

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(name.c_str(), &dnsServer, &server, wifiInitialApPassword.c_str(), "v1");

void configSaved();
void handleRoot();
void handleBomPage();

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Starting up...");
  Serial.println("name: " + name);

  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/bom", handleBomPage);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  Serial.println("Ready.");
}

void loop() {
  iotWebConf.doLoop();
}

void handleBomPage() {
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>Britt</title></head><body><h1>Britt, Hi!</h1></body></html>\n";

  server.send(200, "text/html", s);
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot () {
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal()) {
    // -- Captive portal request were already served.
    return;
  }

  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 13 Optional Group</title></head><div>Status page of ";
  s += iotWebConf.getThingName();
  s += ".</div>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved () {
  Serial.println("Configuration was updated.");
}