/*
 * SafeSerial Library for ESP32 Series (Universal Asynchronous Serial port handler)
 * Created: 2026, Péter Soós in collaboration with Google Gemini and Claude
 * License: MIT
 */

#include "SafeSerial.h"

SafeSerialClass::SafeSerialClass() :
  _stream(&Serial),
  _txQueueSize(30),    // Number of queue lines for TX
  _rxQueueSize(1024),  // Size of RX queue in bytes
  _taskPriority(5),    // Medium priority: safe for Wi-Fi/System tasks
  _taskHandle(NULL),
  _stackSize(4096),    // Medium sized stack to safely handle vsnprintf
  _core(1),            // Recommended core on dual-core ESP32
  _rxBufferSize(0),    // The default value of 0 means, that the hardware's default value is used.
  _txBufferSize(0),    // The default value of 0 means, that the hardware's default value is used.
  _txTimeoutMs(100),
  _maxRxBurst(64),
  _droppedMessages(0)
{
  // Allocate memory for the TX queue buffer (SAFESERIAL_LINE_BUFFER_SIZE bytes per element)
  _txQueue = xQueueCreate(_txQueueSize, SAFESERIAL_LINE_BUFFER_SIZE);

  // Allocate memory for the RX queue (one byte per element)
  _rxQueue = xQueueCreate(_rxQueueSize, sizeof(uint8_t));
}

void SafeSerialClass::_beginTask() {
  // Use Universal task creation for ESP32/S3/C3 compatibility
  xTaskCreateUniversal(_safeSerialTask, "SafeSerialTask", _stackSize, this, _taskPriority, &_taskHandle, _core);
}

QueueHandle_t SafeSerialClass::getTxQueueHandle() {
  return _txQueue;
}

QueueHandle_t SafeSerialClass::getRxQueueHandle() {
  return _rxQueue;
}

#if HWCDC_SERIAL_IS_DEFINED == 1
void SafeSerialClass::begin(HWCDC& port, unsigned long baud) {
  _stream = &port;
  port.setRxBufferSize(_rxBufferSize);
  port.setTxBufferSize(_txBufferSize);
  port.begin(baud);
  port.setTxTimeoutMs(_txTimeoutMs);

  // Handshake loop: wait for the USB host terminal to connect
  uint32_t start = millis();
  while (!port && (millis() - start < 1000)) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  _beginTask();
  _stream->print("\r\n");
}
#endif

#if USBCDC_SERIAL_IS_DEFINED == 1
void SafeSerialClass::begin(USBCDC& port, unsigned long baud) {
  _stream = &port;
  port.begin(baud);
  port.setTxTimeoutMs(_txTimeoutMs);

  uint32_t start = millis();
  while (!port && (millis() - start < 1000)) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  _beginTask();
  _stream->print("\r\n");
}
#endif

#if (HWCDC_SERIAL_IS_DEFINED == 1) || (USBCDC_SERIAL_IS_DEFINED == 1)
void SafeSerialClass::begin(unsigned long baud) {
  // Indirect call to ensure compiler chooses the correct 'begin' overload
  this->begin(Serial, baud);
}
#else
void SafeSerialClass::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin) {
  // Standard ESP32 UART redirection
  this->begin(Serial, baud, config, rxPin, txPin);
}
#endif

void SafeSerialClass::begin(HardwareSerial& port, unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin) {
  _stream = &port;
  port.setRxBufferSize(_rxBufferSize);
  port.setTxBufferSize(_txBufferSize);
  port.begin(baud, config, rxPin, txPin);
  _beginTask();
}

size_t SafeSerialClass::write(uint8_t c) {
  char buf[2] = {(char)c, 0};
  _sendToQueue(buf);
  return 1;
}

size_t SafeSerialClass::write(const uint8_t *buffer, size_t size) {
  if (size == 0) return 0;
  char buf[SAFESERIAL_LINE_BUFFER_SIZE];
  // Crop message to match log line limit
  size_t len = min(size, (size_t)(SAFESERIAL_LINE_BUFFER_SIZE - 1));
  memcpy(buf, buffer, len);
  buf[len] = '\0';
  _sendToQueue(buf);
  return len;
}

void SafeSerialClass::_sendToQueue(const char* msg) {
  if (_txQueue == NULL) return;
  
  char buf[SAFESERIAL_LINE_BUFFER_SIZE];
  memset(buf, 0, sizeof(buf)); // Zero-fill the buffer to ensure no stale data remains
  strncpy(buf, msg, sizeof(buf) - 1); // Copy only as many bytes as available, leaving the rest zeroed
  
  if (xQueueSend(_txQueue, buf, 0) != pdTRUE) {
    // Increment saturating counter — stops at UINT32_MAX instead of wrapping around
    if (__atomic_load_n(&_droppedMessages, __ATOMIC_RELAXED) < UINT32_MAX) {
      __atomic_fetch_add(&_droppedMessages, 1, __ATOMIC_RELAXED);
    }
  }
}

