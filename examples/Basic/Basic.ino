/* 
 * SafeSerial Basic Example
 * This example demonstrates basic usage of SafeSerial logging.
 */

#include <SafeSerial.h>
#include <esp_log.h>

void setup() {
  // Silence internal ESP-IDF logs that bypass our queue logic (optional)
  esp_log_level_set("*", ESP_LOG_NONE);

  // Start SafeSerial (automatically detects port type)
  SafeSerial.begin(115200); 
  vTaskDelay(pdMS_TO_TICKS(1500));
}

void loop() {
  SafeSerial.println("This won't block your code!");
  vTaskDelay(pdMS_TO_TICKS(1000));
}