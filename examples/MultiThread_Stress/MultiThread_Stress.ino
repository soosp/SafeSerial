/* 
 * Multi-Thread SafeSerial Stress Test Example
 * This example demonstrates how to tune SafeSerial for high-frequency
 * logging from multiple concurrent FreeRTOS tasks on an ESP32-S3.
 */

#include <Arduino.h>
#include <esp_log.h>

// This example use a smaller line buffer (128 B) A sketch-level #define does
// NOT reach the library in Arduino IDE, because each library .cpp is a
// separate translation unit. The size is therefore set as a global build flag
// in the companion file "Modem_A76xx.ino.globals.h" (kept next to this
// sketch).
// Under PlatformIO instead add:
//   build_flags = -D SAFESERIAL_LINE_BUFFER_SIZE=128
#include <SafeSerial.h>

// 2. TEST PARAMETERS:
// You can fine tune this test via the constants below to see when it starts to drop messages
// Example: 
//   rate: 3 ms/message
//   tasks: 10
//   messages: 4 message/task
//   message cycle: 3 ms * 10 task * 4 messages/task = 120 ms
// 10 tasks * 4 messages/task = 40 messages every 120ms cycle.
// This equals ~333 messages per second.
// There was no dropped messages with parameters above on HWUSB port of ESP32-S3
const unsigned int rate = 3;                    // Target interval (ms) per message
const unsigned int tasks = 10;                  // Total concurent logging tasks
const unsigned long period = tasks * rate * 4;  // Total cycle duration
const unsigned long duration = 10000;           // Duration of the test in ms

void Task(void *pvParameters);
TaskHandle_t taskHandles[tasks];

void setup() {
  // 3. SILENCE IDF:
  // Stop internal ESP-IDF logs from interfering with our test.
  esp_log_level_set("*", ESP_LOG_NONE);

  // 4. FINE-TUNING SAFESERIAL:
  // Allocate minimum required stack (2KB is safe for 128-byte lines).
  SafeSerial.setStackSize(2048);
  
  // MATCH QUEUE TO BURST: Set queue depth to handle the peak message wave (30 messages).
  // This prevents log dropping when all tasks fire simultaneously at the period start.
  SafeSerial.setTxQueueSize(tasks * 4); 
  
  // HARDWARE BUFFER: Increase ESP32-S3 internal TX buffer for smoother USB transfers.
  //SafeSerial.setTxBufferSize(4096);
  
  // Initialize and launch the SafeSerial background task.
  SafeSerial.begin(115200);

  // Wait until the serial hardware is ready
  vTaskDelay(pdMS_TO_TICKS(2000));

  SafeSerial.println("Multi-Thread SafeSerial timing example started...");
  
  // 5. TASK DISPATCH:
  // Distribute tasks across both CPU cores.
  char taskName[8];
  for (int i = 0; i < tasks; i++) {
    snprintf(taskName, 8, "Task%02u", i);
    // Alternate tasks between Core 0 and Core 1 (i % 2).
    xTaskCreateUniversal(Task, taskName, 3072, NULL, 1, &taskHandles[i], i % 2);
    // Small stagger to prevent simultaneous startup and provide sorted logs from tasks
    vTaskDelay(pdMS_TO_TICKS(rate));
  }

  // Wait until the tasks are finished
  vTaskDelay(pdMS_TO_TICKS(duration + 6000));

  // Prints the number of dropped messages (if any)
  SafeSerial.printf(F("Dropped messages: %u\n"), SafeSerial.getDroppedMessages());
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1));
}

void Task(void *pvParameters) {
  unsigned long currentMillis, startMillis;
  unsigned long previousMillis = 0;
  char *tName = pcTaskGetName(xTaskGetCurrentTaskHandle());
  unsigned int core = xPortGetCoreID();

  // Offset startup to show clear independent timing in logs.
  SafeSerial.printf("%s starting on Core %u\n", tName, core);
  // A 5-second delay to allow startup messages to be checked.
  vTaskDelay(pdMS_TO_TICKS(5000));

  startMillis = millis();

  for (;;) {
    currentMillis = millis();
    // Periodic trigger logic
    if (currentMillis - startMillis < duration ) {
      if (currentMillis - previousMillis >= period) {
        // Each task sends 4 atomic log lines every 'period'.
        SafeSerial.printf("Periodic Hello from %s, running on Core %u\n", tName, core);
        
        // DIAGNOSTICS: Check if our 2048-byte stack is sufficient.
        // HighWatermark remains constant because SafeSerial's stack usage is deterministic.
        SafeSerial.printf(" SafeSerial Task Stack HighWatermark: %u bytes\n", SafeSerial.getStackHighWatermark());

        // DIAGNOSTICS: Check if 3072-byte stack size for the Tasks is sufficient.
        SafeSerial.printf(" %s Stack HighWatermark: %u bytes\n", tName, uxTaskGetStackHighWaterMark(xTaskGetCurrentTaskHandle()));

        // DIAGNOSTICS: Print the approximate time required to run the task loop once.
        SafeSerial.printf(" %s last period: %u ms\n", tName, millis() - previousMillis);

        previousMillis = currentMillis;
      }
      // Critical: Yield to FreeRTOS idle task to prevent WDT resets.
      vTaskDelay(1); 
    } else {
      // The task has been completed; it can be deleted
      vTaskDelete(NULL);
    }
  }
}