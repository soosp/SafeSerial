# SafeSerial for ESP32 Arduino Core

**Asynchronous, Thread-safe and Non-blocking Serial Library (with HWCDC and
USBCDC support) for ESP32 Arduino Core.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Purpose

SafeSerial masks hardware and timing limitations through an asynchronous
queuing mechanism. It ensures that critical code paths (e.g., network
callbacks) never stall due to relatively slow serial port operations.
Supported port types: UART, HWCDC and USBCDC. SafeSerial (with the exception
of a few hardware-specific settings) standardizes and abstracts the serial
interfaces used, allowing them to be used in a uniform manner.

## How it Works

The library leverages FreeRTOS capabilities to decouple the application logic
from the serial peripheral:

1. **Queue Management**: Every message to be sent is placed into a fixed-size
FIFO (First-In-First-Out) queue. This is thread-safe and executes nearly
instantaneously. The bytes to be read also arrive via a FIFO queue.
2. **Background Task**: A dedicated task (using `xTaskCreateUniversal`)
monitors the queues and transmits data when resources are available.

## Installation

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
    soosp/SafeSerial @ ^1.1.0
```

### Arduino IDE

Search for **SafeSerial** in the Library Manager (Sketch → Include Library →
Manage Libraries).

## Quick Start

```cpp
#include <SafeSerial.h>

void setup() {
  // Silence internal ESP-IDF logs that bypass our queue logic (optional)
  esp_log_level_set("*", ESP_LOG_NONE);

  // Start SafeSerial (automatically detects port type)
  SafeSerial.begin(115200); 
}

void loop() {
  SafeSerial.println("This won't block your code!");
  vTaskDelay(pdMS_TO_TICKS(1000));
}
```

## Usage

SafeSerial inherits from the Arduino `Stream` class, making it a drop-in
replacement for standard `Serial` calls.

### Basic printing functions

```cpp
SafeSerial.println("System initialized.");
SafeSerial.printf("IP Address: %s", WiFi.localIP().toString().c_str());
SafeSerial.printf("Sensor value: %d", analogRead(34));
```

### Reading a byte from the Serial port

```cpp
if (SafeSerial.available()) {
  uint8_t c = SafeSerial.read();
}
```

### Reading a line from the Serial port

```cpp
char buf[SAFESERIAL_LINE_BUFFER_SIZE];
// Fill buffer with zeros
memset(buf, 0, sizeof(buf));

if (SafeSerial.available()) {
  // readBytesUntil reads until '\n' or buffer is full
  // This cuts the message to the appropriate line length so that the zero
  // at the end remains
  SafeSerial.readBytesUntil('\n', buf, sizeof(buf) - 1);
}
```

⚠️ **IMPORTANT: Once initialized, replace all standard Serial calls with
SafeSerial. Avoid using `Serial.*` directly to prevent blocking and resource conflicts.**

## Advanced Usage: Custom Ports & Pins

By default SafeSerial use `Serial`. If you need to use an another UART or need
to remap UART pins:

```cpp
#include <SafeSerial.h>

// Create a new instance using SafeSerialClass
// This is needed if you want a custom name or multiple concurrent instances
// for multiple physical ports
SafeSerialClass MyLogger; 

