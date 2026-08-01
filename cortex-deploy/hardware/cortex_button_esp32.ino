/* ============================================================
   CORTEX·01 — ESP32 Wi-Fi button

   Serves the same API your Streamlit dashboard already uses:
     GET /counter -> {"counter": 12}
     GET /reset   -> {"counter": 0}

   The one addition the browser needs is the CORS header. Streamlit
   works without it because Python fetches server-side, where the
   same-origin policy does not apply. JavaScript in a tab is subject
   to it, so without this header the browser throws the reply away
   even though the ESP32 answered correctly.

   Board: ESP32 / ESP32-S3.  Library: WebServer (bundled).
   ============================================================ */

#include <WiFi.h>
#include <WebServer.h>

const char *WIFI_SSID = "YOUR_WIFI";
const char *WIFI_PASS = "YOUR_PASSWORD";

const int BUTTON_PIN  = 4;      // button to GND, internal pull-up
const int DEBOUNCE_MS = 40;

WebServer server(80);

volatile long pressCount = 0;
int  lastReading = HIGH;
int  stableState = HIGH;
unsigned long lastEdge = 0;

// ---- LED matrix hook -------------------------------------------------
// Fill in for your matrix (MAX7219, HUB75, NeoPixel grid, ...).
void showCount(long n) {
  Serial.printf("count = %ld\n", n);
}

// Every response carries CORS, so any page on the LAN can read it.
void sendJSON(const String &body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", body);
}

void handleCounter() {
  sendJSON("{\"counter\":" + String(pressCount) + "}");
}

void handleReset() {
  pressCount = 0;
  showCount(pressCount);
  sendJSON("{\"counter\":0,\"status\":\"reset\"}");
}

// Browsers may send a preflight OPTIONS before the real request.
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.send(204);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); }

  Serial.println();
  Serial.print("ready at http://");
  Serial.println(WiFi.localIP());     // <- paste this into the page

  server.on("/counter", HTTP_GET, handleCounter);
  server.on("/reset",   HTTP_GET, handleReset);
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) handleOptions();
    else {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(404, "application/json", "{\"error\":\"not found\"}");
    }
  });
  server.begin();

  showCount(pressCount);
}

void loop() {
  server.handleClient();

  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastReading) {
    lastEdge = millis();
    lastReading = reading;
  }
  if (millis() - lastEdge > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {          // falling edge = press
      pressCount++;
      showCount(pressCount);
    }
  }
}
