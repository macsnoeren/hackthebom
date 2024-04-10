#include <Arduino.h>

#include <IotWebConf.h>
#include <IotWebConfOptionalGroup.h>

#define STRING_LEN 128
#define NUMBER_LEN 32

const char SSID[] = "_htb";
const char wifiInitialApPassword[] = "bom";

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(SSID, &dnsServer, &server, wifiInitialApPassword, "opt1");

char timeServerIp[STRING_LEN];
char offsetParamValue[NUMBER_LEN];
char dstParamValue[STRING_LEN];

iotwebconf::OptionalParameterGroup timeServerGroup = iotwebconf::OptionalParameterGroup("iotTimeServer", "Time Server", false);
iotwebconf::TextParameter timeServerIpParam = iotwebconf::TextParameter("IP address", "timeServerIp", timeServerIp, STRING_LEN);
iotwebconf::NumberParameter offsetParam = iotwebconf::NumberParameter("Offset", "timeOffset", offsetParamValue, NUMBER_LEN, "0", "-23..23", "min='-23' max='23' step='1'");
iotwebconf::CheckboxParameter dstParam = iotwebconf::CheckboxParameter("DST ON", "timeDst", dstParamValue, STRING_LEN,  false);
iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

void configSaved();
void handleRoot();

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  timeServerGroup.addItem(&timeServerIpParam);
  timeServerGroup.addItem(&offsetParam);
  timeServerGroup.addItem(&dstParam);

//  iotWebConf.setStatusPin(STATUS_PIN);
//  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setHtmlFormatProvider(&optionalGroupHtmlFormatProvider);
  iotWebConf.addParameterGroup(&timeServerGroup);
  iotWebConf.setConfigSavedCallback(&configSaved);

  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  Serial.println("Ready.");
}

void loop() {
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot () {
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 13 Optional Group</title></head><div>Status page of ";
  s += iotWebConf.getThingName();
  s += ".</div>";

  if (timeServerGroup.isActive())
  {
    s += "<div>Time Server defined</div>";
    s += "<ul>";
    s += "<li>Time server IP: ";
    s += timeServerIp;
    s += "<li>Offset: ";
    s += atoi(offsetParamValue);
    s += "<li>DST: ";
    s += dstParam.isChecked() ? "ON" : "OFF";
    s += "</ul>";
    
  } else {
    s += "<div>Time Server not defined</div>";
  }
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved () {
  Serial.println("Configuration was updated.");
}