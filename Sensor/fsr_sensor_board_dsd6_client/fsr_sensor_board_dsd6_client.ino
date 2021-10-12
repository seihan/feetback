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

#define MAX_VALUES 25
#define MEASURED_VALUES 956
#define WHITE_NOISE 200
 
#include <bluefruit.h>
#include <SPI.h>
#include "protocol.h"
#include "toplist.h"
//#include "sole_fsr956_feather.h"
#include "sole_fsr956_bl652.h"
#include "sole.h"

BLEClientService        d6 = BLEClientService(BLEUuid(0x190A));
BLEClientCharacteristic d6_write = BLEClientCharacteristic(BLEUuid(0x0001));
BLEClientCharacteristic d6_notif = BLEClientCharacteristic(BLEUuid(0x0002));

bool debug = true;
uint8_t dsd6_mac[ 6 ] = { 0xF6, 0x7B, 0xFA, 0xBD, 0x3A, 0xF1 };
bool connecting = true;
uint8_t counter = 0;
const String vib = "AT+MOTOR=1\n";  // start vibration
const String vib1 = "AT+MOTOR=11\n";  // 50ms vibration
const String vib2 = "AT+MOTOR=12\n";  // 100ms vibration
const String vib3 = "AT+MOTOR=13\n";  // 150ms vibration
const String sto = "AT+MOTOR=00\n"; // stop vibration
const String bat = "AT+BATT0\n";    // get battery state %

struct message_t msg;
uint16_t data[MEASURED_VALUES];
uint16_t balance[ 2 ] = { };

struct smaller_measurement {
  bool operator()(measure_t& e, measure_t& other) {
    return e.value < other.value;
  }
};

toplist<MAX_VALUES, measure_t, smaller_measurement> top;

void setup()
{
  if ( debug ) Serial.begin(115200);
  if ( debug ) Serial.println("Feetback Sensor");
  if ( debug ) Serial.println("---------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  /* Set the device name */
  Bluefruit.setName("FTB Sensor");

  /* Set the LED interval for blinky pattern on BLUE LED */
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);
  Bluefruit.Central.setConnectCallback(connect_callback);

  d6.begin();

  // Initialize client characteristics of HRM.
  // Note: Client Char will be added to the last service that is begin()ed.
  d6_write.begin();

  // set up callback for receiving measurement
  d6_notif.setNotifyCallback(d6_notify_callback);
  d6_notif.begin();

  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Filter out packet with a min rssi
     - Interval = 100 ms, window = 50 ms
     - Use active scan (used to retrieve the optional scan response adv packet)
     - Start(0) = will scan forever since no timeout is given
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-80);
  //  Bluefruit.Scanner.filterUuid(d6.uuid); // only invoke callback if detect bleuart service
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  if ( debug ) Serial.println("Scanning ...");

  analogReference(AR_INTERNAL_1_2);
  analogReadResolution(14);
  pinMode(MuxDC_PIN, OUTPUT);
  pinMode(MuxADC_PIN, OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(DC_PINS[ i ], OUTPUT);
  for (int i = 0; i < 5; i++) pinMode(ADC_PINS[ i ], INPUT);
  SPI.begin();
  msg.length = MAX_VALUES;
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  // Display the timestamp and device address
  if (report->type.scan_response)
  {
    if ( debug ) Serial.printf("[SR%10d] Packet received from ", millis());
  }
  else
  {
    if ( debug ) Serial.printf("[ADV%9d] Packet received from ", millis());
  }

  // MAC is in little endian --> print reverse
  if ( debug ) Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  if ( debug ) Serial.print("\n");

  for (int i = 0; i < 6; i++) {
    if (report->peer_addr.addr[i] != dsd6_mac[i]) connecting = false;
  }
  if ( connecting ) {
    if ( debug ) Serial.println("Connecting to DS-D6 device...");
    Bluefruit.Central.connect(report);
  }
  else {
    // For Softdevice v6: after received a report, scanner will be paused
    // We need to call Scanner resume() to continue scanning
    delay(10);
    Bluefruit.Scanner.resume();
  }
}

