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

PROGMEM byte text[] = {  
  B00111110, // C
  B01000001,
  B01000001,
  B01000001,
  B00100010,
  B00000000, // space
  B00010001, // i
  B01011111,
  B00000001,
  B00000000, // space
  B00001110, // a
  B00010001,
  B00100001,
  B00100010,
  B00011111,
  B00000000, // space  
  B00011110, // o
  B00100001,
  B00100001,
  B00100001,
  B00011110,
  B00000000, // space  
  B00111101, // !
  B01000000  
};

void setup() {
  
  Serial.begin(57600);

  // All PINs are output
  pinMode(DISPLAY_CS, OUTPUT);
  pinMode(DISPLAY_WR, OUTPUT);
  pinMode(DISPLAY_DATA, OUTPUT);
  
  // Enable System oscillator and LED duty cycle generator
  ht1632c_send_command(HT1632_CMD_SYSON);
  ht1632c_send_command(HT1632_CMD_LEDON);
  
  // select display
  digitalWrite(DISPLAY_CS, LOW);  
 
  // send WRITE command
  ht1632c_send_bits(HT1632C_WRITE, 1 << 2);
  
  // send start address, 00h
  ht1632c_send_bits(0x00, 1 << 6);

  // send data
  for(int i = 0; i < 24; i++) {
    char mychar = pgm_read_byte(text + i);
    ht1632c_send_bits(mychar, 1<<7);
  }
  for(int i = 0; i < 64; i++) {
      digitalWrite(DISPLAY_WR, LOW);
      digitalWrite(DISPLAY_DATA, LOW);
      digitalWrite(DISPLAY_WR, HIGH);
  }
  
  // unselect display
  digitalWrite(DISPLAY_CS, HIGH);  
}

void loop() {}


// ----------------------------------------
//  HT1632C functions
// ----------------------------------------

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
