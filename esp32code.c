#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#include <freertos/semphr.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

#define ch4_PIN 36               // MQ2 sensor pin
#define sound_PIN 34             // Sound sensor pin
#define buzzer 2                 // Buzzer

// WiFi credentials
const char* ssid = "Networkname";
const char* password = "Networkpass";

// WebSocket server
WebSocketsServer webSocket = WebSocketsServer(81);

// Function declarations
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void showModal(String message);
void closeModal();

// Tasks
void security(void *pvParameters);
void gastemp(void *pvParameters);
void opentask(void *pvParameters);
void sleeptask(void *pvParameters);

// Semaphores
SemaphoreHandle_t openSemaphore;
SemaphoreHandle_t gastempSemaphore;
SemaphoreHandle_t securitySemaphore;
SemaphoreHandle_t sleepSemaphore;

struct SensorData {
  int ch4;
  int sound;
};
SensorData sensorData;

ESP32Time rtc(32400);  // Offset in seconds GMT+3
int currentHour = rtc.getHour();

bool securityAlert = false;
int securityStartHour = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 Monitoring with FreeRTOS Tasks.");
  
  pinMode(buzzer, OUTPUT);

  // Initialize the semaphores
  openSemaphore = xSemaphoreCreateBinary();
  gastempSemaphore = xSemaphoreCreateBinary();
  securitySemaphore = xSemaphoreCreateBinary();
  sleepSemaphore = xSemaphoreCreateBinary();

  if (openSemaphore == NULL || gastempSemaphore == NULL || securitySemaphore == NULL || sleepSemaphore == NULL) {
    Serial.println("Failed to create semaphores");
    while (1);
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  // Start with the open task and define time
  xSemaphoreGive(openSemaphore);
  Serial.print("Current Hour: ");
  Serial.println(currentHour);
  
  // Print the IP address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Create the tasks
  xTaskCreatePinnedToCore(opentask, "open Task", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(sleeptask, "Sleep Task", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(gastemp, "measure CH4 ", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(security, "check if anyone in room at certain time", 2048, NULL, 1, NULL, 1);

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  delay(10); // Small delay to avoid excessive CPU usage
}

void opentask(void *pvParameters) {
  while (true) {
    if (xSemaphoreTake(openSemaphore, portMAX_DELAY)) {
      Serial.println("Turning on device");
      xSemaphoreGive(gastempSemaphore); // Signal the gastemp task to run
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Add a small delay to avoid task hogging the CPU
  }
}

void gastemp(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(gastempSemaphore, portMAX_DELAY)) {
      sensorData.ch4 = analogRead(ch4_PIN);

      String data = "Gas: " + String(sensorData.ch4);
      Serial.println("Sending data: " + data); // Debugging line
      webSocket.broadcastTXT(data);

      if (sensorData.ch4 > 5000) {
        securityAlert = true;
        digitalWrite(buzzer, HIGH);
        showModal("Danger detected!");
      } else {
        Serial.println("No danger");
      }

      xSemaphoreGive(securitySemaphore); // Signal the security task to run
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

void security(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(securitySemaphore, portMAX_DELAY)) {
      sensorData.sound =analogRead(sound_PIN) ;
      int dacVal = map(sensorData.sound, 0, 4095, 0, 255);
      double voltage = sensorData.sound / 4095.0 * 3.3;
      Serial.print("Sound Level: ");
      Serial.println(sensorData.sound);
      delay(2000);

      // Check if within the time set on the website
      if (securityStartHour  >= currentHour ) {
        String sounddata = "Sound: " + String(sensorData.sound);     
        Serial.println("Sending data: " + sounddata); // Debugging line
        webSocket.broadcastTXT(sounddata);

        if (sensorData.sound > 10) {
          securityAlert = true;
          //digitalWrite(buzzer, HIGH);
          tone(buzzer, 1000); // Generate a 1kHz tone
          showModal("Security alert!");
        } else {
          Serial.println("No one is here");
        }
      } else {
        Serial.println("Not now");
      }

      xSemaphoreGive(sleepSemaphore); // Signal the sleeptask to run
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void sleeptask(void *pvParameters) {
  while (true) {
    if (xSemaphoreTake(sleepSemaphore, portMAX_DELAY)) {
        Serial.println("Turning off device");
        Serial.println("Entering deep sleep mode...");
        delay(2000);
        Watchdog.sleep(3000);
        Serial.println("Just woke up");
        xSemaphoreGive(openSemaphore); // Signal the opentask to run
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Add a small delay to avoid task hogging the CPU
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %s\n", num, ip.toString().c_str());
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (strcmp((char*)payload, "OK") == 0) {
        securityAlert = false;
        //digitalWrite(buzzer, LOW);
        noTone(buzzer);     // Stop the tone
      } else {
        // Assume payload contains time in HH:MM-HH:MM format
        String timeStr = String((char*)payload);
        int startHour = timeStr.substring(0, 2).toInt();
        int startMinute = timeStr.substring(3, 5).toInt();

        
        // Set the start time
        rtc.setTime(0 , startMinute, startHour, 0 , 0 , 0);
        securityStartHour = startHour;
        Serial.printf("Security time set to %02d:%02d - %02d:%02d\n", startHour, startMinute);
      }
      break;
  }
}

void showModal(String message) {
  String alertMessage = "ALERT: " + message;
  webSocket.broadcastTXT(alertMessage);
}
