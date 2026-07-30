/* ============================================================
   CORTEX·01 — ESP32 BLE button
   Pairs with cortex.html over Web Bluetooth (Nordic UART Service).

   Sends  "P:<count>\n"  on every debounced press.
   Accepts "C:<count>\n" from the browser to stay in sync
           (e.g. after the browser's Reset count).

   Board: any ESP32 / ESP32-S3 with BLE.
   Library: "ESP32 BLE Arduino" (bundled with the ESP32 board package).
   ============================================================ */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "CORTEX-BTN"

// Must match the UUIDs at the bottom of cortex.html
#define SVC_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RX_UUID  "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // browser -> ESP (write)
#define TX_UUID  "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // ESP -> browser (notify)

const int BUTTON_PIN  = 4;      // button to GND, using the internal pull-up
const int DEBOUNCE_MS = 40;

BLECharacteristic *txChar;
bool connected = false;
long  pressCount   = 0;
int   lastReading  = HIGH;
int   stableState  = HIGH;
unsigned long lastEdge = 0;

// ---- LED matrix hook -------------------------------------------------
// Fill this in for your matrix (MAX7219, HUB75, NeoPixel grid, ...).
void showCount(long n) {
  Serial.printf("count = %ld\n", n);
}

class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override    { connected = true; }
  void onDisconnect(BLEServer *s) override {
    connected = false;
    s->getAdvertising()->start();          // let the browser find us again
  }
};

class RxCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String v = String(c->getValue().c_str());
    int i = v.indexOf("C:");
    if (i >= 0) {
      pressCount = v.substring(i + 2).toInt();
      showCount(pressCount);
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  BLEDevice::init(DEVICE_NAME);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCB());

  BLEService *svc = server->createService(SVC_UUID);

  txChar = svc->createCharacteristic(TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  txChar->addDescriptor(new BLE2902());

  BLECharacteristic *rxChar = svc->createCharacteristic(
      RX_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxChar->setCallbacks(new RxCB());

  svc->start();

  // Advertise the service UUID — the page filters on it, so this is required.
  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->setScanResponse(true);
  adv->start();

  showCount(pressCount);
  Serial.println("advertising as " DEVICE_NAME);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastEdge = millis();
    lastReading = reading;
  }

  if (millis() - lastEdge > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {             // falling edge = press
      pressCount++;
      showCount(pressCount);
      if (connected) {
        char buf[24];
        int n = snprintf(buf, sizeof(buf), "P:%ld\n", pressCount);
        txChar->setValue((uint8_t *)buf, n);
        txChar->notify();
      }
    }
  }
}
