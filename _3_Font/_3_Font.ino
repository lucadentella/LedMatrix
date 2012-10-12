#include "aipointe.h"

// HT1632C PINs

#define DISPLAY_CS    3
#define DISPLAY_WR    4
#define DISPLAY_DATA  5

// HT1632C Commands

#define HT1632C_READ    B00000110
#define HT1632C_WRITE   B00000101
#define HT1632C_CMD     B00000100

#define HT1632_CMD_SYSON  0x01
#define HT1632_CMD_LEDON  0x03

// Display buffer

byte buffer[32];


void setup() {

  Serial.begin(57600);

  // all PINs are output
  pinMode(DISPLAY_CS, OUTPUT);
  pinMode(DISPLAY_WR, OUTPUT);
  pinMode(DISPLAY_DATA, OUTPUT);
  
  // enable System oscillator and LED duty cycle generator
  ht1632c_send_command(HT1632_CMD_SYSON);
  ht1632c_send_command(HT1632_CMD_LEDON);
  
  // clear display
  ht1632c_clear_display();
}

void loop() {

  if (Serial.available() > 0) {
    int incomingByte = Serial.read();
    displayChar(incomingByte);
  }
}

void displayChar(int incomingByte) {
  
  // get font details
  int infoPosition = incomingByte - 33;
  unsigned char length = aipoint_info[infoPosition].length;
  unsigned int offset = aipoint_info[infoPosition].offset;
  
  // select display
  digitalWrite(DISPLAY_CS, LOW);  
 
  // send WRITE command
  ht1632c_send_bits(HT1632C_WRITE, 1 << 2);
  
  // send start address, 00h
  ht1632c_send_bits(0x00, 1 << 6);
  
  for(int i = 0; i < 32; i++) {
    if(i < length) {
      char mychar = pgm_read_byte(aipointe_font + offset + i);
      ht1632c_send_bits(mychar, 1<<7);
    }
    else ht1632c_send_bits(0x00, 1<<7);
  }
  
  // unselect display
  digitalWrite(DISPLAY_CS, HIGH);  
}


// ----------------------------------------
//  HT1632C functions
// ----------------------------------------

void ht1632c_clear_display() {
  
  digitalWrite(DISPLAY_CS, LOW);  
  ht1632c_send_bits(HT1632C_WRITE, 1 << 2);
  ht1632c_send_bits(0x00, 1 << 6);
  for(int i = 0; i < 32; i++) ht1632c_send_bits(0x00, 1<<7);
  digitalWrite(DISPLAY_CS, HIGH);  
}

void ht1632c_send_command(byte command) {
  
  digitalWrite(DISPLAY_CS, LOW);  
  ht1632c_send_bits(HT1632C_CMD, 1 << 2);
  ht1632c_send_bits(command, 1 << 7);
  ht1632c_send_bits(0, 1);
  digitalWrite(DISPLAY_CS, HIGH); 
}

void ht1632c_send_bits(byte bits, byte firstbit) {
  
  while(firstbit) {
    digitalWrite(DISPLAY_WR, LOW);
    if (bits & firstbit) digitalWrite(DISPLAY_DATA, HIGH);
    else digitalWrite(DISPLAY_DATA, LOW);
    digitalWrite(DISPLAY_WR, HIGH);
    firstbit >>= 1;
  }
}
