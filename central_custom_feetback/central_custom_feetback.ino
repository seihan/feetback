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

/* This sketch show how to use BLEClientService and BLEClientCharacteristic
   to implement a custom client that is used to talk with Gatt server on
   peripheral.

   Note: you will need another feather52 running peripheral/custom_HRM sketch
   to test with.
*/

#include <bluefruit.h>

SoftwareTimer throughputTimer;
int throughput = 0;

void print_throughput(TimerHandle_t /* xTimerID */) {
  Serial.println(throughput);
  throughput = 0;
}

const uint8_t feetback_service_uuid[16]   = { "FEETBACKSERVICE" }; // 15 chars + '\0'
const uint8_t feetback_char_data_uuid[16] = { "FEETBACK___DATA" };
const uint8_t feetback_char_meta_uuid[16] = { "FEETBACK___META" };

BLEClientService        feetback = BLEClientService(BLEUuid(feetback_service_uuid));
BLEClientCharacteristic ftb_data = BLEClientCharacteristic(BLEUuid(feetback_char_data_uuid));
BLEClientCharacteristic ftb_meta = BLEClientCharacteristic(BLEUuid(feetback_char_meta_uuid));

struct feet_t
{
  uint8_t x;
  uint8_t y;
  uint16_t val[9];
};

// feetback sensor location value is 8 bit
static const char* location_names[] = { "Left", "Right" };

void setup()
{
  throughputTimer.begin(1000, print_throughput);
  throughputTimer.start();

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central Custom HRM Example");
  Serial.println("--------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Bluefruit52 Central");

  // Initialize Feetback client
  feetback.begin();

  // Initialize client characteristics of Feetback.
  // Note: Client Char will be added to the last service that is begin()ed.
  ftb_meta.begin();

  // set up callback for receiving measurement
  ftb_data.setNotifyCallback(feetback_notify_callback);
  ftb_data.begin();

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);
  Bluefruit.Central.setConnectCallback(connect_callback);

  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Don't use active scan
     - Filter only accept HRM service
     - Start(timeout) with timeout = 0 will scan forever (until connected)
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(feetback.uuid);
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds
}

void loop()
{
  // do nothing
}

/**
   Callback invoked when scanner pick up an advertising data
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with hrm service advertised
  // Connect to device with HRM service in advertising
  Bluefruit.Central.connect(report);
}

/**
   Callback invoked when an connection is established
   @param conn_handle
*/
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");
  Serial.print("Discovering Feetback Service ... ");

  // If HRM is not found, disconnect and return
  if ( !feetback.discover(conn_handle) )
  {
    Serial.println("Found NONE");

    // disconect since we couldn't find HRM service
    Bluefruit.Central.disconnect(conn_handle);

    return;
  }

  // Once HRM service is found, we continue to discover its characteristic
  Serial.println("Found it");


  Serial.print("Discovering Measurement characteristic ... ");
  if ( !ftb_data.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");
    Serial.println("Measurement characteristic is mandatory but not found");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  }
  Serial.println("Found it");

  // Measurement is found, continue to look for option Body Sensor Location
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.body_sensor_location.xml
  // Body Sensor Location is optional, print out the location in text if present
  Serial.print("Discovering Body Sensor Location characteristic ... ");
  if ( ftb_meta.discover() )
  {
    Serial.println("Found it");

    // Read 8-bit ftb_meta value from peripheral
    uint8_t loc_value = ftb_meta.read8();

    Serial.print("Body Location Sensor: ");
    Serial.println(location_names[loc_value]);
  } else
  {
    Serial.println("Found NONE");
  }

  // Reaching here means we are ready to go, let's enable notification on measurement chr
  if ( ftb_data.enableNotify() )
  {
    Serial.println("Ready to receive Feetback Measurement value");
  } else
  {
    Serial.println("Couldn't enable notify for Feetback Measurement. Increase DEBUG LEVEL for troubleshooting");
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
}


/**
   Hooked callback that triggered when a measurement value is sent from peripheral
   @param chr   Pointer client characteristic that even occurred,
                in this example it should be ftb_data
   @param data  Pointer to received data
   @param len   Length of received data
*/
void feetback_notify_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{
  if (len != sizeof(feet_t)) {
    Serial.print("Received unknown data of length ");
    Serial.print(len);
    Serial.print(" (Expected: ");
    Serial.print(sizeof(feet_t));
    Serial.println(")");
    return;
  }

  feet_t value;
  memcpy(&value, data, sizeof(value));
  throughput++;

  return;
  Serial.print("Feetback Measurement:  (");
  Serial.print(value.x);
  Serial.print(",");
  Serial.print(value.y);
  Serial.print(") ");

  Serial.println(value.val[0]);
}
