#include <WiFi.h>
#include <ThingSpeak.h>

// ── WiFi (Wokwi uses a built-in simulated network) ──────────────────────────
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// ── ThingSpeak ───────────────────────────────────────────────────────────────
// REPLACE with your real values from ThingSpeak → API Keys tab
unsigned long channelID   = 3289277;             // ← Channel ID
const char*   writeAPIKey = "CFVPTGQH6GE4W3EE"; // ← Write API Key
const char*   readAPIKey  = "H73RMNJFQUC2P6MT";  // ← Read API Key

// ── ThingSpeak Fields ────────────────────────────────────────────────────────
// Field 1 → Left   sensor vibration strength
// Field 2 → Center sensor vibration strength
// Field 3 → Right  sensor vibration strength
// Field 4 → Remote Trigger (dashboard writes 1 here to activate sensors)

// ── Hardware pins (ESP32 analog input-only pins) ─────────────────────────────
#define PIEZO_LEFT   34   // Left   bed sensor  → GPIO34
#define PIEZO_CENTER 35   // Center bed sensor  → GPIO35
#define PIEZO_RIGHT  32   // Right  bed sensor  → GPIO32
#define LED_PIN       2   // Built-in LED (lights up when trigger received)

// ── Thresholds & timing ───────────────────────────────────────────────────────
#define VIBRATION_THRESHOLD  100   // Minimum ADC value to count as vibration
#define CLOUD_POLL_INTERVAL 15000  // ms between ThingSpeak reads (free tier = 15 s)

WiFiClient client;

unsigned long lastPollTime = 0;   // last time we polled ThingSpeak for trigger

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(PIEZO_LEFT,   INPUT);
  pinMode(PIEZO_CENTER, INPUT);
  pinMode(PIEZO_RIGHT,  INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected!");

  ThingSpeak.begin(client);
  Serial.println(" ThingSpeak ready. Waiting for dashboard trigger...");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {

  // ── STEP 1: Poll ThingSpeak for a remote trigger (Field 4) every 15 s ────
  if (millis() - lastPollTime > CLOUD_POLL_INTERVAL) {
    lastPollTime = millis();

    Serial.println("\n Polling ThingSpeak for trigger...");
    long trigger = ThingSpeak.readLongField(channelID, 4, readAPIKey);
    int  status  = ThingSpeak.getLastReadStatus();

    if (status == 200 && trigger == 1) {

      Serial.println(" TRIGGER RECEIVED from ThingSpeak Dashboard!");
      digitalWrite(LED_PIN, HIGH);   // Flash LED to confirm trigger received

      // ── STEP 2: Read all 3 piezo sensors ─────────────────────────────────
      int left   = analogRead(PIEZO_LEFT);
      int center = analogRead(PIEZO_CENTER);
      int right  = analogRead(PIEZO_RIGHT);

      Serial.printf("   LEFT   sensor: %d\n", left);
      Serial.printf("   CENTER sensor: %d\n", center);
      Serial.printf("   RIGHT  sensor: %d\n", right);

      // Detect which positions have vibration above threshold
      if (left   > VIBRATION_THRESHOLD) Serial.println("    LEFT   side of bed is active!");
      if (center > VIBRATION_THRESHOLD) Serial.println("    CENTER of bed is active!");
      if (right  > VIBRATION_THRESHOLD) Serial.println("    RIGHT  side of bed is active!");

      // ── STEP 3: Send all 3 sensor readings back to ThingSpeak ────────────
      ThingSpeak.setField(1, left);    // Field 1 = Left sensor
      ThingSpeak.setField(2, center);  // Field 2 = Center sensor
      ThingSpeak.setField(3, right);   // Field 3 = Right sensor

      Serial.print(" Uploading sensor readings to ThingSpeak... ");
      int response = ThingSpeak.writeFields(channelID, writeAPIKey);

      if (response == 200) {
        Serial.println(" All 3 sensor values uploaded!");
      } else {
        Serial.printf(" Upload failed! HTTP Code: %d\n", response);
      }

      digitalWrite(LED_PIN, LOW);  // Turn off LED
    } else {
      Serial.println("    No trigger. Standing by...");
    }
  }

  delay(10);
}