void setup() {
  // Turn off ESP-IDF logs
  esp_log_level_set("*", ESP_LOG_NONE);

  // Set the parameters BEFORE initialitation
  MyLogger.setTxBufferSize(1024); // Transmit buffer size for UART
  MyLogger.setTxQueueSize(50);    // SafeSerial queue depth
  
  // Using Serial1 with custom pins (RX: 12, TX: 15)
  // This remains non-blocking even if the UART buffer gets full.
  MyLogger.begin(Serial1, 115200, SERIAL_8N1, 12, 15);
  
  MyLogger.println("MyLogger is now active on Serial1 (Pins 12/15)");
}
```

## Configuration

### Setter functions

Instead of hardcoded macros, SafeSerial uses setter functions to allow dynamic
configuration **before** initialization:

|Method|Default|Description|
|:---|:---|:---|
|`setTxQueueSize(uint16_t)`|30|Number of messages buffered before dropping new ones.|
|`setRxQueueSize(uint16_t)`|1024|Size of the RX queue in bytes|
|`setTaskPriority(uint8_t)`|5|FreeRTOS task priority.|
|`setStackSize(size_t)`|4096|RAM allocated for the background task.|
|`setCore(BaseType_t)`|1|CPU core assigned to the logger. Has no effect on single core chips (e.g., on ESP32-C3).|
|`setRxBufferSize(size_t)`|0|Sets size of receive buffer for hardware based (UART, HWCDC) Serial ports. The default value of `0` means, that the hardware's default value is used. Has no effect in case of USBCDC.|
|`setTxBufferSize(size_t)`|0|Sets size of transmit buffer for hardware based (UART, HWCDC) Serial ports. The default value of `0` means, that the hardware's default value is used. Has no effect in case of USBCDC.|
|`setTxTimeoutMs(uint32_t)`|100|Sets the timeout in ms on USB (HWCDC, USBCDC) Serial ports. Has no effect in the case of UART.|
|`setMaxRxBurst(uint8_t)`|64|Sets the maximum amount of bytes to read at once to avoid blocking at receiving.|

### Port type autodetect

If you use the default `Serial` and want to autodetect the port type, simply
use the single-parameter `begin`:

```cpp
SafeSerial.begin(115200);
```

### Parameters of the `begin` function for UARTs

```cpp
SafeSerial.begin(port, baud, config, rxPin, txPin);
```

|Parameter|Type|Description|
|:---|:---|:---|
|`port`|`HardwareSerial&`|The UART instance (Serial0, Serial1, Serial2).|
|`baud`|`unsigned long`|Communication speed.|
|`config`|`uint32_t`|Data bits, parity, and stop bits (default: SERIAL_8N1).|
|`rxPin`|`int8_t`|Custom GPIO for Receive (default: -1).|
|`txPin`|`int8_t`|Custom GPIO for Transmit (default: -1).|

For compatibility reasons, on original ESP32 chips there is available a `begin`
function uses UART0 (`Serial0`) as `Serial`:

```cpp
SafeSerial.begin(baud, config, rxPin, txPin);
```

### Parameters of the `begin` function for HWCDC

```cpp
SafeSerial.begin(port, baud);
```

|Parameter|Type|Description|
|:---|:---|:---|
|`port`|`HWCDC&`|ESP32-S3 HWCDC (default: Serial).|
|`baud`|`unsigned long`|Communication speed.|

### Parameters of the `begin` function for USBCDC

```cpp
SafeSerial.begin(port, baud);
```

|Parameter|Type|Description|
|:---|:---|:---|
|`port`|`USBCDC&`|ESP32-S3 USBCDC (default: Serial).|
|`baud`|`unsigned long`|Communication speed.|

### Recommendations for High-Load Scenarios

If your application generates a high volume of logs:

- **Match Queue to Bursts**: Set setTxQueueSize() to the maximum number of
messages expected in a single "burst" (e.g., if 10 tasks log simultaneously,
use a queue of at least 20).
- **Task Delays**: Ensure your logging tasks have at least a vTaskDelay(1)
to allow the FreeRTOS IDLE task and the Serial stack to breathe.
- **Keep it Short**: Use shorter log lines (see `SAFESERIAL_LINE_BUFFER_SIZE`)
to reduce the amount of data the serial port must process per transaction.

## Memory Management: Why `SAFESERIAL_LINE_BUFFER_SIZE`?

The maximum size of a single log line (including the trailing zero) is defined
as a compile-time constant (`#define`). While dynamic sizing (runtime) might
seem more flexible, SafeSerial prioritizes system stability.

### The Function

This constant determines the size of the static buffers used during string
formatting (printf) and message processing in the background task.

### Why Static over Dynamic?

- Deterministic RAM Usage: By using a fixed-size buffer, the memory is
allocated at the start. You will never face a "surprise" out-of-memory crash
after 48 hours of operation because of a long log line.
- Heap Fragmentation Prevention: In embedded systems, frequently using new,
malloc, or String operations can "punch holes" in your RAM (fragmentation).
Over time, this makes it impossible to allocate even small blocks of memory,
leading to crashes. SafeSerial stays purely on the Stack, which is faster
and safer.
- Performance: Allocating a buffer on the stack is nearly instantaneous (just
moving a pointer), whereas heap allocation requires searching for a free block
of memory.

### How to Change It

If you need longer log lines (e.g., for JSON exports), do not look for a setter
function. Instead, define it in your build environment.

#### In PlatformIO (platformio.ini)

```Ini, TOML
build_flags = 
  -D SAFESERIAL_LINE_BUFFER_SIZE=512  ; Default is 256
```

#### In Arduino IDE

A sketch-level `#define` does **not** work here: every library `.cpp` is
compiled as a separate translation unit and never sees a macro defined in your
`.ino`. Instead, create a file named `<YourSketch>.ino.globals.h` next to the
sketch, containing an ESP32 build-options block:

```cpp
/*@create-file:build.opt@
-DSAFESERIAL_LINE_BUFFER_SIZE=512
*/
```

The ESP32 core applies these flags to *every* translation unit, including the
library itself. The equivalent from the command line is:

```sh
arduino-cli compile \
  --build-property build.extra_flags=-DSAFESERIAL_LINE_BUFFER_SIZE=512 ...
```

