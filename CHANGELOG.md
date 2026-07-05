# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Changed

- Update title of README.md

### Added

- Add Changelog links as per [Keep a Changelog](https://keepachangelog.com)
- `isConnected()` and the background-task connection gate: on a headless
  HW-CDC/USB-CDC port messages are now discarded instead of blocking the task
  on `flush()` (avoids TX-queue congestion and RX stalls while disconnected).
- `getSkippedWhileDisconnected()` counter for messages dropped due to a
  missing terminal (also included in `getDroppedMessages()`).

### Fixed

- Corrected `SAFESERIAL_LINE_BUFFER_SIZE` configuration docs: a sketch-level
  `#define` has no effect in Arduino IDE, since each library `.cpp` is a
  separate translation unit. Documented the `*.ino.globals.h` build-options
  method and fixed the PlatformIO build-flag syntax (`-D NAME=VALUE`).
- Reworked examples that relied on the ineffective sketch `#define`
  (Basic, API_Reference, MultiThread_Stress, RxTx_Line).
- Documented raising `SAFESERIAL_LINE_BUFFER_SIZE` via a global build flag for
  long AT responses in the `Modem_A76xx` example. No `*.ino.globals.h` is
  shipped, since such a file hides the example from the Arduino IDE Examples
  menu. Also documented in MultiThread_Stress.ino.

## [1.0.3] - 2026-05-04

### Changed

- More accurate title for package
- Line legth in text documents have been reduced to a maximum of 80 characters

## [1.0.2] - 2026-04-30

### Fixed

- Markdown documentation fixes

### Changed

- Documentation update about installing library

## [1.0.1] - 2026-04-30

### Fixed

- Fixed comments in the code
- Fixed examples

## [1.0.0] - 2026-04-29

### Added

- First public release

[unreleased]: https://github.com/soosp/SafeSerial/compare/1.0.3...HEAD
[1.0.3]: https://github.com/soosp/SafeSerial/compare/1.0.2...1.0.3
[1.0.2]: https://github.com/soosp/SafeSerial/compare/1.0.1...1.0.2
[1.0.1]: https://github.com/soosp/SafeSerial/compare/1.0.0...1.0.1
[1.0.0]: https://github.com/soosp/SafeSerial/releases/tag/1.0.0
