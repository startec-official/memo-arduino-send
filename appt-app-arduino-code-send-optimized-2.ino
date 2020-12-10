/*
 * 
 * MESSAGING ENGINE FOR MEDICAL ORGANIZATIONS(MEMO) v1.0.0
 * ARDUINO CODE V1.0.0
 * 
 * 
  AUTHORED BY: GABRIEL O. PONON
  STARTEC INNOVATIONS
  
  MIT LICENSE, VISIBLE AUTHOR ATTRIBUTION
  WRITTEN ON: 10/05/2020

  CODEBASE:
  
  PROGMEM string demo
  RETRIEVED FROM: https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
  ADDITIONAL REFERENCE: http://www.nongnu.org/avr-libc/user-manual/pgmspace.html

  GSM Interfacing Code
  RETRIEVED FROM: E-Gizmo Mechatronix (https://www.e-gizmo.net/)
  DERIVED FROM: SEEDSTUDIO (https://www.seeedstudio.com)
  
*/

#include <avr/pgmspace.h>
#include <string.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <GPRS_Shield_Arduino.h>
#include <sim900.h>
#include <MemoryFree.h>
#include <pgmStrToRAM.h>

const char init_message[] PROGMEM = "The device was booted. Now Running Server.";
const char full_slots_message[] PROGMEM = "Slots for your date are filled up. Choose another date";
const char timing_message[] PROGMEM = "You must sign up one day before the desired appointment. Try again";
const char wrong_format_message[] PROGMEM = "Please check your text exactly follows the format. Text HELP to see format";
const char resched_message[] PROGMEM = "You are being rescheduled. Please wait for action from the office";
const char format_message[] PROGMEM = "Format: <FULL NAME>-<MM>/<DD>/<YY>-<optional reason>. Ex: JUAN A. DELA CRUZ-01/01/20";
const char cancel_message[] PROGMEM = "Sorry. Your registration was cancelled by the office. You can register again";
const char reg_success_message[] PROGMEM = "You have succesfully registered. See you on ";
const char resched_success_message[] PROGMEM = "You've been rescheduled. See you on ";

// Then set up a table to refer to your strings.
// STAR
const char *const string_table[] PROGMEM = {init_message, full_slots_message, timing_message, wrong_format_message, resched_message, format_message, cancel_message, reg_success_message,resched_success_message};

char buffer[90];  // make sure this is large enough for the largest string it must hold
char inputString[100];
int input_string_char_ct = 0;
boolean stringComplete = false;

#define PHONE_NUMBER "+************" // TODO: setup admin phone number on startup
#define MESSAGE_LENGTH 160

GPRS GSMDEVICE(2,3,9600);//RX,TX,BAUDRATE

void setup() {
  Serial.begin(9600);
  while (!Serial);  // wait for serial port to connect. Needed for native USB
  Serial.println(F("SCS"));
  Serial.println(F("IB"));
  while(!GSMDEVICE.init()) {
    delay(1000);
    Serial.println(F("IW"));
  }
  Serial.println(F("SI"));
  delay(1000);
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[0])));
  bool msgSent = GSMDEVICE.sendSMS(PHONE_NUMBER,buffer);
  Serial.println(F("SW"));
  delay(1000);
  if( msgSent ) {
    Serial.println(F("SS"));
  }
  else {
    Serial.println(F("SF")); 
  }
}

