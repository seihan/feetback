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

BLEClientDis  clientDis;  // device information client
BLEClientService        bas_client = BLEClientService(BLEUuid(UUID16_SVC_BATTERY));    // Battery service client
BLEClientService        hid_client = BLEClientService(BLEUuid(UUID16_SVC_HUMAN_INTERFACE_DEVICE));    // HID client
BLEClientCharacteristic hid_inf = BLEClientCharacteristic(BLEUuid(UUID16_CHR_HID_INFORMATION));       // READ
BLEClientCharacteristic hid_ctrl_pt = BLEClientCharacteristic(BLEUuid(UUID16_CHR_HID_CONTROL_POINT)); // WRITE
BLEClientCharacteristic prtcl_mode = BLEClientCharacteristic(BLEUuid(UUID16_CHR_PROTOCOL_MODE));      // READ, WRITE NO RESPONSE
BLEClientCharacteristic rprt0 = BLEClientCharacteristic(BLEUuid(UUID16_CHR_REPORT));                   // NOTIFY
BLEClientCharacteristic rprt1 = BLEClientCharacteristic(BLEUuid(UUID16_CHR_REPORT));                   // READ, WRITE NO RESPONSE
BLEClientCharacteristic rprt2 = BLEClientCharacteristic(BLEUuid(UUID16_CHR_REPORT));                   // READ, WRITE NO RESPONSE
BLEClientCharacteristic rprt_map = BLEClientCharacteristic(BLEUuid(UUID16_CHR_REPORT_MAP));           // READ

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central Custom HRM Example");
  Serial.println("--------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Bluefruit52 Central");

  // Configure Battery client
  bas_client.begin();

  // Configure DIS client
  clientDis.begin();


  hid_client.begin();

  // Initialize client characteristics: hid_inf, hid_ctrl_pt, prtcl_mode, rprt, rprt_map
  hid_inf.begin();  // information

  hid_ctrl_pt.begin();  // control point

  prtcl_mode.begin(); // protocol mode

  rprt0.begin(); // report

  // set up callback
  rprt1.setNotifyCallback(rprt_notify_callback);
  //  rprt.instance = 2;
  rprt1.begin();

  rprt2.begin();

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
  Bluefruit.Scanner.filterUuid(hid_client.uuid);
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
    Serial.printf("HID version: %d.%d\n", hidInfo[0], hidInfo[1]);
*/
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");
  Serial.print("Discovering Human Interface Device Service ... ");


  if ( hid_client.discover(conn_handle) )
  {
    Serial.println("Found it");

    // HID device mostly require pairing/bonding
    if ( !Bluefruit.Gap.requestPairing(conn_handle) )
    {
      Serial.print("Failed to paired");
      return;
    }

  } else
  {
    Serial.println("Found NONE");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  }

  Serial.print("Discovering HID information ... ");
  if ( !hid_inf.discover() )
  {
    Serial.println("not found !!!");
    Serial.println("HID information characteristic is mandatory but not found");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  } else
  {
    Serial.println("Found it");
    uint8_t inf_value = hid_inf.read8();
    Serial.println(inf_value);
  }

  Serial.print("Discovering HID control point ... ");
  if ( !hid_ctrl_pt.discover() )
  {
    Serial.println("Found NONE");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  } else
  {
    Serial.println("Found it");
  }

  Serial.print("Discovering protocol mode ... ");
  if ( !prtcl_mode.discover() )
  {
    Serial.println("Found NONE");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  } else
  {
    Serial.println("Found it");
    prtcl_mode.write8(HID_PROTOCOL_MODE_BOOT);
  }
  //    Serial.printf("HID version: %d.%d\n", hidInfo[0], hidInfo[1]);

  Serial.print("Discovering report 0 characteristic ... ");
  if ( !rprt0.discover() )
  {
    Serial.println("Found NONE");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  } else
  {
    Serial.println("Found it");
    uint8_t value = rprt0.read8();
    Serial.println( value);
  }

  Serial.print("Discovering report 1 characteristic ... ");
  /*if ( !rprt1.discover() )
  {
    Serial.println("Found NONE");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  } else
  {
    Serial.println("Found it");
  }

  Serial.print("Discovering report 2 characteristic ... ");
  if ( !rprt2.discover() )
  {
    Serial.println("Found NONE");
    Bluefruit.Central.disconnect(conn_handle);
    return;
  } else
  {
    Serial.println("Found it");
  }
*/
  // Reaching here means we are ready to go, let's enable notifications

  /*if ( rprt1.enableNotify() )
  {
    Serial.println("Ready to receive HID value");
  } else
  {
    Serial.println("Couldn't enable notify. Increase DEBUG LEVEL for troubleshooting");
  }*/

  /*Serial.print("Discovering report map ... ");
    if ( !rprt_map.discover() )
    {
    Serial.println("Found NONE");
    //    Bluefruit.Central.disconnect(conn_handle);
    return;
    } else
    {
    Serial.println("Found it");
    }*/
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
  Serial.println(reason);
}


/**
   Hooked callback that triggered when a measurement value is sent from peripheral
   @param chr   Pointer client characteristic that even occurred,
                in this example it should be ftb_data
   @param data  Pointer to received data
   @param len   Length of received data
*/

void rprt_notify_callback(BLEClientCharacteristic * chr, uint8_t* data, uint16_t len)
{
  Serial.print("Report value: ");

  if ( data[0] & bit(0) )
  {
    uint16_t value;
    memcpy(&value, data + 1, 2);

    Serial.println(value);
  }
  else
  {
    Serial.println(data[0]);
  }
}