> **Heads-up:** a `*.ino.globals.h` file placed in a sketch/example folder
> hides that folder from the Arduino IDE Examples menu (the `.ino` in the
> file name confuses sketch discovery). Use it in your own sketch folders,
> but do not ship it inside a published example — document the flag instead.

⚠️ **Pro Tip**: If you increase this value significantly (e.g., to 1024),
remember to also increase the task stack size using `setStackSize()`.
The line is formatted (via `vsnprintf`) on the stack of the *calling* task,
and a copy is processed on the SafeSerial task's own stack. Both must stay
larger than the maximum line length plus some overhead (use `setStackSize()`
for the SafeSerial task; size your own tasks accordingly).

## Behavior Without an Active Terminal

How a serial port behaves when data is written while **no terminal is
listening** depends on the port type:

- **HW-CDC** (native USB Serial/JTAG): with no terminal open, the hardware TX
  FIFO fills and each write **blocks** (up to the TX timeout, ~100 ms by
  default). On a plain `Serial` this stalls the whole program.
- **USB-CDC** (TinyUSB stack): non-blocking. With no DTR connection the driver
  discards the bytes and execution continues.
- **HW-UART** (real UART / USB-to-UART bridge): non-blocking. The UART pushes
  bytes out of the TX pin regardless of whether anyone listens; there is no
  feedback loop.

Because SafeSerial performs the actual `print()`/`flush()` inside its
background task (never on the caller), your application never blocks in any of
these cases — `println()`/`printf()` only enqueue. However, on a headless
HW-CDC port the background task itself would still block on `flush()`,
congesting the TX queue and stalling RX servicing on that instance.

To prevent this, the background task checks the port before writing: if no
terminal/host is connected it discards the message instead of blocking. UART
ports have no host-presence signal and are always treated as connected. You
can query the state yourself with `isConnected()` (see Diagnostics), e.g. to
skip verbose logging while headless.

> **Note:** HW-CDC/USB-CDC connection detection relies on the port's boolean
> operator (DTR / CDC line state). Its reliability is core-version dependent
> (some early ESP32 Arduino cores always reported "connected"), and some
> terminals do not assert DTR.

## Diagnostics

### Stack size

To test the stack usage use the `UBaseType_t getStackHighWatermark(void)`
function. It returns the lowest value of free stack size of SafeSerial task
since it started:

```cpp
SafeSerial.printf(F("SafeSerial Stack HighWatermark: %u bytes\n"), SafeSerial.getStackHighWatermark());
```

Due to static memory usage, the value remains constant after the task fully
starts.

### Dropped messages

There is a getter function `uint32_t getDroppedMessages(void)` to check the
number of dropped messages during the task's lifetime:

```cpp
SafeSerial.printf(F("Dropped messages: %u\n"), SafeSerial.getDroppedMessages());
```

The result is 32-bit to preserve the atomic nature of the internal counter on
the ESP32. Its maximum value is UINT32_MAX (4,294,967,295). That’s more than
enough for most purposes. When it reaches this value, it stops incrementing;
it does not reset to 0.

### Connection state

`bool isConnected(void)` reports whether a terminal/host is currently present
(always `true` on UART ports). Messages dropped **specifically** because no
terminal was connected are counted both in `getDroppedMessages()` (the total)
and, separately, in `getSkippedWhileDisconnected()`:

```cpp
SafeSerial.printf(F("Skipped while disconnected: %u\n"),
                  SafeSerial.getSkippedWhileDisconnected());
```

Subtracting this from `getDroppedMessages()` isolates the drops caused by
genuine queue overflow (high load) — the value to watch when tuning
`setTxQueueSize()`.

## Limitations

- **Memory Footprint**: By default, it uses \~8.5 kB for the queues (\~7.5 kB
for Tx, \~1 kB for Rx) plus 4 kB for the task stack per instance
(conservative default; typically \~1-2 kB is enough, verifiable with
`getStackHighWatermark()`).
- **Dropping Logs**: If the queue is full (e.g., during a massive flood
of logs), new messages will be dropped to prevent the main application
from hanging.
- **Headless HW-CDC**: When no terminal is connected, messages are dropped
rather than blocking the background task, and counted via
`getSkippedWhileDisconnected()`.
- **Throughput Limit**: Dropped messages depend not only on queue depth and
message frequency, but also on line length and baud rate — the serial interface
transmits one character at a time, so longer lines at lower baud rates increase
the risk of queue overflow.

## License

**MIT** - Free for use in both personal and commercial projects.

**Notice**: Portions of this software were generated using AI language models.
This code is provided "as is".

## Authors

Created in 2026 by **Péter Soós**.
