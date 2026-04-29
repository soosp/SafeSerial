/*
 * SafeSerial Library for ESP32 Series (Universal Asynchronous Serial port handler)
 * Created: 2026, Péter Soós in collaboration with Google Gemini
 * License: MIT
 */

#ifndef SAFESERIAL_H
#define SAFESERIAL_H

#include <Arduino.h>
#include <stdarg.h>

// Sets maximum length of buffer for a single log line (bytes).
#ifndef SAFESERIAL_LINE_BUFFER_SIZE
  #define SAFESERIAL_LINE_BUFFER_SIZE 256
#endif

// Handle Native USB CDC definitions for S3/C3 variants
#if (ARDUINO_USB_MODE == 1) || (ARDUINO_USB_CDC_ON_BOOT == 1)
  #include <USB.h>
#endif

/**
 * @brief Universal ESP32 Asynchronous Thread-safe Serial Class.
 * Offloads serial reads and writes to a dedicated FreeRTOS task to prevent blocking.
 * Inherits from Stream to support standard Arduino read/print methods.
 */
class SafeSerialClass : public Stream {

private:
  Stream* _stream;                    // Pointer to the underlying Serial object
  uint16_t _txQueueSize;              // Number of lines the TX queue can hold
  uint16_t _rxQueueSize;              // Number of bytes the RX queue can hold
  uint8_t  _taskPriority;             // FreeRTOS task priority
  TaskHandle_t _taskHandle;           // Task handler
  uint32_t _stackSize;                // Stack allocated for the background task
  BaseType_t _core;                   // Core ID where the task runs

  size_t _rxBufferSize;               // RX FIFO buffer size
  size_t _txBufferSize;               // TX FIFO buffer size
  uint32_t _txTimeoutMs;              // Timeout for write operations
  uint8_t _maxRxBurst;                // Maximum bytes read per task cycle to prevent RX from starving TX

  uint32_t _droppedMessages;          // Number of dropped messages since initialization

  QueueHandle_t _txQueue;             // FreeRTOS queue handle for TX
  QueueHandle_t _rxQueue;             // FreeRTOS queue handle for RX

  void _beginTask();                  // Internal method to launch the task
  void _sendToQueue(const char* msg); // Pushes formatted string to the queue

  // Background task that actually performs the serial writes
  static void _safeSerialTask(void *pvParameters);

public:
  SafeSerialClass(void);

  // Overloaded begin methods to handle different hardware types on ESP32-S3/C3/Classic
#if HWCDC_SERIAL_IS_DEFINED == 1
  /**
   * @brief Default begin for HWCDC.
   * @param baud Bitrate (default 115200)
   * @param port Reference to HWCDC Serial object (e.g., HWCDCSerial)
   */
  void begin(HWCDC& port, unsigned long baud = 0UL);
#elif USBCDC_SERIAL_IS_DEFINED == 1
  /**
   * @brief Default begin for USBCDC.
   * @param baud Bitrate (default 115200)
   * @param port Reference to USBCDC Serial object (e.g., USBCDCSerial)
   */
  void begin(USBCDC& port, unsigned long baud = 0UL);
#endif

#if (HWCDC_SERIAL_IS_DEFINED == 1) || (USBCDC_SERIAL_IS_DEFINED == 1)
  /**
   * @brief Default begin using the primary 'Serial' object.
   */
  void begin(unsigned long baud = 0UL);
#else
  /**
   * @brief Default begin using the primary 'Serial' object with pin remapping.
   * @param baud Bitrate (default 115200)
   * @param config Serial protocol configuration (default SERIAL_8N1)
   * @param rxPin Custom RX pin (-1 for default)
   * @param txPin Custom TX pin (-1 for default)
   */
  void begin(unsigned long baud = 0UL, uint32_t config = SERIAL_8N1, int8_t rxPin = -1, int8_t txPin = -1);
#endif

  /**
   * @brief Custom UART initialization (Serial0, Serial1, Serial2) with pin remapping.
   * @param port Reference to HardwareSerial object (e.g., Serial1)
   * @param baud Bitrate (default 115200)
   * @param config Serial protocol configuration (default SERIAL_8N1)
   * @param rxPin Custom RX pin (-1 for default)
   * @param txPin Custom TX pin (-1 for default)
   */
  void begin(HardwareSerial& port, unsigned long baud = 0UL, uint32_t config = SERIAL_8N1, int8_t rxPin = -1, int8_t txPin = -1);

  // Setters for task and buffer customization
  void setTxQueueSize(uint16_t size) { _txQueueSize = size; } // Sets number of messages buffered before dropping new ones.
  void setRxQueueSize(uint16_t size) { _rxQueueSize = size; } // Sets number of bytes buffered at read operation.
  void setTaskPriority(uint8_t priority) { _taskPriority = priority; } // Sets SafeSerial FreeRTOS task priority.
  void setStackSize(size_t stack) { _stackSize = stack; } // Sets stack (RAM) allocated for the background task.
  void setCore(BaseType_t core) { _core = core; } // Sets the CPU core assigned to the logger task.
  void setRxBufferSize(size_t size) { _rxBufferSize = size; } // Sets size of receive buffer on hardware based (UART, HWCDC) Serial ports. Set it to 0 for hardware default.
  void setTxBufferSize(size_t size) { _txBufferSize = size; } // Sets size of transmit buffer on hardware based (UART, HWCDC) Serial ports. Set it to 0 for hardware default.
  void setTxTimeoutMs(uint32_t timeout) { _txTimeoutMs = timeout; } // Sets the timeout in ms on USB (HWCDC, USBCDC) Serial ports.
  void setMaxRxBurst(uint8_t burst) { _maxRxBurst = burst; } // Sets the maximum amount of bytes to read at once to avoid blocking at receiving.

  QueueHandle_t getTxQueueHandle();
  QueueHandle_t getRxQueueHandle();

  // Implement mandatory Print methods
  size_t write(uint8_t c) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Wrapped print methods to ensure atomic logging (one call = one queue item)
  size_t print(const __FlashStringHelper *ifsh);
  size_t print(const char s[]);
  size_t print(const String &s);
  size_t print(int n, int base = DEC);
  size_t print(float f, int places = 2);
  size_t print(IPAddress ip);
  size_t print(struct tm * timeinfo, const char * format = nullptr);
  size_t print(struct tm * timeinfo, const __FlashStringHelper * format);

  size_t println(void);
  size_t println(const __FlashStringHelper *ifsh);
  size_t println(const char s[]);
  size_t println(const String &s);
  size_t println(int n, int base = DEC);
  size_t println(float f, int places = 2);
  size_t println(IPAddress ip);
  size_t println(struct tm * timeinfo, const char * format = nullptr);
  size_t println(struct tm * timeinfo, const __FlashStringHelper * format);

  int printf(const char *format, ...);
  int printf(const __FlashStringHelper *format, ...);

  // Implement mandatory read methods
  int available() override;
  int read() override;
  int peek() override;

  // Get HighWaterMark value of stack for diagnostics
  UBaseType_t getStackHighWatermark(void);

  // For diagnostics: number of the dropped messages during the task's lifetime
  uint32_t getDroppedMessages(void);
};

/**
 * @brief Universal ESP32 Asynchronous Thread-safe Serial instance.
 * Offloads serial writes to a dedicated FreeRTOS task to prevent blocking.
 * Inherits from Stream to support standard Arduino read/print methods.
 */
extern SafeSerialClass SafeSerial;

#endif // SAFESERIAL_H