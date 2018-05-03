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
BLEClientUart clientUart;

#define MAX_VALUES 2
#include "protocol.h" //serial communication protocol, includes receive_message()

static struct message_t msg;

const int outputPins[ 4 ] = { 2, 3, 4, 5 };

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

void setup()
{
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Left Haptic Actuator");

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

  pinMode(outputPins[ 0 ], OUTPUT);
  digitalWrite(LED_RED, LOW);
  intro();
  Serial.begin(115200);

  Serial.println("Haptic Feedback Actuator");
  Serial.println("The device receive two pwm values via UART");
  Serial.println("Using serial message protocol defined in 'protocol.h'");
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
  intro();

  Serial.println("Disconnected");
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
  //set values to zero
  for (uint16_t& val : msg.data) {
    val = 0;
  }
  
  while ( Bluefruit.Central.connected() )
  {
    // Not discovered yet
    if ( clientUart.discovered() )
    {
      uint16_t bytesread = receive_message(&msg);

      Serial.print("Received value ");
      Serial.print(msg.data[ 0 ]);
      Serial.print("\t");
      Serial.println(msg.data[ 1 ]);
      analogWrite( outputPins[ 0 ], map( msg.data[ 0 ], 0, 4095, 0, 255 ) ); // rear vib
      analogWrite( outputPins[ 2 ], map( msg.data[ 1 ], 0, 4095, 0, 255 ) ); // front vib

    }
  }
  
  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}
