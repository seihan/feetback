#include <bluefruit.h>

const uint8_t feetback_service_uuid[16]   = { "FEETBACKSERVICE" }; // 15 chars + '\0' '00454349-5652-4553-4B43-414254454546'
//const uint8_t feetback_char_data_uuid[16] = { "FEETBACK___DATA" }; // '00415441-445F-5F5F-4B43-414254454546'
const uint8_t feetback_char_bala_uuid[16] = { "FEETBACKBALANCE" }; // '0045434E-414C-4142-4B43-414254454546'
const uint8_t feetback_char_meta_uuid[16] = { "FEETBACK___META" }; // '00415445-4D5F-5F5F-4B43-414254454546'


BLEClientService        feetback = BLEClientService(BLEUuid(feetback_service_uuid));
//BLEClientCharacteristic ftb_data = BLEClientCharacteristic(BLEUuid(feetback_char_data_uuid));
BLEClientCharacteristic ftb_bala = BLEClientCharacteristic(BLEUuid(feetback_char_bala_uuid));
BLEClientCharacteristic ftb_meta = BLEClientCharacteristic(BLEUuid(feetback_char_meta_uuid));

#define RESOLUTION 10
const int outputPins[ 4 ] = { 28, 29, 30, 31 };
uint16_t balance[ 2 ] = { 0, 0 };
uint16_t pwmvalues[ 2 ] = { 0, 0 };
// functio prototypes
void nfc_to_gpio();
void intro();

void setup() {
  nfc_to_gpio();
  // configure  PWM
  HwPWM0.addPin( 28 );
  HwPWM1.addPin( 29 );

  // Enable PWM modules with 15-bit resolutions(max) but different clock div
  HwPWM0.begin();
  HwPWM0.setResolution(RESOLUTION);
  HwPWM0.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_128); // freq = 16MHz
  HwPWM0.writePin(28, 0, false);

  HwPWM1.begin();
  HwPWM1.setResolution(RESOLUTION);
  HwPWM1.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_128); // freq = 125kHz
  HwPWM1.writePin(29, 0, false);

  // turn off the leaving output pins
  for (int i = 2; i < 4; i++) {
    pinMode(outputPins[ i ], OUTPUT);
    digitalWrite(outputPins[ i ], LOW);
  }

  // configure Serial
  Serial.begin(115200);
  Serial.println("FEETBACK Client Test");
  Serial.println("--------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Ftb Actuator - Right");

  // Initialize HRM client
  feetback.begin();

  // Initialize client characteristics of HRM.
  // Note: Client Char will be added to the last service that is begin()ed.
  ftb_meta.begin();

  // set up callback for receiving measurement
  ftb_bala.setNotifyCallback(feetback_notify_callback);
  ftb_bala.begin();

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
  Bluefruit.Scanner.start(0);                   // // 0 = Don't stop scanning after n seconds
  intro();
}

void loop() {
  if ( Serial.available() > 0 ) {
    char input = Serial.read();
    switch ( input ) {
      case 'u' :
        Serial.print("Up, new PWMvalue = ");
        for (int i = 0; i < 2; i++) {
          pwmvalues[i] += 100;
          if ( pwmvalues[i] > bit(RESOLUTION) - 1 ) pwmvalues[i] = bit(RESOLUTION) - 1;
        }
        Serial.println(pwmvalues[0]);
        HwPWM0.writePin(28, pwmvalues[0], false);
        HwPWM1.writePin(29, pwmvalues[1], false);
        break;
      case 'd' :
        Serial.print("Down, new PWMvalue = ");
        for (int i = 0; i < 2; i++) {
          pwmvalues[i] -= 100;
          if ( pwmvalues[i] >= UINT_LEAST16_MAX - 97 ) pwmvalues[i] = 0;
        }
        Serial.println(pwmvalues[0]);
        HwPWM0.writePin(28, pwmvalues[0], false);
        HwPWM1.writePin(29, pwmvalues[1], false);
        break;
      default :
        break;
    }
  }
}

void nfc_to_gpio() {
  Serial.println("Bluefruit52 NFC to GPIO Pin Config");
  Serial.println("----------------------------------\n");
  if ((NRF_UICR->NFCPINS & UICR_NFCPINS_PROTECT_Msk) == (UICR_NFCPINS_PROTECT_NFC << UICR_NFCPINS_PROTECT_Pos)) {
    Serial.println("Fix NFC pins");
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_UICR->NFCPINS &= ~UICR_NFCPINS_PROTECT_Msk;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    Serial.println("Done");
    delay(500);
    NVIC_SystemReset();
  }

}

void intro() {
  for (int i = 0; i < 3; i++); {
    HwPWM0.writePin(28, bit(RESOLUTION) - 1, false);
    HwPWM1.writePin(29, bit(RESOLUTION) - 1, false);
    delay(500);
    HwPWM0.writePin(28, 0, false);
    HwPWM1.writePin(29, 0, false);
    delay(500);
  }
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
  Serial.print("Discovering FEETBACK Service ... ");

  // If HRM is not found, disconnect and return
  if ( !feetback.discover(conn_handle) )
  {
    Serial.println("Found NONE");

    // disconnect since we couldn't find HRM service
    Bluefruit.disconnect(conn_handle);

    return;
  }

  // Once FEETBACK service is found, we continue to discover its characteristic
  Serial.println("Found it");

  Serial.print("Discovering Balance characteristic ... ");
  if ( !ftb_bala.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");
    Serial.println("Balance characteristic is mandatory but not found");
    Bluefruit.disconnect(conn_handle);
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

    // feetback sensor location value is 8 bit
    const char* location_names[] = { "Left", "Right" };

    // Read 8-bit BSLC value from peripheral
    uint8_t loc_value = ftb_meta.read8();

    Serial.print("Body Location Sensor: ");
    Serial.println(location_names[loc_value]);
  } else
  {
    Serial.println("Found NONE");
  }

  // Reaching here means we are ready to go, let's enable notification on measurement chr
  if ( ftb_bala.enableNotify() )
  {
    Serial.println("Ready to receive FEETBACK Balance value");
  } else
  {
    Serial.println("Couldn't enable notify for FEETBACK Balance. Increase DEBUG LEVEL for troubleshooting");
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

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  intro();
}

/**
   Hooked callback that triggered when a measurement value is sent from peripheral
   @param chr   Pointer client characteristic that even occurred,
                in this example it should be hrmc
   @param data  Pointer to received data
   @param len   Length of received data
*/
void feetback_notify_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{

  if (len != sizeof(balance)) {
    Serial.print("Received unknown data of length ");
    Serial.print(len);
    Serial.print(" (Expected: ");
    Serial.print(sizeof(balance));
    Serial.println(")");
    return;
  }

  Serial.print("FEETBACK Balance: ");

  memcpy(&balance[0], data, 2);
  memcpy(&balance[1], data + 2, 2);

  Serial.print(balance[0]);
  Serial.print("\t");
  Serial.println(balance[1]);

  for (int i = 0; i < 2; i++) {
    pwmvalues[i] = constrain( balance[i], 0, UINT_LEAST16_MAX);
    pwmvalues[i] = map( pwmvalues[i], 0, UINT_LEAST16_MAX, 0, bit(RESOLUTION) - 1 );
  }
  Serial.print(pwmvalues[0]);
  Serial.print("\t");
  Serial.println(pwmvalues[1]);
  HwPWM0.writePin(28, pwmvalues[0], false);
  HwPWM1.writePin(29, pwmvalues[1], false);
}
