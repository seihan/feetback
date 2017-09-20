/*********************************************************************
  This is an example for our nRF52 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

/*
   This sketch demonstrate the central API(). A additional bluefruit
   that has bleuart as peripheral is required for the demo.
*/

#include <bluefruit.h>

BLEClientDis  clientDis;
BLEClientUart bleuart;

#define MAX_VALUES 2
#define ROWS 16
#define COLUMNS 5

#include "protocol.h"

static struct message_t msg;
int duration = 0;

// array to store values
uint16_t values[ ROWS ][ COLUMNS ] = {}; //[rows][columns]
// array to store mean values
uint16_t rowmean[ ROWS ]  = {};
// array to store mean values
uint16_t rowmax[ ROWS ]  = {};
//temporary variable
uint16_t maxi = 0;

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


/*
 * Callback invoked when scanner picks up an advertising packet
 * @param report Structural advertising data
 */
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising data contains the BleUart service UUID
  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
  {
    Serial.print("BLE UART service detected. Connecting ... ");

    // Connect to the device with bleuart service in advertising packet
    Bluefruit.Central.connect(report);
  }
}

void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");

  Serial.print("Dicovering DIS ... ");
  if ( clientDis.discover(conn_handle) )
  {
    Serial.println("Found it");
    char buffer[32 + 1];

    // read and print out Manufacturer
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getManufacturer(buffer, sizeof(buffer)) )
    {
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    // read and print out Model Number
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getModel(buffer, sizeof(buffer)) )
    {
      Serial.print("Model: ");
      Serial.println(buffer);
    }

    // read and print out Device Serial Number
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getSerial(buffer, sizeof(buffer)) )
    {
      Serial.print("Serial: ");
      Serial.println(buffer);
    }
    Serial.println();
  }

  Serial.print("Discovering BLE Uart Service ... ");

  if ( bleuart.discover(conn_handle) )
  {
    Serial.println("Found it");

    Serial.println("Enable TXD's notify");
    bleuart.enableTXD();

    Serial.println("Ready to receive from peripheral");
  } else
  {
    Serial.println("Found NONE");
  }
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  Serial.println("Bluefruit will auto start scanning (default)");
}

void uart_rx_callback(void)
{
  Serial.print("[RX]: ");

  // while ( clientUart.available() )
  while ( bleuart.available() )
  {
    //    Serial.print( (char) clientUart.read() );
    Serial.print( (char) bleuart.read() );
  }

  Serial.println();
}

void setup()
{
  pinMode(s0, OUTPUT); //muxing control pins
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

  // up to 1 peripheral conn and 1 central conn
  Bluefruit.begin(true, true);
  Bluefruit.setName("LeftSensorSole");

  // Configure DIS client
  clientDis.begin();

  // Init BLE Central Uart Serivce
  bleuart.begin();

  bleuart.setRxCallback(uart_rx_callback);

  // Increase BLink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Start Central Scan
  Bluefruit.Central.setScanCallback(scan_callback);
  Bluefruit.Central.startScanning();
}

void loop()
{
  //read values
  duration = millis();
  for (int i = 0; i < COLUMNS; i++) {
    //digitalWrite(columnPins[i], HIGH);
    for (int j = 0; j < ROWS; j++) {
      values[ j ][ i ] = readMux(j);
      //  delay(1);
    }
    //digitalWrite(columnPins[i], LOW);
  }
  
  //print values
  for (int i = 0; i < ROWS; i++) {
    //digitalWrite(columnPins[i], HIGH);
    for (int j = 0; j < COLUMNS; j++) {
      Serial.print(values[ i ][ j ]);
      Serial.print("\t");
    }
  Serial.println();
  }
  Serial.println("--------------------------------\n");
  
  //compute mean and max values
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      //rows[ i ] += values[ i ][ j ]; //sum all columns in one row
      if (maxi < values[ i ][ j ]) { // get maximum
        maxi = values[ i ][ j ];
      }
    }
    //rows[ i ] = rows[ i ] / COLUMNS //compute mean
    rowmax[ i ] = maxi; //maximum value in row
    maxi = 0;
    if (i < 7) { //maximum value between row 1 to 7 -> FRONT
      if (msg.data[ 0 ] < rowmax[ i ]) {
        msg.data[ 0 ] = rowmax[ i ];
      }
    }
    if (i > 8) { //maximum value between row 8 to 16 -> REAR
      if (msg.data[ 1 ] < rowmax[ i ]) {
        msg.data[ 1 ] = rowmax[ i ];
      }
    }
  }
  duration = millis() - duration;

  //map values for pwm
  msg.data[ 0 ] = map(msg.data[ 0 ], 0, 400, 0, 255);
  msg.data[ 1 ] = map(msg.data[ 1 ], 0, 400, 0, 255);

  //debug print
  /*  for(int i = 0; i < 16; i++){
      Serial.print(values[i][0]);
      Serial.print("\t");
      Serial.println(rowmax[i]);
    }
    Serial.println("----------\n");
    for(int i = 0; i < 2; i++){
      Serial.print(msg.data[ i ]);
      Serial.print("\t");
    }
    Serial.println("\n----------\n");
    Serial.print("duration = ");
    Serial.print(duration);
    Serial.println("ms\n");*/


  if ( Bluefruit.Central.connected() )
  {
    //Bluefruit.Central.setScanCallback(scan_callback);
    //Bluefruit.Central.startScanning();
    // Not discovered yet
    if ( bleuart.discovered() )
    {
      // transmit message via bleuart
      send_message(&msg);
    }
  }
  for (uint16_t& val : msg.data) { //reset values to zero
    val = 0;
  }
}
