/* 
 * Basic Multi Thread SafeSerial Example
 * This example demonstrates basic usage of SafeSerial logging
 * from multiple concurrent FreeRTOS tasks.
*/

#include <Arduino.h>
#include <esp_log.h>
#include <SafeSerial.h>

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
  vTaskDelay(pdMS_TO_TICKS(2000));
  SafeSerial.println("Basic Multi Thread SafeSerial example");

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
