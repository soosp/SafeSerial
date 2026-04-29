/*
 * SafeSerial API Reference Example
 * This example demonstrates all available SafeSerial methods.
 * It can be used as a reference or a smoke test on a new platform.
 */

#include <Arduino.h>
#include <esp_log.h>
#include <time.h>

#define SAFESERIAL_LINE_BUFFER_SIZE 256
#include <SafeSerial.h>

void setup() {
    // Disable ESP IDF logging
    esp_log_level_set("*", ESP_LOG_NONE);

    // Initialize SafeSerial
    SafeSerial.setStackSize(2048);
    SafeSerial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1500));

    SafeSerial.println("--- SafeSerial API Reference Example ---\n");

    // -------------------------------------------------------------------------
    // print() variants
    // -------------------------------------------------------------------------
    SafeSerial.println("-- print() --");

    SafeSerial.print("print(const char[]):       ");
    SafeSerial.print("Hello, World!");
    SafeSerial.print("\n");

    SafeSerial.print("print(String):             ");
    String str = "Hello from String";
    SafeSerial.print(str);
    SafeSerial.print("\n");

    SafeSerial.print("print(F()):                ");
    SafeSerial.print(F("Hello from Flash"));
    SafeSerial.print("\n");

    SafeSerial.print("print(int, DEC):           ");
    SafeSerial.print(-42, DEC);
    SafeSerial.print("\n");

    SafeSerial.print("print(int, HEX):           ");
    SafeSerial.print(255, HEX);
    SafeSerial.print("\n");

    SafeSerial.print("print(float, 2 places):    ");
    SafeSerial.print(3.14159f, 2);
    SafeSerial.print("\n");

    SafeSerial.print("print(float, 4 places):    ");
    SafeSerial.print(3.14159f, 4);
    SafeSerial.print("\n");

    // IPAddress can be created manually without a WiFi connection
    SafeSerial.print("print(IPAddress):          ");
    IPAddress ip(192, 168, 1, 100);
    SafeSerial.print(ip);
    SafeSerial.print("\n");

    // struct tm can be filled manually without NTP
    SafeSerial.print("print(struct tm*, format): ");
    struct tm timeinfo = {};
    timeinfo.tm_year = 126;  // Years since 1900 (2026)
    timeinfo.tm_mon  = 0;    // January (0-indexed)
    timeinfo.tm_mday = 1;
    timeinfo.tm_hour = 12;
    timeinfo.tm_min  = 0;
    timeinfo.tm_sec  = 0;
    SafeSerial.print(&timeinfo, "%Y-%m-%d %H:%M:%S");
    SafeSerial.print("\n");

    // -------------------------------------------------------------------------
    // println() variants
    // -------------------------------------------------------------------------
      // Wait for the TX queue to drain before continuing
    vTaskDelay(pdMS_TO_TICKS(500));
    SafeSerial.println("\n-- println() --");

    SafeSerial.println("println(const char[]):     Hello, World!");
    SafeSerial.println(String("println(String):           Hello from String"));
    SafeSerial.println(F("println(F()):              Hello from Flash"));

    SafeSerial.print("println(int, DEC):         ");
    SafeSerial.println(-42, DEC);

    SafeSerial.print("println(int, HEX):         ");
    SafeSerial.println(255, HEX);

    SafeSerial.print("println(float, 2 places):  ");
    SafeSerial.println(3.14159f, 2);

    SafeSerial.print("println(float, 4 places):  ");
    SafeSerial.println(3.14159f, 4);

    SafeSerial.print("println(IPAddress):        ");
    SafeSerial.println(ip);

    SafeSerial.print("println(struct tm*):       ");
    SafeSerial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

    // -------------------------------------------------------------------------
    // printf() variants
    // -------------------------------------------------------------------------
    // Wait for the TX queue to drain before continuing
    vTaskDelay(pdMS_TO_TICKS(500));
    SafeSerial.println("\n-- printf() --");

    SafeSerial.printf("printf(char*, ...):        %s, %d, %.2f\n", "Hello", 42, 3.14f);
    SafeSerial.printf(F("printf(F(), ...):          %s, %d, %.2f\n"), "Hello", 42, 3.14f);

    // -------------------------------------------------------------------------
    // write() variants
    // -------------------------------------------------------------------------
    // Wait for the TX queue to drain before continuing
    vTaskDelay(pdMS_TO_TICKS(500));
    SafeSerial.println("\n-- write() --");

    SafeSerial.print("write(uint8_t):            ");
    SafeSerial.write('A');
    SafeSerial.print("\n");

    SafeSerial.print("write(buffer, size):       ");
    const uint8_t buf[] = "Hello from buffer";
    SafeSerial.write(buf, sizeof(buf) - 1);  // -1 to exclude the null terminator
    SafeSerial.print("\n");

    // -------------------------------------------------------------------------
    // Diagnostics
    // -------------------------------------------------------------------------
    // Wait for the TX queue to drain before continuing
    vTaskDelay(pdMS_TO_TICKS(500));
    SafeSerial.println("\n-- Diagnostics --");
    SafeSerial.printf("getStackHighWatermark():   %u bytes\n", SafeSerial.getStackHighWatermark());
    SafeSerial.printf("getDroppedMessages():      %u\n", SafeSerial.getDroppedMessages());

    SafeSerial.println("\n-- Read methods (available, read, peek) --");
    SafeSerial.println("Type something and press Enter...");
}

void loop() {
    // -------------------------------------------------------------------------
    // Read methods: available(), peek(), read()
    // -------------------------------------------------------------------------
    if (SafeSerial.available()) {
        // peek() returns the next byte without removing it from the queue
        int peeked = SafeSerial.peek();
        SafeSerial.printf("peek():                    '%c' (0x%02X)\n", (char)peeked, peeked);

        // read() removes and returns the next byte from the queue
        int c = SafeSerial.read();
        SafeSerial.printf("read():                    '%c' (0x%02X)\n", (char)c, c);

        SafeSerial.printf("available() after read():  %d bytes remaining\n", SafeSerial.available());
    }

    vTaskDelay(pdMS_TO_TICKS(10));
}