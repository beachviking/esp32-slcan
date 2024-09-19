// -------------------------------------------------------------
// esp32-slcan for esp32 S3 - poc code
// -------------------------------------------------------------
// by beachviking
//
// Inspired by https://github.com/mintynet/teensy-slcan by mintynet
//
// This example uses the CAN feather shield from SKPANG for a generic ESP32-S3
// module.
// http://skpang.co.uk/catalog/canbus-featherwing-for-esp32-p-1556.html


#include <Arduino.h>
#include "driver/twai.h"

#define ESP_CAN_RX GPIO_NUM_4
#define ESP_CAN_TX GPIO_NUM_5

// default values
boolean slcan     = true;
boolean cr        = false;
boolean timestamp = false;
const char NEW_LINE = '\r';

//Initialize configuration structures using macro initializers
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(ESP_CAN_TX, ESP_CAN_RX, TWAI_MODE_NORMAL);
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_10KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

// Function prototypes
bool send_canmsg(char *buf, boolean ext, boolean rtr);
void parse_slcancmd(char *buf);
void slcan_ack();
void slcan_nack();
void xfer_tty2can();
void xfer_can2tty();
void changeCANFilter(const char *buf, bool mask);
void changeCANSpeed(const char *buf);
void start_can();
void stop_can();

// -------------------------------------------------------------

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  xfer_can2tty();
  xfer_tty2can();
}

bool send_canmsg(char *buf, boolean ext, boolean rtr) {
  int n;
  twai_message_t message;  

  if (!slcan)
    return false;

  int dlc = 0;
  int msg_id = 0;
  int value = 0;

  if(ext) {
    // extended id message requested
    sscanf(&buf[0], "%08X", &msg_id);      
    sscanf(&buf[8], "%01X", &dlc);

    if (dlc > 8) 
      return false;

    message.identifier = msg_id;
    message.extd = 1;
    message.data_length_code = dlc;        
    buf += 9;
  } 
  else {
    // standard id message requested
    sscanf(&buf[0], "%03X", &msg_id);      
    sscanf(&buf[3], "%01X", &dlc);

    if (dlc > 8) 
      return false;
                     
    message.identifier = msg_id;
    message.extd = 0;
    message.data_length_code = dlc;        
    buf += 4;  
  }

  if(!rtr) {
    n = dlc*2;  
      
    for (unsigned i = 0; i < dlc && n >= 2; i++, buf += 2, n -= 2) {
      sscanf(&buf[0], "%02X", &value);
      message.data[i] = value; 
    }      
  }

  message.ss = true;

  //Queue message for transmission
  return(twai_transmit(&message, 0) == ESP_OK);
} // send_canmsg()


// -------------------------------------------------------------

void parse_slcancmd(char *buf)
{                           // LAWICEL PROTOCOL
  switch (buf[0]) {
    case 'O':               // OPEN CAN
      slcan=true;
      start_can();
      // CAN.begin(can_baudrate);
      slcan_ack();
      break;
    case 'C':               // CLOSE CAN
      slcan=false;
      stop_can();
      // CAN.end();
      slcan_ack();
      break;
    case 't':               // send std frame
      if (send_canmsg(&buf[1],false,false))
        slcan_ack();
      else
        slcan_nack();
      break;
    case 'T':               // send ext frame
      if(send_canmsg(&buf[1],true,false))
        slcan_ack();
      else
        slcan_nack();
      break;
    case 'r':               // send std rtr frame
      if(send_canmsg(&buf[1],false,true))
        slcan_ack();
      else
        slcan_nack();
      break;
    case 'R':               // send ext rtr frame
      if(send_canmsg(&buf[1],true,true))
        slcan_ack();
      else
        slcan_nack();
      break;
    case 'Z':               // ENABLE TIMESTAMPS
      switch (buf[1]) {
        case '0':           // TIMESTAMP OFF  
          timestamp = false;
          slcan_ack();
          break;
        case '1':           // TIMESTAMP ON
          timestamp = true;
          slcan_ack();
          break;
        default:
          break;
      }
      break;
    case 'M':               ///set ACCEPTANCE CODE ACn REG
      changeCANFilter(&buf[1], false);
      slcan_ack();
      break;
    case 'm':               // set ACCEPTANCE MASK AMn REG
      changeCANFilter(&buf[1], true);
      slcan_ack();
      break;
    case 'S':               // CAN bit-rate      
      changeCANSpeed(&buf[1]);
      break;
    case 'F':               // STATUS FLAGS SJA1000, TBD
      //Serial.print("F00");
      //CAN.dumpRegisters(Serial);
      slcan_ack();
      break;
    case 'V':               // VERSION NUMBER
      Serial.print("V1");
      slcan_ack();
      break;
    case 'N':               // SERIAL NUMBER
      Serial.print("N2208");
      slcan_ack();
      break;
   case 'l':               // (NOT SPEC) TOGGLE LINE FEED ON SERIAL
      cr = !cr;
      slcan_nack();
      break;
    case 'h':               // (NOT SPEC) HELP SERIAL
      Serial.println();
      Serial.println(F("esp32-slcan"));
      Serial.println();
      Serial.println(F("O\t=\tStart slcan"));
      Serial.println(F("C\t=\tStop slcan"));
      Serial.println(F("t\t=\tSend std frame"));
      Serial.println(F("r\t=\tSend std rtr frame"));
      Serial.println(F("T\t=\tSend ext frame"));
      Serial.println(F("R\t=\tSend ext rtr frame"));
      Serial.println(F("Z0\t=\tTimestamp Off"));
      Serial.print(F("Z1\t=\tTimestamp On"));
      if (timestamp) Serial.print(F("  ON"));
      Serial.println();
      Serial.println(F("S0\t=\tSpeed 10k"));
      Serial.println(F("S1\t=\tSpeed 25k"));
      Serial.println(F("S2\t=\tSpeed 50k"));
      Serial.println(F("S3\t=\tSpeed 100k"));
      Serial.println(F("S4\t=\tSpeed 125k"));
      Serial.println(F("S5\t=\tSpeed 250k"));
      Serial.println(F("S6\t=\tSpeed 500k"));
      Serial.println(F("S7\t=\tSpeed 800k"));
      Serial.println(F("S8\t=\tSpeed 1000k"));
      Serial.println(F("F\t=\tFlags        N/A"));
      Serial.println(F("N\t=\tSerial No"));
      Serial.println(F("V\t=\tVersion"));
      Serial.println(F("-----NOT SPEC-----"));
      Serial.println(F("h\t=\tHelp"));
      Serial.print(F("l\t=\tToggle CR "));
      if (cr) {
        Serial.println(F("ON"));
      } else {
        Serial.println(F("OFF"));
      }
      Serial.print(F("CAN_SPEED:\t"));
      // Serial.print(t_config.brp);
      Serial.print(F("bps"));
      if (timestamp) {
        Serial.print(F("\tT"));
      }
      if (slcan) {
        Serial.print(F("\tON"));
      } else {
        Serial.print(F("\tOFF"));
      }
      Serial.println();
      slcan_nack();
      break;      
    default:
      slcan_nack();
      break;
  }
} // parse_slcancmd()

