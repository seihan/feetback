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
#include <bluefruit.h>

const uint8_t feetback_service_uuid[16]   = { "FEETBACKSERVICE" }; // 15 chars + '\0' '00454349-5652-4553-4B43-414254454546'
const uint8_t feetback_char_data_uuid[16] = { "FEETBACK___DATA" }; // '00415441-445F-5F5F-4B43-414254454546'
const uint8_t feetback_char_meta_uuid[16] = { "FEETBACK___META" }; // '00415445-4D5F-5F5F-4B43-414254454546'
BLEService        feetback = BLEService(BLEUuid(feetback_service_uuid));
BLECharacteristic ftb_data = BLECharacteristic(BLEUuid(feetback_char_data_uuid));
BLECharacteristic ftb_meta = BLECharacteristic(BLEUuid(feetback_char_meta_uuid));

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

struct feet_t
{
  uint8_t x;
  uint8_t y;
  uint16_t val[9];
};

// Advanced function prototypes
void startAdv(void);
void setup_feetback(void);
void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);

void setup()
{
  Serial.begin(115200);

  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("FEETBACK Service Test");
  Serial.println("-----------------------\n");

  // Initialise the Bluefruit module
  Serial.println("Initialise the nRF52 module");
  Bluefruit.begin();

 // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  
   // Set the advertised device name (keep it short!)
  Serial.println("Setting Device Name to 'FEETBACK Prph'");
  Bluefruit.setName("FEETBACK Prph");

  // Set the connect/disconnect callback handlers
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Hermann Elektro");
  bledis.setModel("Feetback Seeed");
  bledis.begin();

  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  blebas.write(100);

  // Setup the Heart Rate Monitor service using
  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the Feetback Service");
  setup_feetback();

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
  startAdv();

  Serial.println("Ready Player One!!!");
  Serial.println("\nAdvertising");
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include Feetback Service UUID
  Bluefruit.Advertising.addService(feetback);

  // Include Name
  Bluefruit.Advertising.addName();

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

void setup_feetback(void)
{
  // Configure the Feetback service
  feetback.begin();

  // Note: You must call .begin() on the BLEService before calling .begin() on
  // any characteristic(s) within that service definition.. Calling .begin() on
  // a BLECharacteristic will cause it to be added to the last BLEService that
  // was 'begin()'ed!

  // Configure the Feetback Measurement characteristic
  // Properties = Notify
  // B0   = UINT8 - Position X
  // B1   = UINT8 - Position Y
  // B2:3 = UINT16 - Value
  ftb_data.setProperties(CHR_PROPS_NOTIFY);
  ftb_data.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  ftb_data.setFixedLen(sizeof(feet_t));
  ftb_data.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  ftb_data.begin();

  //  feetback sensor location value is 8 bit
  //  static const char* location_names[] = { "Left", "Right" };
  ftb_meta.setProperties(CHR_PROPS_READ);
  ftb_meta.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  ftb_meta.setFixedLen(1);
  ftb_meta.begin();
  ftb_meta.write8(1);    // Set the characteristic to 'Right' (1)
}

void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
   Callback invoked when a connection is dropped
   @param conn_handle connection where this event happens
   @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  Serial.println("Advertising!");
}

void cccd_callback(BLECharacteristic& chr, uint16_t cccd_value)
{
  // Display the raw request packet
  Serial.print("CCCD Updated: ");
  //Serial.printBuffer(request->data, request->len);
  Serial.print(cccd_value);
  Serial.println("");

  // Check the characteristic this CCCD update is associated with in case
  // this handler is used for multiple CCCD records.
  if (chr.uuid == ftb_data.uuid) {
    if (chr.notifyEnabled()) {
      Serial.println("Feetback Measurement 'Notify' enabled");
    } else {
      Serial.println("Feetback Measurement 'Notify' disabled");
    }
  }
}

uint8_t x = 0;
uint8_t y = 0;
uint16_t val = 0xfeba;

void loop()
{
  digitalToggle(LED_RED);

  if ( Bluefruit.connected() ) {
    val = val << 1 | (val & 0x8000) >> 15;
    feet_t feetdata = { x++, y--, val };

    // Note: We use .notify instead of .write!
    // If it is connected but CCCD is not enabled
    // The characteristic's value is still updated although notification is not sent
    if ( ftb_data.notify(&feetdata, sizeof(feetdata)) ) {  //ftb_data.notify(&feetdata, sizeof(feetdata)) ) {
      //      Serial.print("Feetback Measurement updated to: "); Serial.println(val);
    } else {
      Serial.println("ERROR: Notify not set in the CCCD or not connected!");
    }
  }

  // delay(100);
}