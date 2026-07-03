/*
 * Global build options for the MultiThread_Stress example (Arduino IDE / 
 * arduino-cli).
 *
 * The ESP32 core (mkbuildoptglobals) passes the flags in the block below to
 * EVERY translation unit of this sketch, including the SafeSerial library .cpp.
 * A plain #define in the .ino cannot do this: each library .cpp is compiled as
 * a separate translation unit and would fall back to the 256-byte default.
 *
 * PlatformIO does NOT run mkbuildoptglobals; there, set the same flag via
 * platformio.ini:  build_flags = -D SAFESERIAL_LINE_BUFFER_SIZE=128
 */

/*@create-file:build.opt@
// Lines starting with // (or * or #) are ignored by the build.opt parser.
// Raise the SafeSerial line buffer so long AT responses fit (default: 256).
-DSAFESERIAL_LINE_BUFFER_SIZE=128
*/
