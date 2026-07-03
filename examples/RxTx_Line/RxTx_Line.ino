/* 
 * SafeSerial Line Rx/Tx Example
 * This example demonstrates how to use SafeSerial to read from 
 * and write to serial port line by line.
 * It echoes back every line received on the serial console.
 */

#include <Arduino.h>
#include <SafeSerial.h>

// Small sketch-local line buffer to demonstrate line-by-line reading.
// NOTE: this only sizes the buffer in THIS sketch; it does NOT change the
// library's internal buffer (SAFESERIAL_LINE_BUFFER_SIZE is a build flag).
constexpr size_t LINE_LEN = 10;


void setup() {
    SafeSerial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1500));
    SafeSerial.println("SafeSerial Rx/Tx test");
}

void loop() {
    char buf[LINE_LEN];
    // Fill buffer with zeros
    memset(buf, 0, sizeof(buf));

    if (SafeSerial.available()) {
        // readBytesUntil reads until '\n' or buffer is full
        // This cuts the message to the appropriate line length so that the zero at the end remains
        if (SafeSerial.readBytesUntil('\n', buf, sizeof(buf) - 1)) {
            SafeSerial.println(buf);
        }
    }
    vTaskDelay(1);
}