// -------------------------------------------------------------
// esp32-slcan for esp32
// -------------------------------------------------------------
// Reworked for esp32 by beachviking
//
// Requires arduino-CAN library by Sandeep Mistry 
// https://github.com/sandeepmistry/arduino-CAN

// Inspired by https://github.com/mintynet/teensy-slcan by mintynet
//
// This example uses the CAN feather shield from SKPANG for Adafruit's ESP32 HUZZAH Feather
// http://skpang.co.uk/catalog/canbus-featherwing-for-esp32-p-1556.html

#include <CAN.h>

#define CAN_RX_PIN  GPIO_NUM_26
#define CAN_TX_PIN  GPIO_NUM_25
#define CAN_DEFAULT_SPEED 50E3

//#define hex2int(c) ( (c >= '0' && c <= '9') ? (c - '0') : ((c & 0xf) + 9) )

// Function prototypes
void send_canmsg(char *buf, boolean ext, boolean rtr);
void parse_slcancmd(char *buf);
void slcan_ack();
void slcan_nack();
void xfer_tty2can();
void xfer_can2tty();
void changeCANFilter(const char *buf, bool mask);
void changeCANSpeed(const char *buf);

// default values
boolean slcan     = true;
boolean cr        = false;
boolean timestamp = false;
long can_baudrate = CAN_DEFAULT_SPEED;
long acceptance_mask = 0x1FFFFFFF; // default: accept all. Sets AM0, AM1, AM2, AM3, SJA1000
long acceptance_id = 0x0;        // default: accept all. Sets AC0, AC1, AC2 & AC3, SJA1000

const char NEW_LINE = '\r';

// -------------------------------------------------------------

void setup()
{
  delay(1000);

  Serial.begin(115200);
  while (!Serial);

  CAN.setPins(GPIO_NUM_26,GPIO_NUM_25);
     
  Serial.println(F("esp32-slcan"));
  Serial.print(F("default speed = "));
  Serial.print(can_baudrate);
  Serial.println(F("bps"));

  parse_slcancmd("C/0");

} //setup()

// -------------------------------------------------------------

void loop()
{
  xfer_can2tty();
  xfer_tty2can();
} //loop()

//----------------------------------------------------------------

void send_canmsg(char *buf, boolean ext, boolean rtr) {
  int n;
  
  if (slcan) {
    int dlc = 0;
    int msg_id = 0;
    int value = 0;
 
    if(ext) {
      // extended id message requested
      sscanf(&buf[0], "%08x", &msg_id);      
      sscanf(&buf[8], "%01x", &dlc);

      if (dlc > 8) 
        return;
        
      CAN.beginExtendedPacket(msg_id, dlc, rtr);        
      buf += 9;
    } 
    else {
      // standard id message requested
      sscanf(&buf[0], "%03x", &msg_id);      
      sscanf(&buf[3], "%01x", &dlc);

      if (dlc > 8) 
        return;
        
      CAN.beginPacket(msg_id, dlc, rtr);                
      buf += 4;  
    }

    if(!rtr) {
      n = dlc*2;  
        
      for (unsigned i = 0; i < dlc && n >= 2; i++, buf += 2, n -= 2) {
        sscanf(&buf[0], "%02x", &value);
        CAN.write(value);
      }      
    }

    CAN.endPacket();
  }
} // send_canmsg()

// -------------------------------------------------------------

void parse_slcancmd(char *buf)
{                           // LAWICEL PROTOCOL
  switch (buf[0]) {
    case 'O':               // OPEN CAN
      slcan=true;
      CAN.begin(can_baudrate);
      slcan_ack();
      break;
    case 'C':               // CLOSE CAN
      slcan=false;
      CAN.end();
      slcan_ack();
      break;
    case 't':               // send std frame
      send_canmsg(&buf[1],false,false);
      slcan_ack();
      break;
    case 'T':               // send ext frame
      send_canmsg(&buf[1],true,false);
      slcan_ack();
      break;
    case 'r':               // send std rtr frame
      send_canmsg(&buf[1],false,true);
      slcan_ack();
      break;
    case 'R':               // send ext rtr frame
      send_canmsg(&buf[1],true,true);
      slcan_ack();
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
      Serial.println(F("snn\t=\tSpeed 0xnnk  N/A"));
      Serial.println(F("S0\t=\tSpeed 10k    N/A"));
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
      Serial.print(can_baudrate);
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
    case '0': slcan_nack(); break; // Not supported by the ESP32 hardware
    case '1': slcan_nack(); break; // Not supported by the ESP32 hardware
    case '2': can_baudrate=50E3; slcan_ack(); break;
    case '3': can_baudrate=100E3; slcan_ack(); break;
    case '4': can_baudrate=125E3; slcan_ack(); break;
    case '5': can_baudrate=250E3; slcan_ack(); break;
    case '6': can_baudrate=500E3; slcan_ack(); break;
    //case '7': can_baudrate=800E3; slcan_ack(); break;
    case '7': slcan_nack(); break;
    case '8': can_baudrate=1000E3; slcan_ack(); break;
    default: slcan_nack(); return;
  }
}

//----------------------------------------------------------------

void changeCANFilter(const char *buf, bool mask)
{
  if (slcan) { return; }

  if(mask)
    sscanf(&buf[0], "%08x", &acceptance_mask);      
  else
    sscanf(&buf[0], "%08x", &acceptance_id);      

  CAN.filterExtended(acceptance_id,acceptance_mask);
}

//----------------------------------------------------------------

void slcan_ack()
{
  Serial.write('\r');
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
  if ((ser_length = Serial.available()) > 0)
  {
    for (int i = 0; i < ser_length; i++) {
      char val = Serial.read();
      cmdbuf[cmdidx++] = val;
      if (cmdidx == 32)
      {
        slcan_nack();
        cmdidx = 0;
      } else if (val == '\r')
      {
        cmdbuf[cmdidx] = '\0';
        parse_slcancmd(cmdbuf);
        cmdidx = 0;
      }
    }
  }
} //xfer_tty2can()

// -------------------------------------------------------------

void xfer_can2tty()
{
  long time_now = 0;
  int packetSize = CAN.parsePacket();

  if(packetSize) {
    if(CAN.packetRtr()) {
      if(CAN.packetExtended()) {
        // Extended ID RTR
        Serial.printf("R%08x%d", CAN.packetId(), CAN.packetDlc());           
      } else {
        // Standard ID RTR
        Serial.printf("r%03x%d", CAN.packetId(), CAN.packetDlc());
      }
    } else {
      
      if(CAN.packetExtended()) {
        Serial.printf("T%08x%d", CAN.packetId(), CAN.packetDlc());      
      } else {
        Serial.printf("t%03x%d", CAN.packetId(), CAN.packetDlc());      
      }
    
      for(auto i = 0; i < CAN.packetDlc(); i++) {
        Serial.printf("%02x", CAN.read());
      }
    }
    
    if (timestamp) {
      time_now = millis() % 60000;
      Serial.printf("%04x", time_now);            
    }
    
    Serial.write(NEW_LINE);
    if (cr) Serial.println("");
  }
}
