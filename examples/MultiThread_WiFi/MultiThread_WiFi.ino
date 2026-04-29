/* 
 * Multi Thread SafeSerial Example with WiFi event handling
 * This example demonstrates basic usage of SafeSerial logging
 * from multiple concurrent FreeRTOS tasks with WiFi event handling.
*/

#include <Arduino.h>
#include <esp_log.h>
#include <SafeSerial.h>
#include <WiFi.h>

const char *ssid = "your-ssid";
const char *password = "your-password";

// WARNING: This function is called from a separate FreeRTOS task (thread)!
// This code is a modified version of WiFiClientEvents.ino from ESP32 Arduino Core examples
void networkEvent(arduino_event_id_t event) {
  SafeSerial.printf("Network event occured. Event ID: %u\n", event);

  switch (event) {
    case ARDUINO_EVENT_WIFI_READY:               SafeSerial.println("WiFi interface ready"); break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:           SafeSerial.println("Completed scan for access points"); break;
    case ARDUINO_EVENT_WIFI_STA_START:           SafeSerial.println("WiFi client started"); break;
    case ARDUINO_EVENT_WIFI_STA_STOP:            SafeSerial.println("WiFi clients stopped"); break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:       SafeSerial.println("Connected to access point"); break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:    SafeSerial.println("Disconnected from WiFi access point"); break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE: SafeSerial.println("Authentication mode of access point has changed"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      SafeSerial.printf("Obtained IP address: %s\n", WiFi.localIP().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:         SafeSerial.println("Lost IP address and IP address is reset to 0"); break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:           SafeSerial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_FAILED:            SafeSerial.println("WiFi Protected Setup (WPS): failed in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:           SafeSerial.println("WiFi Protected Setup (WPS): timeout in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_PIN:               SafeSerial.println("WiFi Protected Setup (WPS): pin code in enrollee mode"); break;
    case ARDUINO_EVENT_WIFI_AP_START:            SafeSerial.println("WiFi access point started"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:             SafeSerial.println("WiFi access point stopped"); break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:     SafeSerial.println("Client connected"); break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:  SafeSerial.println("Client disconnected"); break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:    SafeSerial.println("Assigned IP address to client"); break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:   SafeSerial.println("Received probe request"); break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:          SafeSerial.println("AP IPv6 is preferred"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:         SafeSerial.println("STA IPv6 is preferred"); break;
    case ARDUINO_EVENT_ETH_GOT_IP6:              SafeSerial.println("Ethernet IPv6 is preferred"); break;
    case ARDUINO_EVENT_ETH_START:                SafeSerial.println("Ethernet started"); break;
    case ARDUINO_EVENT_ETH_STOP:                 SafeSerial.println("Ethernet stopped"); break;
    case ARDUINO_EVENT_ETH_CONNECTED:            SafeSerial.println("Ethernet link up"); break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:         SafeSerial.println("Ethernet link down"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:               SafeSerial.println("Obtained IP address"); break;
    default:                                     break;
  }
}

// Define tasks
void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);
void Task4(void *pvParameters);
TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;
TaskHandle_t task4Handle;

// The setup function runs once when you press reset or power on the board.
void setup() {
  // Disable ESP IDF logging
  esp_log_level_set("*", ESP_LOG_NONE);

  // Initialize serial communication at 115200 bits per second:
  SafeSerial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1500));
  SafeSerial.println("Multi Thread SafeSerial example with WiFi event handling");

  WiFi.onEvent(networkEvent);
  // delete old config
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_13dBm); // Some buggy S3/C3 boards need to decrease WiFi TX power to avoid RF interference.
  WiFi.setSleep(false);

  // Set up tasks to run independently.
  // Creating tasks with xTaskCreateUniversal() on dedicated CPU cores if more than one is available
  // Parameters: Task function, Name, Stack size, Parameter, Priority, Task handle, Core
  xTaskCreateUniversal(Task, "Task1", 2048, NULL, 1, &task1Handle, 0);  // name Task1, core 0
  xTaskCreateUniversal(Task, "Task2", 2048, NULL, 1, &task2Handle, 0);  // name Task2, core 0
  xTaskCreateUniversal(Task, "Task3", 2048, NULL, 1, &task3Handle, 1);  // name Task3, core 1 if available
  xTaskCreateUniversal(Task, "Task4", 2048, NULL, 1, &task4Handle, 1);  // name Task4, core 1 if available
  // Delete setup() task
  vTaskDelete(NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000)); // loop() not used in this example
}

void Task(void *pvParameters) {
  unsigned long currentMillis;
  unsigned long previousMillis = 0;
  unsigned long period = random(1000, 5000); // Random period in ms
  char *tName = pcTaskGetName(xTaskGetCurrentTaskHandle() ); // Task name
  unsigned int core = xPortGetCoreID(); // CPU core

  // Wait with the print so that the independence of the printing of the tasks becomes clearer.
  vTaskDelay(pdMS_TO_TICKS(period));
  SafeSerial.printf("%s starting on Core %u\n", tName, core);

  for (;;) {
    currentMillis = millis();
    if (currentMillis - previousMillis >=  period) {
      SafeSerial.printf("Periodic Hello from %s, running on Core %u\n", tName, core);
      previousMillis = currentMillis;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
