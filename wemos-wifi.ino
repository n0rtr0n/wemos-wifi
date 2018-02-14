#include <Time.h>
#include <TimeAlarms.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define PIN D3

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "ARRIS-EA42";
const char* password = "BPP8FD500994";

const char* mode = "default";
const char* previousMode = "";

uint32_t msCurrentModeStartedAt = 0;
uint32_t msSinceCurrentMode = 0;
uint8_t brightness = 0;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  server.send(200, "text/plain", "usage:\n/rainbow");
}

void handleNotFound(){
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  //Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/rainbow", respondRainbow);
  server.on("/off", respondOff);
  server.on("/blue", respondBlue);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void respondRainbow() {
  server.send(200, "text/plain", "rainbow");
  switchMode("rainbow");
}

void respondBlue() {
  server.send(200, "text/plain", "blue");
  switchMode("blue");
}

void respondOff() {
  
  server.send(200, "text/plain", "shutting off...");
  switchMode("off");
}

void switchMode(char* newMode) {
  if (mode != newMode) {
    previousMode = mode;
    mode = newMode;
    msCurrentModeStartedAt = millis();
    strip.setBrightness(50);
  }
  return;
}

void loop(void){
  server.handleClient();

  if (mode == "rainbow") {
    rainbow(20);
  } else if (mode == "off") {
    off();
  } else if (mode == "blue") {
    blue();
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void off() {
  for(int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  strip.show();
}

void blue() {
  msSinceCurrentMode = millis() - msCurrentModeStartedAt;
  
  if (msSinceCurrentMode >= 300000) {
    brightness = 100;
  } else {
    brightness = (uint8_t) (msSinceCurrentMode / 3000);
  }
  
  strip.setBrightness(brightness);
  for(int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(30,30,255));
  }
  strip.show();
}

