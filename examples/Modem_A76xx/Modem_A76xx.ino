/* 
 * SafeSerial Dual Instance Example – SimCom A76xx AT Console
 * This example demonstrates how to use two concurrent SafeSerial instances:
 * one for the debug console (SafeSerial) and one for a modem (modem).
 * Characters typed in the serial console are forwarded to the modem as AT commands,
 * and the modem's responses are printed back to the console.
 */

#include <Arduino.h>
#include <SafeSerial.h>

// This example runs with the default 256-byte line buffer, which is enough
// for typical AT command/response traffic. Very long responses (large HTTP
// bodies, verbose GNSS output) are truncated at 255 characters.
//
// To raise the limit, set SAFESERIAL_LINE_BUFFER_SIZE as a GLOBAL BUILD FLAG
// (a sketch-level #define does not reach the library .cpp). See the README
// section "How to Change It". In short:
//   - PlatformIO:  build_flags = -D SAFESERIAL_LINE_BUFFER_SIZE=512
//   - Arduino IDE: a companion <sketch>.ino.globals.h file. NOTE: such a file
//     is intentionally NOT shipped here, because a *.ino.globals.h file in an
//     example folder hides the example from the IDE's Examples menu.

// Pin definitions (ESP32-S3 example)
#define MODEM_RX  40  // GPIO pin connected to modem TX
#define MODEM_TX  39  // GPIO pin connected to modem RX
#define MODEM_PWR  6  // GPIO pin connected to modem power key

// Second SafeSerial instance for the modem on Serial1
SafeSerialClass modem;

void setup() {
    // Initialize debug console
    SafeSerial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1500));
    SafeSerial.println("--- SimCom A76xx Test ---");

    // Configure and initialize modem UART
    modem.setRxQueueSize(1024);  // Large RX queue for long AT responses
    modem.setTxQueueSize(20);    // Small TX queue: AT commands are short
    modem.begin(Serial1, 115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

    // Hardware reset: pull power key low then high to boot the modem
    pinMode(MODEM_PWR, OUTPUT);
    digitalWrite(MODEM_PWR, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));   // Hold reset
    digitalWrite(MODEM_PWR, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1500));  // Wait for modem to boot
}

void loop() {
    char buf[SAFESERIAL_LINE_BUFFER_SIZE];
    memset(buf, 0, sizeof(buf));

    // Forward modem responses to the debug console
    if (modem.available()) {
        if (modem.readBytesUntil('\n', buf, sizeof(buf) - 1)) {
            SafeSerial.println(buf);
            memset(buf, 0, sizeof(buf));
        }
        vTaskDelay(1);
    }

    // Forward console input to the modem as AT commands
    if (SafeSerial.available()) {
        if (SafeSerial.readBytesUntil('\n', buf, sizeof(buf) - 1)) {
            modem.printf("%s\r", buf);  // SimCom requires \r as line terminator
            memset(buf, 0, sizeof(buf));
        }
        vTaskDelay(1);
    }

    vTaskDelay(1);
}