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

#define SPACING_SIZE      1      // space between letters
#define SPACE_SIZE        3      // "space" character length
#define BLANK_SIZE        7      // blank columns before restarting the string
#define SCROLLING_SPEED   150    // scrolling speed in ms

#define TEXT_BUFFER_SIZE  100    // max characters in string

// FSM states

#define S_OFF             0
#define S_LETTER          1
#define S_SPACING         2
#define S_SPACE           3
#define S_BLANK           4

byte display_buffer[32];
char text_buffer[TEXT_BUFFER_SIZE];
char display_string[TEXT_BUFFER_SIZE];

byte actual_state;
byte letter_info;
byte letter_length;
unsigned int letter_offset;
byte letter_position;
byte column_position;
byte text_position;
byte buffer_position;
byte space_count;
long previous_millis;


void setup() {

  Serial.begin(57600);
  Serial.println("Scrolling Demo");
  Serial.println();

  // all PINs are output
  pinMode(DISPLAY_CS, OUTPUT);
  pinMode(DISPLAY_WR, OUTPUT);
  pinMode(DISPLAY_DATA, OUTPUT);
  
  // enable System oscillator and LED duty cycle generator
  ht1632c_send_command(HT1632_CMD_SYSON);
  ht1632c_send_command(HT1632_CMD_LEDON);
  
  // clear display
  ht1632c_clear_display();
  
  // clear buffer
  for(int i = 0; i < 32; i++) display_buffer[i] = 0x00;  
  
  // reset FSM state and timer
  actual_state = S_OFF;
  previous_millis = 0;
}

void loop() {
  
  long current_millis = millis();
  
  // Time to scroll?
  if(current_millis - previous_millis > SCROLLING_SPEED) {
    previous_millis = current_millis;
    scroll();
  }
  
  // New character received from Serial?
  if (Serial.available() > 0) {
    
    // Read incoming character
    char incoming_char = Serial.read();
    
    // End of line?
    if(incoming_char == '\n') {
      
      // Add string terminator to rx buffer
      text_buffer[text_position] = '\0';
      
      // Copy received string
      strcpy(display_string, text_buffer);
      
      // initialize variables
      text_position = 0;      
      letter_position = 0;
      column_position = 0;
      buffer_position = 0;
      get_letter_variables();
      actual_state = S_LETTER;
      
      // clear buffer
      for(int i = 0; i < 32; i++) display_buffer[i] = 0x00;       
      
      Serial.print("Now displaying: ");
      Serial.println(display_string);
    }
    
    // Normal character? Save it in rx buffer
    else if(text_position < TEXT_BUFFER_SIZE - 2) {
      text_buffer[text_position] = incoming_char;
      text_position++;
    }
    
    // End of buffer reached? Restart from 0
    else {
      text_buffer[0] = incoming_char;
      text_position = 1;
    }
  }
}

void get_letter_variables() {
  
  // Get letter information from font descriptor
  letter_info = display_string[letter_position] - 33;
  letter_length = aipoint_info[letter_info].length;
  letter_offset = aipoint_info[letter_info].offset;
}

void scroll() {
  
  byte new_byte;
  
  switch(actual_state) {
    
    case S_OFF:
      return;
    
    case S_LETTER:
      
      // End of string reached?
      if(display_string[letter_position] == '\0') {
        new_byte = 0x00;
        column_position = 1;
        actual_state = S_BLANK;
        break;
      }
      
      // Character to be displayed is space?
      if(display_string[letter_position] == ' ') {
        new_byte = 0x00;
        column_position = 1;
        actual_state = S_SPACE;
        break;
      }
      
      new_byte = pgm_read_byte_near(aipointe_font + letter_offset + column_position);
      column_position++;
      if(column_position == letter_length) {
        column_position = 0;
        actual_state = S_SPACING;
      }
      break;

    // End of character reached? Send space
    case S_SPACING:
      new_byte = 0x00;
      column_position++;
      if(column_position == SPACING_SIZE)  {
        column_position = 0;
        letter_position++;
        get_letter_variables();
        actual_state = S_LETTER;
      }
      break;

    // Send "space" character
    case S_SPACE:
      new_byte = 0x00;
      column_position++;
      if(column_position == SPACE_SIZE)  {
        column_position = 0;
        letter_position++;
        get_letter_variables();
        actual_state = S_LETTER;
      }
      break;
    
    // Send "blank" before the next string  
    case S_BLANK:
    
      new_byte = 0x00;
      column_position++;
      if(column_position == BLANK_SIZE) {
        letter_position = 0;
        column_position = 0;        
        get_letter_variables();
        actual_state = S_LETTER;
      }    
      break;    
  }
  
  updateDisplay(new_byte);
}

void updateDisplay(byte new_byte) {
  
  display_buffer[buffer_position] = new_byte;
  buffer_position = (buffer_position + 1) % 32;


  digitalWrite(DISPLAY_CS, LOW);  
  ht1632c_send_bits(HT1632C_WRITE, 1 << 2);
  ht1632c_send_bits(0x00, 1 << 6);

  for(int i = 0; i < 32; i++) {
    ht1632c_send_bits(display_buffer[(i + buffer_position) % 32], 1<<7);  
  }

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

void ht1632c_display_buffer(byte* buffer) {

  digitalWrite(DISPLAY_CS, LOW);  
  ht1632c_send_bits(HT1632C_WRITE, 1 << 2);
  ht1632c_send_bits(0x00, 1 << 6);
  for(int i = 0; i < 32; i++) ht1632c_send_bits(buffer[i], 1<<7);
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
  