void connect_callback(uint16_t conn_handle)
{
  if ( debug ) Serial.println("Connected");
  send_ble_cmd(vib3);
  if ( debug ) Serial.print("Discovering D6 Service ... ");

  // If HRM is not found, disconnect and return
  if ( !d6.discover(conn_handle) )
  {
    if ( debug ) Serial.println("Found NONE");

    // disconnect since we couldn't find HRM service
    Bluefruit.disconnect(conn_handle);

    return;
  }
  // Once FEETBACK service is found, we continue to discover its characteristic
  if ( debug ) Serial.println("Found it");

  if ( debug ) Serial.print("Discovering NOTIFY characteristic ... ");
  if ( !d6_notif.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    if ( debug ) Serial.println("not found !!!");
    if ( debug ) Serial.println("Notify characteristic is mandatory but not found");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  if ( debug ) Serial.println("Found it");

  // Measurement is found, continue to look for option Body Sensor Location
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.body_sensor_location.xml
  // Body Sensor Location is optional, print out the location in text if present
  if ( debug ) Serial.print("Discovering WRITE characteristic ... ");
  if ( d6_write.discover() )
  {
    if ( debug ) Serial.println("Found it");
  } else
  {
    if ( debug ) Serial.println("Found NONE");
  }

  // Reaching here means we are ready to go, let's enable notification on measurement chr
  if ( d6_notif.enableNotify() )
  {
    if ( debug ) Serial.println("Ready to receive D6 Notify value");
  } else
  {
    if ( debug ) Serial.println("Couldn't enable notify for DSD6. Increase DEBUG LEVEL for troubleshooting.");
  }
  Bluefruit.printInfo();
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

  if ( debug ) Serial.print("Disconnected, reason = 0x");
  if ( debug ) Serial.println(reason, HEX);
}

void d6_notify_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{
  for (int i = 0; i < len; i++) {
    if ( debug ) Serial.print(data[i]);
  }
}

void send_ble_cmd(String cmd) {
  //  Serial.print("TxBle: ");
  //  Serial.println(cmd);
  char buf[12] = {};
  cmd.toCharArray(buf, 12);
  d6_write.write(buf, 12);
  d6_write.write8(10);
}

void loop()
{
  uint16_t nval = read_sole(data); // read values

  top.clear();
  for (uint16_t i = 0; i != nval; i++) {
    top.add(measure_t{i, data[i]});
  }
  nval = 0;
  for (uint16_t& val : balance) val = 0;
  for (const measure_t& val : top) {
    msg.data[nval] = val; // add top values to payload
    if ( msg.data[nval].value > WHITE_NOISE ) {
      if ( msg.data[nval].offset < 428 ) { // sum rear values to balance
        if (UINT16_MAX - balance[0] >= msg.data[nval].value)
          balance[ 0 ] += msg.data[nval].value;
      } else {                                // sum front values to balance
        if (UINT16_MAX - balance[1] >= msg.data[nval].value)
          balance[ 1 ] += msg.data[nval].value;
      }
    }
    nval++;
  }
  Serial.print(balance[0]);
  Serial.print('\t');
  Serial.println(balance[1]);
  counter = ( counter +  1 ) % 16;
 
  switch (counter) {
    case 3 : 
        if ( (balance[0] == UINT_LEAST16_MAX) || (balance[1] == UINT_LEAST16_MAX) ) {
          if ( balance[0] > balance[1] ) {
            send_ble_cmd(vib1);
          } else {
            send_ble_cmd(vib3);
          }
        }
      break;
    case 7 : 
      if ( (balance[0] > 50000) || (balance[1] > 50000) ) {
        if ( balance[0] > balance[1] ) {
          send_ble_cmd(vib1);
        } else {
          send_ble_cmd(vib3);
        }
      }
      break;
    case 11 :
      if ( (balance[0] > 30000) || (balance[1] > 30000) ) {
        if ( balance[0] > balance[1] ) {
          send_ble_cmd(vib1);
        } else {
          send_ble_cmd(vib3);
        }
      }
      break;
    case 15 :
      if ( (balance[0] > 5000) || (balance[1] > 5000) ) {
        if ( balance[0] > balance[1] ) {
          send_ble_cmd(vib1);
        } else {
          send_ble_cmd(vib3);
        }
      }
      break;
     default :
      break;
  }
}