// Atomic wrappers: All print calls are funneled through vsnprintf to prevent
// multiple tasks from mixing characters within the same log line.
size_t SafeSerialClass::print(const __FlashStringHelper *ifsh) { return printf(F("%s"), (const char *)ifsh); }
size_t SafeSerialClass::print(const char s[]) { return printf(F("%s"), s); }
size_t SafeSerialClass::print(const String &s) { return printf(F("%s"), s.c_str()); }
size_t SafeSerialClass::print(int n, int base) { return printf(base == HEX ? F("%x") : F("%d"), n); }
size_t SafeSerialClass::print(float f, int places) { return printf(F("%.*f"), places, f); }
size_t SafeSerialClass::print(IPAddress ip) { return printf(F("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]); }

size_t SafeSerialClass::print(struct tm * timeinfo, const char * format) {
    char buf[64];
    size_t s = strftime(buf, sizeof(buf), (format == nullptr) ? "%c" : format, timeinfo);
    if (s > 0) {
        return printf("%s", buf);
    }
    return 0;
}

size_t SafeSerialClass::print(struct tm * timeinfo, const __FlashStringHelper * format) {
    return print(timeinfo, (const char *)format);
}

size_t SafeSerialClass::println(void) { return print(F("\n")); }
size_t SafeSerialClass::println(const __FlashStringHelper *ifsh) { return printf(F("%s\n"), (const char *)ifsh); }
size_t SafeSerialClass::println(const char s[]) { return printf(F("%s\n"), s); }
size_t SafeSerialClass::println(const String &s) { return printf(F("%s\n"), s.c_str()); }
size_t SafeSerialClass::println(int n, int base) { return printf(base == HEX ? F("%x\n") : F("%d\n"), n); }
size_t SafeSerialClass::println(float f, int places) { return printf(F("%.*f\n"), places, f); }
size_t SafeSerialClass::println(IPAddress ip) { return printf(F("%d.%d.%d.%d\n"), ip[0], ip[1], ip[2], ip[3]); }

size_t SafeSerialClass::println(struct tm * timeinfo, const char * format) {
    char buf[64];
    size_t s = strftime(buf, sizeof(buf), (format == nullptr) ? "%c" : format, timeinfo);
    if (s > 0) {
        return printf("%s\n", buf);
    }
    return 0;
}

size_t SafeSerialClass::println(struct tm * timeinfo, const __FlashStringHelper * format) {
    return println(timeinfo, (const char *)format);
}

int SafeSerialClass::printf(const char *format, ...) {
  char buf[SAFESERIAL_LINE_BUFFER_SIZE];
  va_list arg;
  va_start(arg, format);
  int len = vsnprintf(buf, sizeof(buf), format, arg);
  va_end(arg);
  if (len > 0) _sendToQueue(buf);
  return len;
}

int SafeSerialClass::printf(const __FlashStringHelper *format, ...) {
  char buf[SAFESERIAL_LINE_BUFFER_SIZE];
  va_list arg;
  va_start(arg, format);
  int len = vsnprintf(buf, sizeof(buf), (const char *)format, arg);
  va_end(arg);
  if (len > 0) _sendToQueue(buf);
  return len;
}

int SafeSerialClass::available() {
  return uxQueueMessagesWaiting(_rxQueue);
}

int SafeSerialClass::read() {
    uint8_t c;
    if (xQueueReceive(_rxQueue, &c, 0)) return c;
    return -1;
}

int SafeSerialClass::peek() {
    uint8_t c;
    if (xQueuePeek(_rxQueue, &c, 0)) return c;
    return -1;
}

void SafeSerialClass::_safeSerialTask(void *pvParameters) {
  SafeSerialClass *instance = (SafeSerialClass *)pvParameters;
  uint8_t rxByte;
  char txBuf[SAFESERIAL_LINE_BUFFER_SIZE];

  // Let hardware settle
  vTaskDelay(pdMS_TO_TICKS(100));
  instance->_stream->flush();

  for (;;) {
    uint8_t rxBurst = 0;
    while (instance->_stream->available() && rxBurst < instance->_maxRxBurst) {
      rxByte = instance->_stream->read();
      xQueueSend(instance->_rxQueue, &rxByte, 0);
      rxBurst++;
    }

    if (xQueueReceive(instance->_txQueue, txBuf, pdMS_TO_TICKS(1))) {
      if (instance->_stream) {
        instance->_stream->print(txBuf);
        instance->_stream->flush();
        vTaskDelay(1);
      }
    }
    vTaskDelay(1);
  }
}

// For diagnostics: the minimum stack size of the task since it started
UBaseType_t SafeSerialClass::getStackHighWatermark(void) {
  return (_taskHandle == NULL) ? 0 : uxTaskGetStackHighWaterMark(_taskHandle);
}

// For diagnostics: number of the dropped messages during the task's lifetime
uint32_t SafeSerialClass::getDroppedMessages(void) {
  return __atomic_load_n(&_droppedMessages, __ATOMIC_RELAXED);
}

// SafeSerial global instance definition
SafeSerialClass SafeSerial;