//----------------------------------------------------------------

void changeCANSpeed(const char *buf)
{
  if (slcan) { return; }
  
  switch (buf[0])
  {
    case '0': t_config = TWAI_TIMING_CONFIG_10KBITS(); slcan_ack(); break;
    case '1': t_config = TWAI_TIMING_CONFIG_25KBITS(); slcan_ack(); break; 
    case '2': t_config = TWAI_TIMING_CONFIG_50KBITS(); slcan_ack(); break;
    case '3': t_config = TWAI_TIMING_CONFIG_100KBITS(); slcan_ack(); break;
    case '4': t_config = TWAI_TIMING_CONFIG_125KBITS(); slcan_ack(); break;
    case '5': t_config = TWAI_TIMING_CONFIG_250KBITS(); slcan_ack(); break;
    case '6': t_config = TWAI_TIMING_CONFIG_500KBITS(); slcan_ack(); break;
    case '7': t_config = TWAI_TIMING_CONFIG_800KBITS(); slcan_ack(); break;
    case '8': t_config = TWAI_TIMING_CONFIG_1MBITS(); slcan_ack(); break;
    default: slcan_nack(); return;
  }
}

//----------------------------------------------------------------

void changeCANFilter(const char *buf, bool mask)
{
  if (slcan) { return; }

  if(mask)
    sscanf(&buf[0], "%08X", &f_config.acceptance_mask);      
  else
    sscanf(&buf[0], "%08X", &f_config.acceptance_code);      
}

//----------------------------------------------------------------

void slcan_ack()
{
  Serial.write("Z\r",2);
} // slcan_ack()

//----------------------------------------------------------------

void slcan_nack()
{
  Serial.write('\a');
} // slcan_nack()

// -------------------------------------------------------------

void xfer_tty2can()
{
  int ser_length;
  static char cmdbuf[32];
  static int cmdidx = 0;

  if ((ser_length = Serial.available()) <= 0)
    return;

  for (int i = 0; i < ser_length; i++) {
    char val = Serial.read();
    cmdbuf[cmdidx++] = val;

    if (cmdidx == 32)
    {
      slcan_nack();
      cmdidx = 0;
      continue;
    }
    
    if (val == '\r')
    {
      cmdbuf[cmdidx] = '\0';
      parse_slcancmd(cmdbuf);
      cmdidx = 0;
    }
  }
} //xfer_tty2can()

// -------------------------------------------------------------

void xfer_can2tty()
{
  long time_now = 0;
  // int packetSize = CAN.parsePacket();
  twai_message_t message;

  if (twai_receive(&message, pdMS_TO_TICKS(10000)) != ESP_OK)
    return;

  if(message.rtr) {
      if(message.extd) {
        // Extended ID RTR
        Serial.printf("R%08x%d", message.identifier, message.data_length_code);           
      } else {
        // Standard ID RTR
        Serial.printf("r%03x%d", message.identifier, message.data_length_code);
      }
  } else {      
    if(message.extd) {
      Serial.printf("T%08x%d", message.identifier, message.data_length_code);      
    } else {
      Serial.printf("t%03x%d", message.identifier, message.data_length_code);      
    }
  
    for(auto i = 0; i < message.data_length_code; i++) {
      Serial.printf("%02X", message.data[i]);
    }
  }
    
  if (timestamp) {
    time_now = millis() % 60000;
    Serial.printf("%04x", time_now);            
  }
    
  Serial.write(NEW_LINE);
  if (cr) Serial.println("");

}

void start_can() {
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

void stop_can() {
  //Stop the TWAI driver
  if (twai_stop() == ESP_OK) {
      printf("Driver stopped\n");
  } else {
      printf("Failed to stop driver\n");
      return;
  }

  //Uninstall the TWAI driver
  if (twai_driver_uninstall() == ESP_OK) {
      printf("Driver uninstalled\n");
  } else {
      printf("Failed to uninstall driver\n");
      return;
  }  
}
