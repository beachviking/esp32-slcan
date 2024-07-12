// An interactive server that reads from the serial port.

#ifndef serial_input_h_included
#define serial_input_h_included

#include "main.h"

void serial_server_init();
void serial_server_start();
void serial_server_poll();
void serial_server_stop();

#endif // !serial_input_h_included
