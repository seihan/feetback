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


uint32_t dutycycle = 0;
uint32_t last = 0;

uint32_t inc_last = 0;

int pwm_values[ 2 ] = {0, 0};


enum constants {
  DUTY_MIN = 0,
  DUTY_MAX = 300,
};
const int OUTPUT_PINS[ 4 ] = { 28, // OUT1 *
                               29, // OUT2 *
                               30, // OUT3
                               31 // OUT4
                             };

/*
   This sketch demonstrate the central API(). A additional bluefruit
   that has bleuart as peripheral is required for the demo.
*/
#include <bluefruit.h>

BLEClientDis  clientDis;
BLEClientUart clientUart;

#define MAX_BLE 2

#include "protocol.h"

struct message_t msgble;

void setup()
{
  for (int i = 0; i < 4; i++) {
    pinMode(OUTPUT_PINS[ i ], OUTPUT);
    digitalWrite(OUTPUT_PINS[ i ], LOW);
  }
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);

  intro();

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central BLEUART Example");
  Serial.println("-----------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Bluefruit52 Central");

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

  msgble.length = MAX_BLE;
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
  } else
  {
    // For Softdevice v6: after received a report, scanner will be paused
    // We need to call Scanner resume() to continue scanning
    Bluefruit.Scanner.resume();
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
   @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  intro();
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
  uint32_t now = millis();
  duty_cycle(now);
  pwm_values[ 1 ] = 0;
  if ( Bluefruit.Central.connected() )
  {
    // Not discovered yet
    if ( clientUart.discovered() )
    {
      //      measure(now);
      int bytesread = receive_message(&msgble);
      for (int i = 0; i < 2; i++) pwm_values[ i ] = constrain(msgble.data[ i ], 500, 4095);
      dutycycle = map(pwm_values[ 0 ], 500, 4095, DUTY_MIN, DUTY_MAX);
      //analogWrite(OUTPUT_PINS[ 0 ], map(pwm_values[ 1 ], 500, 4095, 0, 240));

      // Discovered means in working state
      // Get Serial input and send to Peripheral
      if ( Serial.available() )
      {
        uint32_t inc_dur = now - inc_last;
        if (inc_dur >= 300) {
          inc_last = now;

          /*char str[20 + 1] = { 0 };
            Serial.readBytes(str, 20);

            clientUart.print( str );*/
          Serial.println(String(msgble.data[ 0 ]) + "\t" + String(msgble.data[ 1 ]) + "\t" + String(dutycycle));
        }
      }
    }
  }
}

void measure(uint32_t now) {
  uint32_t inc_dur = now - inc_last;
  if (inc_dur >= 900) {
    inc_last = now;
    // "measure"
    dutycycle = (dutycycle + 10) % (DUTY_MAX + 1);
  }
}

void duty_cycle(uint32_t now) {
  uint32_t duration = now - last;

  if (dutycycle && duration >= DUTY_MAX) {
    last = now;
    digitalWrite(OUTPUT_PINS[ 0 ], HIGH);
  } else if (duration >= dutycycle) {
    digitalWrite(OUTPUT_PINS[ 0 ], LOW);
  }
}

void intro() {
  for (int i  = 0; i < 3; i++) {
    for (int i = 0; i < 4; i++) analogWrite(OUTPUT_PINS[ i ], 200);
    delay(200);
    for (int i = 0; i < 4; i++) analogWrite(OUTPUT_PINS[ i ], 0);
    delay(100);
  }
}
