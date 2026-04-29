/* 
 * SafeSerial Basic Rx/Tx Example
 * This example demonstrates how to use SafeSerial to read from 
 * and write to serial port characer by character.
 * It echoes back every character received on the serial console.
 */

#include <Arduino.h>
#include <SafeSerial.h>

void setup() {
    SafeSerial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1500));
    SafeSerial.println("SafeSerial Rx/Tx test");
}

void loop() {
    if (SafeSerial.available()) {
        uint8_t c = SafeSerial.read();
        SafeSerial.write(c);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}
