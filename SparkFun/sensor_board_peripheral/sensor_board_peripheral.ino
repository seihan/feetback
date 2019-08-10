// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
   Serial Port over BLE
   Create UART service compatible with Nordic's *nRF Toolbox* and Adafruit's *Bluefruit LE* iOS/Android apps.

   BLESerial class implements same protocols as Arduino's built-in Serial class and can be used as it's wireless
   replacement. Data transfers are routed through a BLE service with TX and RX characteristics. To make the
   service discoverable all UUIDs are NUS (Nordic UART Service) compatible.

   Please note that TX and RX characteristics use Notify and WriteWithoutResponse, so there's no guarantee
   that the data will make it to the other end. However, under normal circumstances and reasonable signal
   strengths everything works well.
*/

// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"


// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   6 //2
#define BLE_RST   9

// create ble serial instance, see pinouts above
BLESerial BLESerial(BLE_REQ, BLE_RDY, BLE_RST);

#define MAX_VALUES 2
#define MAX_VALUES_SL 165
#define ROWS 11
#define COLUMNS 15
#define READINGS 1

#include "protocol.h"

static struct message_sl msg;
static struct message_t msg_ble;

int SIG_pin = A5;
unsigned long previousMillis = 0;
const long interval = 200;
bool sCheck = false;

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
  // ADC config
  //  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  pinMode(SIG_pin, INPUT);

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

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
  msg_ble.length = MAX_VALUES;

  Serial.begin(115200);

  // BLE
  BLESerial.setLocalName("LeftSensorSole");
  BLESerial.begin();
}

uint16_t readMux(int channel) {
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
    //{1, 1, 1, 1} //channel 15
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[ i ], muxChannel[ channel ][ i ]);
  }

  //read the value at the SIG pin
  uint16_t val = analogRead(SIG_pin);

  //return the value
  return val;
}

void columnsLOW( void ) {
  for (int i = 0; i < COLUMNS; i++) {
    digitalWrite(columnPins[ i ], LOW);
  }
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

void loop() {
  BLESerial.poll();

  //set values to zero
  for (uint16_t& val : msg.data) {
    val = 0;
  }
  for (uint16_t& val : msg_ble.data) {
    val = 0;
  }

  int k = 0;
  for (int i = 0; i < COLUMNS; i++) {
    digitalWrite(columnPins[ i ], HIGH);
    for (int j = 0; j < ROWS; j++) {
      uint16_t tmp = 0;
      for (int r = 0; r < READINGS; r++) {
        tmp += readMux( j );
      }
      tmp = tmp / READINGS;
      msg.data[ k ] = tmp;
      // get maximum values
      if ( k <= 66 ) {
        if ( msg_ble.data[ 0 ] < msg.data[ k ] ) msg_ble.data[ 0 ] = msg.data[ k ];
      }
      else {
        if ( msg_ble.data[ 1 ] < msg.data[ k ] ) msg_ble.data[ 1 ] = msg.data[ k ];
      }
      k++;
      // delay(1);
    }
    digitalWrite(columnPins[ i ], LOW);
  }

  // transmit values
  // if ( Serial.available() ) {};    // send_to_serial(&msg);
  // unsigned long currentMillis = millis();
  switch ( BLESerial.read() ) {
    case 'r':
      sCheck = true;
      break;
    case 's':
      sCheck = false;
      break;
    default:
      break;
  }
  if ( sCheck ) {
    BLESerial.println(String(msg_ble.data[ 0 ]) + "\t" + String(msg_ble.data[ 1 ]));
  }
  else send_to_central(&msg_ble);
  //if ( BLESerial.available() )
  //  send_to_central(&msg_ble);
}
