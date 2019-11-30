#include <bluefruit.h>

BLEClientService        d6 = BLEClientService(BLEUuid(0x190A));
BLEClientCharacteristic d6_write = BLEClientCharacteristic(BLEUuid(0x0001));
BLEClientCharacteristic d6_notif = BLEClientCharacteristic(BLEUuid(0x0002));

uint8_t dsd6_mac[ 6 ] = { 0xF6, 0x7B, 0xFA, 0xBD, 0x3A, 0xF1 };
bool connecting = false;
const String vib = "AT+MOTOR=1\n";  // start vibration
const String vib1 = "AT+MOTOR=11\n";  // 50ms vibration
const String vib2 = "AT+MOTOR=12\n";  // 100ms vibration
const String vib3 = "AT+MOTOR=13\n";  // 150ms vibration
const String sto = "AT+MOTOR=00\n"; // stop vibration
const String bat = "AT+BATT0\n";    // get battery state %

void setup() {
  Serial.begin(115200);
  Serial.println("MPOW DS D6 Client");
  Serial.println("---------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  /* Set the device name */
  Bluefruit.setName("DSD6 Client");

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
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  // Display the timestamp and device address
  if (report->type.scan_response)
  {
    Serial.printf("[SR%10d] Packet received from ", millis());
  }
  else
  {
    Serial.printf("[ADV%9d] Packet received from ", millis());
  }

  // MAC is in little endian --> print reverse
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.print("\n");

  for (int i = 0; i < 6; i++) {
    if (report->peer_addr.addr[i] == dsd6_mac[i]) connecting = true;
    else connecting = false;
  }
  if ( connecting ) {
    Serial.println("Connecting to DS-D6 device...");
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
  Serial.println("Connected");
  Serial.print("Discovering D6 Service ... ");

  // If HRM is not found, disconnect and return
  if ( !d6.discover(conn_handle) )
  {
    Serial.println("Found NONE");

    // disconnect since we couldn't find HRM service
    Bluefruit.disconnect(conn_handle);

    return;
  }
  // Once FEETBACK service is found, we continue to discover its characteristic
  Serial.println("Found it");

  Serial.print("Discovering NOTIFY characteristic ... ");
  if ( !d6_notif.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");
    Serial.println("Notify characteristic is mandatory but not found");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  Serial.println("Found it");

  // Measurement is found, continue to look for option Body Sensor Location
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.body_sensor_location.xml
  // Body Sensor Location is optional, print out the location in text if present
  Serial.print("Discovering WRITE characteristic ... ");
  if ( d6_write.discover() )
  {
    Serial.println("Found it");
  } else
  {
    Serial.println("Found NONE");
  }

  // Reaching here means we are ready to go, let's enable notification on measurement chr
  if ( d6_notif.enableNotify() )
  {
    Serial.println("Ready to receive D6 Notify value");
  } else
  {
    Serial.println("Couldn't enable notify for DSD6. Increase DEBUG LEVEL for troubleshooting.");
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

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void d6_notify_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{
  Serial.print("RxBle: ");
  char answer[len] = {};
  memcpy(&answer, data, len);
  for (int i = 0; i < len; i++) {
    Serial.print(answer[i]);
  }
}

void send_ble_cmd(String cmd) {
  Serial.print("TxBle: ");
  Serial.println(cmd);
  char buf[12] = {};
  cmd.toCharArray(buf, 12);
  d6_write.write(buf, 12);
  d6_write.write8(10);
}

void loop() {
  if ( Serial.available() )
  {
    char str[20 + 1] = { 0 };
    Serial.readBytes(str, 20);
    send_ble_cmd(str);
  }
}
