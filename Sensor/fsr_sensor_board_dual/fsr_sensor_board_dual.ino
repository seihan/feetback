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
#include <SPI.h>

SPISettings settings(20000000, MSBFIRST, SPI_MODE0);

// Peripheral uart service
BLEUart bleuart;

// Central uart client
BLEClientUart clientUart;

#define MAX_VALUES 2 // 956
#include "protocol.h"

#include "sole_fsr956.h"
#include "sole.h"

struct message_t msg;

bool ready = false;
bool central = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("FSR Sensor Board - Wireless BLE UART");
  Serial.println("---------------------------\n");

  msg.length = MAX_VALUES;
  analogReference(AR_INTERNAL_1_2);
  analogReadResolution(14);
  pinMode(MuxDC_PIN, OUTPUT);
  pinMode(MuxADC_PIN, OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(DC_PINS[ i ], OUTPUT);
  for (int i = 0; i < 5; i++) pinMode(ADC_PINS[ i ], INPUT);
  SPI.begin();

  // Initialize Bluefruit with max concurrent connections as Peripheral = 1, Central = 1
  // SRAM usage required by SoftDevice will increase with number of connections
  Bluefruit.begin(true, true);
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("FSR Sensor - left");

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

void loop()
{
  uint16_t values[ 956 ] = {};
  uint16_t data[ 4 ] = {};

  for (uint16_t& val : msg.data) val = 0;

  read_sole(values);

  for (int i = 0; i < 956; i++) {
    if ( i <= 428 ) {
      if ( data[ 0 ] < values[ i ] ) data[ 0 ] = values[ i ];
    }
    else {
      if ( data[ 1 ] < values[ i ] ) data[ 1 ] = values[ i ];
    }
  }

  if (ready) {
    int bytesread = receive_message(&msg);
    for (int i = 0; i < 2; i++){
      data[ i + 2 ] = msg.data[ i ];
    }
    
  } else {
    // Request CPU to enter low-power mode until an event/interrupt occurs
    waitForEvent();
  }

  if (central) {
    bleuart.println(String(data[ 0 ]) + ";" + String(data[ 1 ]) + ";" + String(data[ 2 ]) + ";" + String(data[ 3 ]));
  } else {
    // Request CPU to enter low-power mode until an event/interrupt occurs
    waitForEvent();
  }
}

/*------------------------------------------------------------------*/
/* Peripheral
  ------------------------------------------------------------------*/
void prph_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  central = true;

  Serial.print("[Prph] Connected to ");
  Serial.println(peer_name);
}

void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  central = false;

  Serial.println();
  Serial.println("[Prph] Disconnected");
}

void prph_bleuart_rx_callback(void)
{
  // Forward data from Mobile to our peripheral
  /*char str[20 + 1] = { 0 };
    bleuart.read(str, 20);

    Serial.print("[Prph] RX: ");
    Serial.println(str);

    if ( clientUart.discovered() )
    {
    clientUart.print(str);
    } else
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

  ready = true;

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

  ready = false;

  Serial.println("[Cent] Disconnected");
}

/**
   Callback invoked when uart received data
   @param cent_uart Reference object to the service where the data
   arrived. In this example it is clientUart
*/
void cent_bleuart_rx_callback(BLEClientUart& cent_uart)
{
  /*char str[20 + 1] = { 0 };
    cent_uart.read(str, 20);

    Serial.print("[Cent] RX: ");
    Serial.println(str);

    if ( bleuart.notifyEnabled() )
    {
    // Forward data from our peripheral to Mobile
    bleuart.print( str );
    } else
    {
    // response with no prph message
    clientUart.println("[Cent] Peripheral role not connected");
    }*/
}
