#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>


const char* ssid     = "Wokwi-GUEST";
const char* password = "";


constexpr char THINGSBOARD_SERVER[] = "thingsboard.cloud";
constexpr char TOKEN[]              = "auOD7G1FeuaAb96d11Zl";

#define PIEZO_LEFT   34
#define PIEZO_CENTER 35
#define PIEZO_RIGHT  32
#define LED_PIN       2

#define VIBRATION_THRESHOLD  100
#define CLOUD_POLL_INTERVAL 5000 // Send data every 5 seconds

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient);

unsigned long lastPollTime = 0;

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected!");
}

void connectToThingsBoard() {
  if (!tb.connected()) {
    Serial.print("Connecting to ThingsBoard node...");
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
      return;
    }
    Serial.println(" Connected to ThingsBoard!");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIEZO_LEFT,   INPUT);
  pinMode(PIEZO_CENTER, INPUT);
  pinMode(PIEZO_RIGHT,  INPUT);
  pinMode(LED_PIN, OUTPUT);

  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  if (!tb.connected()) {
    connectToThingsBoard();
  }

  tb.loop();

  if (millis() - lastPollTime > CLOUD_POLL_INTERVAL) {
    lastPollTime = millis();

    int left   = analogRead(PIEZO_LEFT);
    int center = analogRead(PIEZO_CENTER);
    int right  = analogRead(PIEZO_RIGHT);


    bool activeLeft   = (left > 500);
    bool activeCenter = (center > 500);
    bool activeRight  = (right > 500);

    Serial.printf("Sending Data -> L: %d (%s) | C: %d (%s) | R: %d (%s)\n", 
                  left, activeLeft ? "ACTIVE" : "OFF", 
                  center, activeCenter ? "ACTIVE" : "OFF", 
                  right, activeRight ? "ACTIVE" : "OFF");

    tb.sendTelemetryData("left", left);
    tb.sendTelemetryData("center", center);
    tb.sendTelemetryData("right", right);
    
 
    tb.sendTelemetryData("leftActive", activeLeft);
    tb.sendTelemetryData("centerActive", activeCenter);
    tb.sendTelemetryData("rightActive", activeRight);


    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }

  delay(10);
}
