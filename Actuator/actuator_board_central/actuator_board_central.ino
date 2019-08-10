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

// Peripheral uart service
BLEUart bleuart; // Not used but defined in protocol.h

BLEClientDis  clientDis;
BLEClientUart clientUart;

#define MAX_VALUES 2 // 956
#include "protocol.h"

struct message_t msg;

const int outputPins[ 4 ] = { 2, 3, 4, 5 };

bool ready = false;

void setup()
{
  analogReference(AR_INTERNAL);
  analogReadResolution(8);
  Serial.begin(115200);

  Serial.println("Actuator Board");
  Serial.println("-----------------------------------\n");

  // UART Protocol
  msg.length = MAX_VALUES;

  // PWM
  for (int i = 0; i < 4; i++)
  {
    HwPWMx[ 0 ]->addPin( outputPins[i] );
  }
  HwPWM0.begin();
  HwPWM0.setResolution(10);
  HwPWM0.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_1); // default : freq = 16Mhz

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(false, true);

  Bluefruit.setName("Actuator Handle - right");

  // Configure DIS client
  clientDis.begin();

  // Init BLE Central Uart Serivce
  clientUart.begin();
  clientUart.setRxCallback(bleuart_rx_callback);

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Don't use active scan
     - Start(timeout) with timeout = 0 will scan forever (until connected)
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // // 0 = Don't stop scanning after n seconds
}

/**
   Callback invoked when scanner pick up an advertising data
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising contain BleUart service
  if ( Bluefruit.Scanner.checkReportForService(report, clientUart) )
  {
    Serial.print("BLE UART service detected. Connecting ... ");

    // Connect to device with bleuart service in advertising
    Bluefruit.Central.connect(report);
  }
}

/**
   Callback invoked when an connection is established
   @param conn_handle
*/
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

    Serial.println();
  }

  Serial.print("Discovering BLE Uart Service ... ");

  if ( clientUart.discover(conn_handle) )
  {
    Serial.println("Found it");

    Serial.println("Enable TXD's notify");
    clientUart.enableTXD();

    Serial.println("Ready to receive from peripheral");

    ready = true;
  } else
  {
    Serial.println("Found NONE");

    // disconect since we couldn't find bleuart service
    Bluefruit.Central.disconnect(conn_handle);
  }
}

/**
   Callback invoked when a connection is dropped
   @param conn_handle
   @param reason
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");

  ready = false;
}

/**
   Callback invoked when uart received data
   @param uart_svc Reference object to the service where the data
   arrived. In this example it is clientUart
*/
void bleuart_rx_callback(BLEClientUart& uart_svc)
{
  /*Serial.print("[RX]: ");

    while ( uart_svc.available() )
    {
    Serial.print( (char) uart_svc.read() );
    }

    Serial.println();*/
}

void loop()
{
  int32_t t;
  float temperature;
  sd_temp_get(&t);
  temperature = t / 4.0;
  Serial.println("Temperature = " + String(temperature) + " degree C");
  delay(1000);
  if (ready) {
    int bytesread = receive_message(&msg);
    for (int i = 0; i < 2; i++) {
      Serial.print(String( msg.data[ i ] ) + "\t");
      if (msg.data[ i ] > 2000) {
        HwPWMx[ 0 ] -> writePin( outputPins[ i ], map( msg.data[ i ], 0, 7500, 0, 1023 ), false);
      } else {
        HwPWMx[ 0 ] -> writePin( outputPins[ i ], 0, false);
      }
    }
    Serial.println();
  } else {
    // Request CPU to enter low-power mode until an event/interrupt occurs
    waitForEvent();
  }
}
