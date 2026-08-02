/* ============================================================
   CORTEX·01 — ESP32 Wi-Fi button + WS2812B pulse strip

   Serves the same API the web page and Streamlit dashboard use:
     GET /counter -> {"counter": 12}
     GET /reset   -> {"counter": 0, "status": "reset"}

   Every response carries Access-Control-Allow-Origin. Streamlit
   works without it because Python fetches server-side, where the
   same-origin policy does not apply. JavaScript in a browser tab
   is subject to it, so without this header the browser throws the
   reply away even though the ESP32 answered correctly.

   Each press launches a pulse down the LED strip. The pulse color
   shifts from white toward full red as the count approaches
   TRANSITION_PRESSES.

   Board: ESP32 / ESP32-S3.  Libraries: WebServer (bundled), FastLED.
   ============================================================ */

#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>

#define Button_Pin 5
#define LED 7
#define num_leds 100

#define MAX_PULSES 8
#define PULSE_SPEED 10
#define TRAIL_FADE 20

#define TRANSITION_PRESSES 300   // how many presses to go from white to full red

// ---- WiFi credentials ----
const char* ssid = "router12345";
const char* password = "router12345";

WebServer server(80);

CRGB leds[num_leds];

struct Pulse {
  bool active = false;
  unsigned long startTime = 0;
};

Pulse pulses[MAX_PULSES];

int currentState = HIGH;
int lastState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
int counter = 0;

// ---- HTTP handlers ----
// Every response carries CORS, so any page on the LAN can read it.
void sendJSON(const String &body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", body);
}

void handleGetCounter() {
  sendJSON("{\"counter\":" + String(counter) + "}");
}

void handleResetCounter() {
  counter = 0;
  Serial.println("Counter reset via HTTP");
  sendJSON("{\"counter\":" + String(counter) + ",\"status\":\"reset\"}");
}

// Browsers may send a preflight OPTIONS before the real request.
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.send(204);
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());     // <- paste this into the page
}

void setup() {
  delay(500);
  pinMode(Button_Pin, INPUT_PULLUP);
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED, GRB>(leds, num_leds).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(150);
  FastLED.clear();
  FastLED.show();

  setupWiFi();

  server.on("/counter", HTTP_GET, handleGetCounter);
  server.on("/reset", HTTP_GET, handleResetCounter);
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) handleOptions();
    else {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(404, "application/json", "{\"error\":\"not found\"}");
    }
  });
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("READY");
}

void triggerPulse() {
  for (int i = 0; i < MAX_PULSES; i++) {
    if (!pulses[i].active) {
      pulses[i].active = true;
      pulses[i].startTime = millis();
      Serial.print("Pulse started in slot ");
      Serial.println(i);
      return;
    }
  }
  Serial.println("Max pulses reached, ignoring press");
}

void updateButton() {
  int reading = digitalRead(Button_Pin);

  if (reading != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentState) {
      currentState = reading;
      if (currentState == LOW) {
        triggerPulse();
        counter += 1;
        Serial.println(counter);
      }
    }
  }

  lastState = reading;
}

// Interpolates from white -> red as counter approaches TRANSITION_PRESSES
CRGB getPulseColor() {
  int p = min(counter, TRANSITION_PRESSES);
  uint8_t g = map(p, 0, TRANSITION_PRESSES, 255, 0);
  uint8_t b = map(p, 0, TRANSITION_PRESSES, 255, 0);
  return CRGB(255, g, b);
}

void updatePulses() {
  fadeToBlackBy(leds, num_leds, TRAIL_FADE);

  CRGB pulseColor = getPulseColor();

  for (int i = 0; i < MAX_PULSES; i++) {
    if (!pulses[i].active) continue;

    unsigned long elapsed = millis() - pulses[i].startTime;
    int pos = elapsed / PULSE_SPEED;

    if (pos >= num_leds) {
      pulses[i].active = false;
      continue;
    }

    leds[pos] = pulseColor;
  }

  FastLED.show();
}

void loop() {
  server.handleClient();  // non-blocking, checks for incoming HTTP requests
  updateButton();
  updatePulses();
}
