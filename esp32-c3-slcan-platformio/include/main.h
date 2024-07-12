// Snappysense program configuration and base include file.

// For a general overview, see comment block in main.cpp.

#ifndef main_h_included
#define main_h_included

#include "Arduino.h"


////////////////////////////////////////////////////////////////////////////////
//
// CODING STANDARDS
//
// - All <filename>.cpp files shall have a corresponding <filename>.h and the
//   <filename>.cpp file shall include <filename>.h first.
// - All <filename>.h shall include main.h first.
// - Do not worry about out-of-memory conditions (OOM) except perhaps for large
//   allocations (several KB at least).  The Arduino libraries are not reliable
//   enough to handle OOM properly anyway.  Basically, just ignore the problem.
// - Avoid using libraries that depend on exception handling, or handle exceptions
//   close to API calls if necessary.
// - Naming conventions: functions and variables and file names are snake_case,
//   types are CamelCase, global constants are ALL_UPPER_SNAKE_CASE.
// - Indent using spaces, not tabs.  Preferably 2 spaces per level.
// - Try to conform to whatever standard is used in the file you're editing, if
//   it differs (in reasonable ways) from the above.
// - Try to limit line lengths to 100 chars (allowing two side-by-side editors
//   on laptops and three on most standalone displays)

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTIONAL CONFIGURATION

// Hardware version you're compiling for.  Pick one.  See device.cpp for more.
//#define SNAPPY_HARDWARE_1_0_0
#define SNAPPY_HARDWARE_1_1_0

// Radio configuration.  Pick one (or none).
//
// LoRaWAN support is NOT IMPLEMENTED.
#define SNAPPY_WIFI
//#define SNAPPY_LORA

// Set this to make config.cpp include development_config.h with compiled-in values
// and short time intervals for many things (to speed up testing).
// Otherwise, we're in "production" mode and default values are mostly blank and
// the device must be provisioned from interactive config mode.
//#define SNAPPY_DEVELOPMENT

// With SNAPPY_TIMESTAMPS, every sent message gets a timestamp, and no messages
// are sent until the time has been configured.  Requires SNAPPY_NTP or SNAPPY_LORA
// for the time service.
#define SNAPPY_TIMESTAMPS

/////
//
// Profile 1: WiFi
//
// Here we have NTP, MQTT, and WiFi Access Point configuration.

// With SNAPPY_NTP, synchronize the time with an ntp server at startup (only).
// See time_server.h.  This requires SNAPPY_WIFI.
#define SNAPPY_NTP

// With SNAPPY_MQTT, the device will upload readings to a predefined mqtt broker
// every so often, and download config messages and commands.  Requires SNAPPY_WIFI.
#define SNAPPY_MQTT

// SNAPPY_WEBCONFIG causes a WiFi access point to be created with an SSID
// printed on the display when the device comes up in config mode, allowing for both
// factory provisioning of ID, certificates, and so on, as well as user provisioning
// of network names and other local information.  See CONFIG.md at the root of the
// repo.  Requires SNAPPY_WIFI.
#define SNAPPY_WEBCONFIG

/////
//
// Profile 2: LoRaWAN
//
// Here we have time service and message upload via LoRa, and factory config over I2C.
// There is no user config.
//
// LoRaWAN support is NOT IMPLEMENTED.

// SNAPPY_I2CCONFIG causes the device to go into I2C slave mode on address TBD when the
// device comes up in config mode, allowing for a connected I2C master to upload config
// data by writing to port TBD.  Requires !SNAPPY_WIFI because the mode that the device
// switches into on a long button press changes.
//
// SNAPPY_I2CCONFIG is NOT IMPLEMENTED.
//#define SNAPPY_I2CCONFIG

// // Control the various sensors
// #define SENSE_TEMPERATURE
// #define SENSE_HUMIDITY
// #define SENSE_UV
// #define SENSE_LIGHT
// #define SENSE_PRESSURE
// #define SENSE_ALTITUDE
// #define SENSE_AIR_QUALITY_INDEX
// #define SENSE_TVOC
// #define SENSE_CO2
// #define SENSE_MOTION
// #define SENSE_NOISE

// ----------------------------------------------------------------------------
// The following are mostly useful during development and would not normally be
// enabled in production.

// Include the log(stream, fmt, ...) functions, see log.h.  If the serial device is
// connected then log messages will appear there, otherwise they will be discarded.
#define LOGGING

// Play a tune when starting the device
//#define STARTUP_SONG

// With SERIAL_COMMAND_SERVER, the device listens for commands on the serial line, the
// command "help" will provide a list of possible commands.
#define SERIAL_COMMAND_SERVER

// If enabled, trigger a long button press this many seconds after startup.  Useful for
// experimenting with the ESP32 config system off the PCB.
//#define SIMULATE_LONG_PRESS 3

// If enabled, trigger a short button press this many seconds after startup.  Useful for
// going into monitoring mode off the PCB.
//#define SIMULATE_SHORT_PRESS 120

// If enabled, ignore the button on the board.  Useful for experimenting with the ESP32
// and sensors off the board.
//#define DISABLE_BUTTON

#if defined(SERIAL_COMMAND_SERVER)
# define SNAPPY_COMMAND_PROCESSOR
#endif

#define WARN_UNUSED __attribute__((warn_unused_result))
#define NO_RETURN __attribute__ ((noreturn))

// Event codes for events posted to main_event_queue
enum class EvCode {
  NONE = 0,
  
  // Main task state machine (timer-driven and event-driven).  There's a
  // blurry line between these states and the external events, below,
  // but mainly, these states can cause state changes, while the events
  // below generally don't do that.
  START_CYCLE,

  // External events being handled in the main task, orthogonally to its
  // state machine.
  PERFORM,            // Interactive command, from serial listener; transfers a String object

  // Serial listener task state machine (timer-driven)
  SERIAL_SERVER_POLL,
};

struct SnappyEvent {
  SnappyEvent() : code(EvCode::NONE), pointer_data(nullptr) {}
  SnappyEvent(EvCode code) : code(code), pointer_data(nullptr) {}
  SnappyEvent(EvCode code, void* data) : code(code), pointer_data(data) { assert(data != nullptr); }
  SnappyEvent(EvCode code, uint32_t data) : code(code), scalar_data(data) {}
  EvCode code;
  union {
    void* pointer_data;
    uint32_t scalar_data;
  };
};

void put_main_event(EvCode code);
void put_main_event_from_isr(EvCode code);
void put_main_event(EvCode code, void* data);
void put_main_event(EvCode code, uint32_t payload);

#endif