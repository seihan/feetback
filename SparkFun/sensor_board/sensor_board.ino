#define MAX_VALUES_SL 132
#define ROWS 13
#define COLUMNS 12

#include "protocol.h"

static struct message_sl msgsl;
int duration = 0;
int SIG_pin = A5;
int EN_pin = 20;

// column pins to set column high
const int columnPins[ COLUMNS ] = {
  14, 13, 12, 11, 8, 3, 2, 20, 22, 23, 30, 31
};

//mux control pins
int s0 = 19;
int s1 = 18;
int s2 = 17;
int s3 = 16;

void setup() {
  // ADC config
  //  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  for (int i = 0; i < COLUMNS; i++) {
    pinMode(columnPins[ i ], OUTPUT);
    digitalWrite(columnPins[ i ], LOW);
  }

  digitalWrite(EN_pin, LOW); // enable 16 channel multiplexer

  // message length
  msgsl.length = MAX_VALUES_SL;

  Serial.begin(115200);
}

int readMux(int channel) {
  int controlPin[ ] = {s0, s1, s2, s3};

  int muxChannel[ ROWS ][ 4 ] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    /*{1, 0, 1, 0}, //channel 5
      {0, 1, 1, 0}, //channel 6
      {1, 1, 1, 0}, //channel 7*/
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
    digitalWrite(controlPin[ i ], muxChannel[ channel ][ i ]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}

void loop() {
  duration = millis();
  int k = 0;
  for (int i = 0; i < COLUMNS; i++) {
    digitalWrite(columnPins[ i ], HIGH);
    for (int j = 1; j < ROWS - 1; j++) {
      int tmp = 0;
      for (int r = 0; r < 3; r++) {
        tmp += readMux( j );
      }
      msgsl.data[ k ] = tmp / 3;
      k++;
      //  Serial.print( msg.data[ k ] );
      //  Serial.print("\t");
      delay(1);
    }
    digitalWrite(columnPins[ i ], LOW);
    // Serial.println();
  }
  // Serial.println("---------------------------------------------------------------------------------------------------------------------");
  //transmit values
  if ( Serial.available() ) {
    send_to_serial(&msgsl);
  }
  for (uint16_t& val : msgsl.data) { //reset values to zero
    val = 0;
  }
}