void loop() {
  
  if(stringComplete) {
    char message_buffer[146]; // TODO: check for send buffer in library, adjust accordingly
    char outgoing_number[16]; // TODO: add null termination if this doesn't work
    
    int number_idx;
    
    // get the phone number
    for( int i = 1 ; i < input_string_char_ct ; i ++ ) { 
      if(  inputString[i] == ';' || i == input_string_char_ct - 1 ) {
        number_idx = i;
        outgoing_number[i-1] = '\0';
        break;
      }
      outgoing_number[i-1] = inputString[i];
    }
    // handle different send message cases
    switch(inputString[0]) { // BOY
      Serial.print(F("Free Memory: "));
      Serial.println(freeMemory());
      case 'F': // case for appointments full, choose a new date
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[1])));
        sprintf( message_buffer, "%s" , buffer );
        break;
      case 'T':
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[2])));
        sprintf( message_buffer, "%s" , buffer );
        break;
      case 'P':
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[3])));
        sprintf( message_buffer, "%s" , buffer );
        break;
      case 'M':
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[4])));
        sprintf( message_buffer, "%s" , buffer );
        break;
      case 'H':
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[5])));
        sprintf( message_buffer, "%s" , buffer );
        break;
      case 'X':
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[6])));
        sprintf( message_buffer, "%s" , &buffer );
        break;
      case 'C':
        char custom_message_buffer[50];
        for( int i = number_idx + 1 ; i < input_string_char_ct ; i ++ ){
          custom_message_buffer[ i - (number_idx + 1) ] = inputString[i];
        }
        custom_message_buffer[ input_string_char_ct ] = '\0';
        sprintf( message_buffer, "%s" , custom_message_buffer);
        break;
      case 'S': {
      int counterFlag = 0;
      int date_idx = 0, time_idx = 0, code_idx = 0, order_idx = 0;
      char date_string[9],time_string[17],code_string[5], order_string[7];
        counterFlag = 0;
        date_idx = 0;
        time_idx = 0;
        code_idx = 0;
        order_idx = 0;
        
        for( int i = number_idx + 1 ; i < input_string_char_ct ; i ++ ) {
          if( inputString[i] == '|' ) {
            switch(counterFlag) {
              case 0 :
                date_string[ i - (number_idx + 1) ] = '\0';
                date_idx = i;
                break;
              case 1 :
                time_string[ i - date_idx - 1 ] = '\0';
                time_idx = i;
                break;
              case 2 :
                code_string[ i - time_idx - 1 ] = '\0';
                code_idx = i;
                break;
              case 3 :
                order_string[ i - code_idx - 1 ] = '\0';
                order_idx = i;
                break;
            }
            counterFlag ++;
          }
          else {
            switch(counterFlag) {
              case 0 :
                date_string[i - ( number_idx + 1 ) ] = inputString[i];
                break;
              case 1 :
                time_string[i - date_idx - 1] = inputString[i];
                break;
              case 2 :
                code_string[i - time_idx - 1] = inputString[i];
                break;
              case 3 :
                order_string[ i - code_idx - 1] = inputString[i];
                break;
            }
          }
        }
        
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[7])));
        sprintf(message_buffer,"%s%s, %s! Your code is: %s. In the line you are %s",&buffer,&date_string,&time_string,&code_string,&order_string);
      }
        break;
      case 'R': {
        int counterFlag = 0;
        int date_idx = 0, time_idx = 0, code_idx = 0, order_idx = 0;
        char date_string[9],time_string[17],code_string[5];
        
        for( int i = number_idx + 1 ; i < input_string_char_ct ; i ++ ) {
          if( inputString[i] == '|' ) {
            switch(counterFlag) {
              case 0 :
                date_string[ i - (number_idx + 1) ] = '\0';
                date_idx = i;
                break;
              case 1 :
                time_string[ i - date_idx - 1 ] = '\0';
                time_idx = i;
                break;
              case 2 :
                code_string[ i - time_idx - 1 ] = '\0';
                code_idx = i;
                break;
            }
            counterFlag ++;
          }
          else {
            switch(counterFlag) {
              case 0 :
                date_string[i - ( number_idx + 1 ) ] = inputString[i];
                break;
              case 1 :
                time_string[i - date_idx - 1] = inputString[i];
                break;
              case 2 :
                code_string[i - time_idx - 1] = inputString[i];
                break;
            }
          }
        }
        Serial.print(F("Free Memory: "));
        Serial.println(freeMemory());
        strcpy_P(buffer, (char *)pgm_read_word(&(string_table[8])));
        sprintf(message_buffer,"%s%s, %s! Your code is: %s",&buffer,&date_string,&time_string,&code_string);
      }
        break;
      default:
        sprintf(message_buffer,"%s","System error");
    }
    // send the message
    Serial.println(F("SI"));
    bool msgSent = GSMDEVICE.sendSMS(outgoing_number,message_buffer);
//    delay(5000); // delay necessary for GSM to complete send task unimpeded
    if( msgSent ) {
      Serial.println(F("SS"));
    }
    else {
      Serial.println(F("SF"));
    }
    // reset variables for serialEvent read
    memset(inputString, 0, sizeof inputString);
    memset(message_buffer, 0, sizeof message_buffer);
    input_string_char_ct = 0;
    stringComplete = false;
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    inputString[input_string_char_ct] = inChar;
    input_string_char_ct ++;
    if (inChar == '\n') {
      inputString[input_string_char_ct] = '\0';
      input_string_char_ct ++;
      stringComplete = true;
    }
  }
}
