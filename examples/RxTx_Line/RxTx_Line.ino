/* 
 * SafeSerial Line Rx/Tx Example
 * This example demonstrates how to use SafeSerial to read from 
 * and write to serial port line by line.
 * It echoes back every line received on the serial console.
 */

#include <Arduino.h>

#define SAFESERIAL_LINE_BUFFER_SIZE 10
#include <SafeSerial.h>


void setup() {
    SafeSerial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1500));
    SafeSerial.println("SafeSerial Rx/Tx test");
}

void loop() {
    char buf[SAFESERIAL_LINE_BUFFER_SIZE];
    memset(buf, 0, sizeof(buf));

    if (SafeSerial.available()) {
        // readBytesUntil reads until '\n' or buffer is full
        if (SafeSerial.readBytesUntil('\n', buf, sizeof(buf) - 1)) {
            SafeSerial.println(buf);
        }
    }
    vTaskDelay(1);
}