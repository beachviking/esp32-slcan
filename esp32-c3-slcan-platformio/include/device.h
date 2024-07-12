// Interface to actual hardware device

#ifndef device_h_included
#define device_h_included

#include "main.h"

// Initialize the hardware.
void device_setup();

// Bring the signal on the periperal power line high, if it is currently low.  This will
// re-initialize the devices that were affected by power-off.  It is not possible to
// selectively initialize some devices.
void power_peripherals_on();

// Bring the signal on the periperal power line low UNCONDITIONALLY.  This has the effect
// of disabling the I2C devices (mic, gas, environment, and display in HW v1.0.0) as well
// as the PIR.  Note that the WAKE/BTN1 button and serial lines are not affected.
void power_peripherals_off();

// The following methods will have limited or no functionality if the peripherals have been
// powered off.  Some will report this fact on the log or on their output stream.
// Some will just do nothing.

// Go into a state where `msg` is displayed on all available surfaces and the
// device hangs.  If `is_error` is true then an additional error indication
// may be produced (eg a sound or additional message).
void enter_end_state(const char* msg, bool is_error = false) NO_RETURN;


#endif // !device_h_included
