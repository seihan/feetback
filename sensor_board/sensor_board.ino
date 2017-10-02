#define MAX_VALUES 80
#define ROWS 16
#define COLUMNS 5

#include "protocol.h"

static struct message_t msg;
int duration = 0;

// column pins to set column high
const int columnPins[ COLUMNS ] = {
  27, 30, 16, 6, 20
};

//mux control pins
int s0 = 15;
int s1 = 7;
int s2 = 11;
int s3 = 31;

//mux in "SIG" pin
int SIG_pin = A3;

void setup(){
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);
  pinMode(SIG_pin, INPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  msg.length = MAX_VALUES;
  
  Serial.begin(115200);
}

/**
 * analogRead through a 16 channel muxer
 */
uint16_t readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[ ROWS ][ 4 ] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  uint16_t val = analogRead(SIG_pin);

  //return the value
  return val;
}

void sendAll(unsigned char* str, uint16_t len){
  Serial.write(str, len);  
}

void loop(){
  //read values
  duration = millis();
  int k = 0;
  for(int i = 0; i < COLUMNS; i++){
    //digitalWrite(columnPins[i], HIGH);
    for(int j = 0; j < ROWS; j++){
      msg.data[ k ] = readMux(j); //values[ j ][ i ];
      // msg.data[ k ] = k; //values[ j ][ i ];
      //print values
      //Serial.print(msg.data[ k ]);
      //Serial.print(" ");
      k++;
    }
    //Serial.println();
    //digitalWrite(columnPins[i], LOW);
  }
  //Serial.println("-------------------------------------------------");

  //transmit values
  send_message(&msg);
  for (uint16_t& val : msg.data) { //reset values to zero
    val = 0;
  }
//  delay(200);
}
