#include <bluefruit.h>

const uint8_t feetback_service_uuid[16]   = { "FEETBACKSERVICE" }; // 15 chars + '\0' '00454349-5652-4553-4B43-414254454546'
const uint8_t feetback_char_data_uuid[16] = { "FEETBACK___DATA" }; // '00415441-445F-5F5F-4B43-414254454546'
const uint8_t feetback_char_bala_uuid[16] = { "FEETBACKBALANCE" }; // '0045434E-414C-4142-4B43-414254454546'
const uint8_t feetback_char_meta_uuid[16] = { "FEETBACK___META" }; // '00415445-4D5F-5F5F-4B43-414254454546'

BLEService        feetback = BLEService(BLEUuid(feetback_service_uuid));
BLECharacteristic ftb_data = BLECharacteristic(BLEUuid(feetback_char_data_uuid));
BLECharacteristic ftb_bala = BLECharacteristic(BLEUuid(feetback_char_bala_uuid));
BLECharacteristic ftb_meta = BLECharacteristic(BLEUuid(feetback_char_meta_uuid));

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

#include <SPI.h>

//#define HWFC  true
#define MAX_VALUES 25
#define MEASURED_VALUES 956

#include "protocol.h"
#include "toplist.h"
//#include "sole_fsr956_feather.h"
#include "sole_fsr956_bl652.h"
#include "sole.h"

struct message_t msg;
uint16_t data[MEASURED_VALUES];
const int LED2_PIN = 19;
uint16_t balance[ 2 ] = { 0, 0 };

struct smaller_measurement {
  bool operator()(measure_t& e, measure_t& other) {
    return e.value < other.value;
  }
};

toplist<MAX_VALUES, measure_t, smaller_measurement> top;

// Advanced function prototypes
void startAdv(void);
void setup_feetback(void);
void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
void cccd_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t cccd_value);


void setup()
{
  Serial.begin(115200);
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
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Hermann Elektro");
  bledis.setModel("Feetback");
  bledis.begin();

  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  blebas.write(100);

  // Setup the Feetback service using
  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the Feetback Service");
  setup_feetback();

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
  startAdv();

  Serial.println("Ready Player One!!!");
  Serial.println("\nAdvertising");

  Serial.print("Message size (bytes): ");
  Serial.println(sizeof(msg));
  digitalWrite(LED2_PIN, HIGH);
  analogReference(AR_INTERNAL_1_2);
  analogReadResolution(14);
  pinMode(MuxDC_PIN, OUTPUT);
  pinMode(MuxADC_PIN, OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(DC_PINS[ i ], OUTPUT);
  for (int i = 0; i < 5; i++) pinMode(ADC_PINS[ i ], INPUT);
  SPI.begin();
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
  ftb_data.setFixedLen(sizeof(msg.data));
  ftb_data.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  ftb_data.begin();

  // Configure the Feetback Balance characteristic
  // Properties = Notify
  // B0   = UINT16 - Front
  // B1   = UINT16 - Rear
  ftb_bala.setProperties(CHR_PROPS_NOTIFY);
  ftb_bala.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  ftb_bala.setFixedLen(sizeof(balance));
  ftb_bala.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  ftb_bala.begin();

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
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

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

void cccd_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t cccd_value)
{
  // Display the raw request packet
  Serial.print("CCCD Updated: ");
  //Serial.printBuffer(request->data, request->len);
  Serial.print(cccd_value);
  Serial.println("");

  // Check the characteristic this CCCD update is associated with in case
  // this handler is used for multiple CCCD records.
  if (chr->uuid == ftb_data.uuid) {
    if (chr->notifyEnabled(conn_hdl)) {
      Serial.println("Feetback Measurement 'Notify' enabled");
    } else {
      Serial.println("Feetback Measurement 'Notify' disabled");
    }
  }

  // Check the characteristic this CCCD update is associated with in case
  // this handler is used for multiple CCCD records.
  if (chr->uuid == ftb_bala.uuid) {
    if (chr->notifyEnabled(conn_hdl)) {
      Serial.println("Feetback Balance 'Notify' enabled");
    } else {
      Serial.println("Feetback Balance 'Notify' disabled");
    }
  }
}

void loop()
{
  uint16_t nval = read_sole(data); // read values

  top.clear();
  for (uint16_t i = 0; i != nval; i++) {
    top.add(measure_t{i, data[i]});
  }
  msg.length = MAX_VALUES;
  nval = 0;
  for (uint16_t& val : balance) val = 0;
  for (const measure_t& val : top) {
    msg.data[nval++] = val; // add top values to payload
    if ( msg.data[nval - 1].offset < 428 ) { // sum rear values to balance
      balance[ 0 ] += msg.data[nval - 1].value;
      if ( (UINT_LEAST16_MAX - balance[ 0 ]) < 0 ) balance[ 0 ] = UINT_LEAST16_MAX;
    } else {                                // sum front values to balance
      balance[ 1 ] += msg.data[nval - 1].value;
      if ( (UINT_LEAST16_MAX - balance[ 1 ]) < 0 ) balance[ 1 ] = UINT_LEAST16_MAX;
    }
  }

  if ( Bluefruit.connected() ) {
    // Note: We use .notify instead of .write!
    // If it is connected but CCCD is not enabled
    // The characteristic's value is still updated although notification is not sent
    // Message header and length are not actuallly required for this characteristic
    if ( ! ftb_data.notify(msg.data, sizeof(msg.data)) ) {
      Serial.println("ERROR: Notify not set in the CCCD or not connected!");
    }
    if ( ! ftb_bala.notify(balance, sizeof(balance)) ) {
      Serial.println("ERROR: Notify not set in the CCCD or not connected!");
    }
  }

  // send_to_serial(&msg); // transmit values
}
