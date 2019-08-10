#define MAX_VALUES_SL 165
#define ROWS 11
#define COLUMNS 15
#define READINGS 3

#include "protocol.h"

static struct message_sl msg;
int duration = 0;
int SIG_pin = A5;

// column pins to set column high
const int columnPins[ COLUMNS ] = {
  15, 14, 13, 12, 11, 25, 28, // rear
  8, 5, 4, 20, 22, 23, 30, 31 // front
};

//mux control pins
int s0 = 19;
int s1 = 18;
int s2 = 17;
int s3 = 16;

void setup() {
  pinMode(SIG_pin, INPUT);
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

  // message length
  msg.length = MAX_VALUES_SL;

  Serial.begin(9600);
}

int readMux(int channel) {
  int controlPin[ ] = {s0, s1, s2, s3};

  int muxChannel[ ROWS ][ 4 ] = {
    //{0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    /*{0, 1, 1, 0}, //channel 6
      {1, 1, 1, 0}, //channel 7
      {0, 0, 0, 1}, //channel 8*/
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    //    {1, 1, 1, 1} //channel 15
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

void columnsHIGH( void ) {
  int input = Serial.read();
  switch ( input ) {
    case '1':
      Serial.println("\n\nColumn 1, PIN " + String(columnPins[ 0 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 0 ], HIGH);
      break;
    case '2':
      Serial.println("\n\nColumn 2, PIN " + String(columnPins[ 1 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 1 ], HIGH);
      break;
    case '3':
      Serial.println("\n\nColumn 3, PIN " + String(columnPins[ 2 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 2 ], HIGH);
      break;
    case '4':
      Serial.println("\n\nColumn 4, PIN " + String(columnPins[ 3 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 3 ], HIGH);
      break;
    case '5':
      Serial.println("\n\nColumn 5, PIN " + String(columnPins[ 4 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 4 ], HIGH);
      break;
    case '6':
      Serial.println("\n\nColumn 6, PIN " + String(columnPins[ 5 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 5 ], HIGH);
      break;
    case '7':
      Serial.println("\n\nColumn 7, PIN " + String(columnPins[ 6 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 6 ], HIGH);
      break;
    case '8':
      Serial.println("\n\nColumn 8, PIN " + String(columnPins[ 7 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 7 ], HIGH);
      break;
    case '9':
      Serial.println("\n\nColumn 9, PIN " + String(columnPins[ 8 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 8 ], HIGH);
      break;
    case 'q':
      Serial.println("\n\nColumn 10, PIN " + String(columnPins[ 9 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 9 ], HIGH);
      break;
    case 'w':
      Serial.println("\n\nColumn 11, PIN " + String(columnPins[ 10 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 10 ], HIGH);
      break;
    case 'e':
      Serial.println("\n\nColumn 12, PIN " + String(columnPins[ 11 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 11 ], HIGH);
      break;
    case 'r':
      Serial.println("\n\nColumn 13, PIN " + String(columnPins[ 12 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 12 ], HIGH);
      break;
    case 't':
      Serial.println("\n\nColumn 14, PIN " + String(columnPins[ 13 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 13 ], HIGH);
      break;
    case 'z':
      Serial.println("\n\nColumn 15, PIN " + String(columnPins[ 14 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 14 ], HIGH);
      break;
    case 'u':
      Serial.println("\n\nColumn 16, PIN " + String(columnPins[ 15 ]) + "\n\n");
      columnsLOW();
      digitalWrite(columnPins[ 15 ], HIGH);
      break;
    default:
      break;
  }
}

void columnsLOW( void ) {
  for (int i = 0; i < COLUMNS; i++) {
    digitalWrite(columnPins[ i ], LOW);
  }
}

void loop() {
  duration = millis();
  for (uint16_t& val : msg.data) { //set values to zero
    val = 0;
  }
  columnsHIGH();
  int k = 0;
  // for (int i = 0; i < COLUMNS; i++) {
  //   digitalWrite(columnPins[ i ], HIGH);
  for (int j = 0; j < ROWS; j++) {
    uint16_t tmp = 0;
    for (int r = 0; r < READINGS; r++) {
      tmp += readMux( j );
    }
    tmp = tmp / READINGS;
    msg.data[ k ] = tmp;
    Serial.print( msg.data[ k ] );
    Serial.print("\t");
    k++;
    delay(1);
  }
  //  digitalWrite(columnPins[ i ], LOW);
  Serial.println();
  //Serial.println("---------------------------------------------------------------------------------------------------------------------");
  //transmit values
  if ( Serial.available() ) {
    // send_to_serial(&msgsl);
  }
}
