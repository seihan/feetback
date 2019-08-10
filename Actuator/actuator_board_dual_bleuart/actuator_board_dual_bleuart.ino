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
   This sketch demonstrate how to run both Central and Peripheral roles
   at the same time. It will act as a relay between an central (mobile)
   to another peripheral using bleuart service.

   Mobile <--> DualRole <--> peripheral Ble Uart
*/
#include <bluefruit.h>

// Peripheral uart service
BLEUart bleuart;

// Central uart client
BLEClientUart clientUart;
#define MAX_VALUES 2 // 956

#include "protocol.h"

static struct message_t msg;

const int outputPins[ 4 ] = { 2, 3, 4, 5 };
uint16_t maximum = 1000;

const int ledPin =  LED_RED;
int outState = LOW;             // ledState used to set the LED

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
const long duty = 200; // ms
const long cycle = 40;
const long interval = 1000; // ms
int counter = 0;
bool battery_warning = false;
bool oneTap = false;
bool on;
uint16_t origns [ 2 ] = {};
void setup()

{
  analogReadResolution(12);
  pinMode(outputPins[ 0 ], OUTPUT);
  pinMode(ledPin, OUTPUT);
  intro();
  Serial.begin(115200);
  Serial.println("Haptic Feedback Actuator");
  msg.length = MAX_VALUES;

  // Initialize Bluefruit with max concurrent connections as Peripheral = 1, Central = 1
  // SRAM usage required by SoftDevice will increase with number of connections
  Bluefruit.begin(true, true);
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("ProprioT right");

  // Callbacks for Peripheral
  Bluefruit.setConnectCallback(prph_connect_callback);
  Bluefruit.setDisconnectCallback(prph_disconnect_callback);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);

  // Configure and Start BLE Uart Service
  bleuart.begin();
  bleuart.setRxCallback(prph_bleuart_rx_callback);

  // Init BLE Central Uart Serivce
  clientUart.begin();
  clientUart.setRxCallback(cent_bleuart_rx_callback);


  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Filter only accept bleuart service
     - Don't use active scan
     - Start(timeout) with timeout = 0 will scan forever (until connected)
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(bleuart.uuid);
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  // Set up and start advertising
  startAdv();
}

void intro() {
  for (int i = 0; i < 3; i++)
  {
    delay(200);
    for (int j = 0; j < 4; j++) {
      analogWrite(outputPins[ j ], 255);
    }
    delay(200);
    for (int j = 0; j < 4; j++) {
      analogWrite(outputPins[ j ], 0);
    }
  }
}

void tap(void) {
  unsigned long duration = 0;
  previousMillis = millis();
  while (duration < 200) {
    analogWrite( outputPins[ 0 ], 200 );
    analogWrite( outputPins[ 1 ], 200 );
    duration = millis() - previousMillis;
    receive_message(&msg);
  }
  analogWrite( outputPins[ 0 ], 0 );
  analogWrite( outputPins[ 1 ], 0 );
}

int read_battery(void) {
  int val = 0;
  for (int i = 0; i < 5; i++) {
    val += analogRead(A7);
  }
  return val / 5;
}
void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

/*------------------------------------------------------------------*/
/* Peripheral
  ------------------------------------------------------------------*/
void prph_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Prph] Connected to ");
  Serial.println(peer_name);
}

void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("[Prph] Disconnected");
}

void prph_bleuart_rx_callback(void)
{
  /*// Forward data from Mobile to our peripheral
    char str[20+1] = { 0 };
    bleuart.read(str, 20);

    Serial.print("[Prph] RX: ");
    Serial.println(str);

    if ( clientUart.discovered() )
    {
    clientUart.print(str);
    }else
    {
    bleuart.println("[Prph] Central role not connected");
    }*/
}

/*------------------------------------------------------------------*/
/* Central
  ------------------------------------------------------------------*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising contain BleUart service
  if ( Bluefruit.Scanner.checkReportForService(report, clientUart) )
  {
    Serial.println("BLE UART service detected. Connecting ... ");

    // Connect to device with bleuart service in advertising
    Bluefruit.Central.connect(report);
  }
}

void cent_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Cent] Connected to ");
  Serial.println(peer_name);;

  if ( clientUart.discover(conn_handle) )
  {
    // Enable TXD's notify
    clientUart.enableTXD();
  } else
  {
    // disconect since we couldn't find bleuart service
    Bluefruit.Central.disconnect(conn_handle);
  }
}

void cent_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("[Cent] Disconnected");
}

/**
   Callback invoked when uart received data
   @param cent_uart Reference object to the service where the data
   arrived. In this example it is clientUart
*/
void cent_bleuart_rx_callback(BLEClientUart& cent_uart)
{
  /*char str[20+1] = { 0 };
    cent_uart.read(str, 20);

    Serial.print("[Cent] RX: ");
    Serial.println(str);

    if ( bleuart.notifyEnabled() )
    {
    // Forward data from our peripheral to Mobile
    bleuart.print( str );
    }else
    {
    // response with no prph message
    clientUart.println("[Cent] Peripheral role not connected");
    }  */
}

void loop()
{
//  if ( !battery_warning ) {
   // while (bleuart.available() ) {
      int bytesread = receive_message(&msg);
      counter++;

      /*if ( (msg.data[ 0 ] > 400) && (oneTap) ) {
        oneTap = false;
        tap();
        Serial.println("Tap");
        }
        if ( (msg.data[ 1 ] > 400) && (oneTap) ) {
        oneTap = false;
        tap();
        Serial.println("Tap");
        }*/

      if ( (msg.data[ 0 ] < 350) && (msg.data[ 1 ] < 350) ) oneTap = true;

      for (int i = 0; i < 2; i++) {

        origns[ i ] = 0;
        origns[ i ] = msg.data[ i ];

        if ( maximum < msg.data[ i ] ) maximum = msg.data[ i ];

        msg.data[ i ] = constrain( map( msg.data[ i ], 0, maximum, 0, 255 ), 0, 255 );
      }
      bleuart.print(origns[ 0 ]);
      bleuart.print(";");
      bleuart.println(origns[ 1 ]);

      if ( on ) analogWrite(outputPins[ 0 ], msg.data[ 0 ]); // rear vib
      else analogWrite(outputPins[ 0 ], 0); // rear vib
      analogWrite(outputPins[ 1 ], msg.data[ 1 ]); // front vib
   // }
  //}
  /*else {
    for ( int i = 0; i < 4; i++ ) {
      analogWrite( outputPins[ i ], 0 );
    }
    digitalWrite( LED_RED, HIGH );
    delay(500);
    digitalWrite( LED_RED, LOW );
    delay(1000);
  }*/

  unsigned long currentMillis = millis();

  if ( currentMillis - previousMillis1 >= duty ) {
    previousMillis1 = currentMillis;
    on = false;
  }

  if ( currentMillis - previousMillis1 >= cycle ) {
    previousMillis2 = currentMillis;
    on = true;
  }

  if ( currentMillis - previousMillis >= interval ) {
    previousMillis = currentMillis;
    if ( read_battery() < 2291 ) battery_warning = true; // 2900mV lower voltage limit
    else battery_warning = false;
    Serial.println( String( origns[ 0 ] ) + "\t" + String( origns[ 1 ] ) + "\t" + String( maximum ) + "\t" + String( read_battery() / 0.79 ) + "mV\t" + "counts = " + String(counter) );
    counter = 0;
  }
  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
  // do nothing, all the work is done in callback
}
