#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// --- WiFi Config ---
const char* ssid = "Your ssid router";
const char* password = "12345678";

// --- LED Matrix Config ---
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D6

MD_Parola matrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// --- NTP Time Client ---
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // GMT+7, update each 60 second

// --- Web Server ---
ESP8266WebServer server(80);

// --- Global Variables ---
String text = "Succsesfull!!!";
int speed = 50;
bool showText = true;

void handleRoot() {
  String html = "<form action='/update' method='get'>"
                "text: <input type='text' name='text'><br>"
                "speed: <input type='number' name='speed'><br>"
                "<input type='submit' value='send'></form>";
  server.send(200, "text/html", html);
}

void handleUpdate() {
  if (server.hasArg("text")) {
    text = server.arg("text");
  }
  if (server.hasArg("speed")) {
    speed = server.arg("speed").toInt();
    if (speed < 10) speed = 10;
    if (speed > 200) speed = 200;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();

  matrix.begin();
  matrix.setIntensity(5);
  matrix.displayClear();

  timeClient.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();

  if (showText) {
    matrix.displayText(text.c_str(), PA_CENTER, speed, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!matrix.displayAnimate()) server.handleClient();
  } else {
    String clock = timeClient.getFormattedTime();
    matrix.displayText(clock.c_str(), PA_CENTER, speed, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!matrix.displayAnimate()) server.handleClient();
  }

  showText = !showText; // Switch between text and clock or reverse 
}
