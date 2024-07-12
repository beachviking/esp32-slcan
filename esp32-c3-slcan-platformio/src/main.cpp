// -------------------------------------------------------------
// esp32-slcan for esp32 C3 - poc code
// -------------------------------------------------------------
// by beachviking
//
// Inspired by https://github.com/mintynet/teensy-slcan by mintynet
//
// This example uses the CAN feather shield from SKPANG for a generic ESP32-C3
// module.
// http://skpang.co.uk/catalog/canbus-featherwing-for-esp32-p-1556.html


#include <Arduino.h>
#include "driver/twai.h"

#define ESP_CAN_RX GPIO_NUM_3
#define ESP_CAN_TX GPIO_NUM_2

void setup() {
  // put your setup code here, to run once:

    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(ESP_CAN_TX, ESP_CAN_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_10KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }
}

void loop() {
  // put your main code here, to run repeatedly:
//Wait for message to be received
  twai_message_t message;
  if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
    printf("Message received\n");
  } else {
    //printf("Failed to receive message\n");
    return;
  }

  //Process received message
  if (message.extd) {
    printf("Message is in Extended Format\n");
  } else {
    printf("Message is in Standard Format\n");
  }
  printf("ID is %d\n", message.identifier);
  if (!(message.rtr)) {
    for (int i = 0; i < message.data_length_code; i++) {
        printf("Data byte %d = %d\n", i, message.data[i]);
    }
  }  
}