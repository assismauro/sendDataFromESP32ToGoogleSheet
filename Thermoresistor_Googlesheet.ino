#define GOOGLE_SCRIPT_URL "https://script.google.com/macros/s/<SECRET_CODE_ACCESS>/exec"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include "Thermistor.h"

// WiFi credentials.

char ssid[] = "Casa do Maurao";
char pass[] = "142857bema";

//char ssid[] = "BRENG - 2.4GHz";
//char pass[] = "vector23";


unsigned long lastSheetSend = 0;
const long sheetInterval = 8000; 

Thermistor thermistor(-1); // -1: data will be read from a MCP3008

#define BATCH_SIZE 30  // Send 10 readings at once
#define SEND_INTERVAL 30000  // Send every 10 seconds

struct TemperatureReading {
  float t0, t1, t2;
  unsigned long millisFromStart;
};

unsigned long programStartTime = 0;

TemperatureReading dataBuffer[BATCH_SIZE];
int bufferIndex = 0;
unsigned long lastSendTime = 0;

float t0, t1, t2; 

const byte MOVING_AVERAGE_WINDOW = 4;

float movingAverage(int sensor, float value) {
    static float history[3][MOVING_AVERAGE_WINDOW] = {{0}};
    static byte indices[3] = {0};
    static float sums[3] = {0};
    static byte counts[3] = {0};
    
    // Remove oldest value from sum if window is full
    if (counts[sensor] == MOVING_AVERAGE_WINDOW) {
        sums[sensor] -= history[sensor][indices[sensor]];
    } else {
        counts[sensor]++;
    }
    
    // Add new value
    history[sensor][indices[sensor]] = value;
    sums[sensor] += value;
    
    // Update index
    indices[sensor] = (indices[sensor] + 1) % MOVING_AVERAGE_WINDOW;
    
    return sums[sensor] / counts[sensor];
}

void readThermistorData()
{
  t0 = movingAverage(0, thermistor.temperatureSPI(0));
  t1 = movingAverage(1, thermistor.temperatureSPI(1));
  t2 = movingAverage(2, thermistor.temperatureSPI(2));
}

void sendToGoogleSheets() {
  if (bufferIndex == 0) return;
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); 
    
    http.begin(client, GOOGLE_SCRIPT_URL);
    client.setTimeout(20000);
    http.setTimeout(20000);
    http.setReuse(true);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON with batch data
    StaticJsonDocument<1024> doc;
    JsonArray readings = doc.createNestedArray("readings");
    
    for (int i = 0; i < bufferIndex; i++) {
      JsonObject reading = readings.createNestedObject();
      reading["millisFromStart"] = dataBuffer[i].millisFromStart;
      reading["t0"] = round(dataBuffer[i].t0 * 10) / 10.0;
      reading["t1"] = round(dataBuffer[i].t1 * 10) / 10.0;
      reading["t2"] = round(dataBuffer[i].t2 * 10) / 10.0;
      
      // Debug: Print what we're putting in JSON
      Serial.print("Buffer ");
      Serial.print(i);
      Serial.print(": millis=");
      Serial.print(dataBuffer[i].millisFromStart);
      Serial.print(", t0=");
      Serial.print(dataBuffer[i].t0);
      Serial.print(", t1=");
      Serial.print(dataBuffer[i].t1);
      Serial.print(", t2=");
      Serial.println(dataBuffer[i].t2);
    }
    doc["batch_size"] = bufferIndex;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.print("📤 Sending batch of ");
    Serial.print(bufferIndex);
    Serial.println(" readings");
    
    // CRITICAL: Print the actual JSON being sent
    Serial.print("📄 JSON being sent: ");
    Serial.println(jsonString);
    
    int httpCode = http.POST(jsonString);
    
    String response = http.getString();
    Serial.print("📥 Response: ");
    Serial.println(response);
    
    if (httpCode == 200) {
      Serial.println("✅ Batch saved to Google Sheets");
      bufferIndex = 0;
    } else {
      Serial.print("❌ Batch error: ");
      Serial.println(httpCode);
      Serial.print("Error description: ");
      Serial.println(http.errorToString(httpCode));
    }
    http.end();
  }
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  programStartTime = millis();
  Serial.println("Start Setup");
  for (int i = 0;i<3;i++)
  {
    delay(1000);
  }
  Serial.print("Connection to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
  }
  Serial.println("Starting thermistor...");
  thermistor.useAREF(true);
  Serial.println("thermistor started...");
  Serial.println("End setup.");
}

void loop() {
  // Read data every second
  static unsigned long lastReadTime = 0;
  if (millis() - lastReadTime >= 1000) {
    Serial.print("Reading ");
    Serial.println(millis());
    lastReadTime = millis();
    
    readThermistorData();
    
    // Store in buffer
    if (bufferIndex < BATCH_SIZE) {
      dataBuffer[bufferIndex].t0 = t0;
      dataBuffer[bufferIndex].t1 = t1;
      dataBuffer[bufferIndex].t2 = t2;
      dataBuffer[bufferIndex].millisFromStart = millis() - programStartTime;
      bufferIndex++;
    }
  }
  
  // Send batch periodically
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = millis();
    sendToGoogleSheets();
  }
}
