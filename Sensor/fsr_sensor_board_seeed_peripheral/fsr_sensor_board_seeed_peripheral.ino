
/*********************************************************************
  This is an example for our nRF52 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************
  FSR Sensor Board

  Read 956 pressure values and transmit 2 (front, rear) via BLE UART

  Peripheral -> Central (Smart Device)

********************************************************************/
#include <bluefruit.h>
#include <SPI.h>

// BLE Service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

//SPISettings settings(250000, MSBFIRST, SPI_MODE0); // 250000 500000 1000000 2000000 4000000

#define VBAT_PIN (A7)
#define MAX_SERIAL 956
#define MAX_BLE 2
#define DXTIME 100
#define AXTIME 10
#define ADCTIME 0
#define ADCOUNTS 1
#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096
#define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider

#include "protocol.h"

#include "sole_fsr956.h"
#include "sole.h"

struct message_t msgsl; // -> Serial UART
struct message_t msgble; // -> BLE UART

bool ready = false; // ready for transmitting?
char serial_buffer[21]; // Serial input


void setup()
{
  //    NRF_POWER ->DCDCEN = 1;
  //    NRF_PWM0  ->ENABLE = 0; //disable all pwm instance
  //    NRF_PWM1  ->ENABLE = 0;
  //    NRF_PWM2  ->ENABLE = 0;
  //    NRF_TWIM1 ->ENABLE = 0; //disable TWI Master
  //    NRF_TWIS1 ->ENABLE = 0; //disable TWI Slave
  //    NRF_SPI1 -> ENABLE = 0; //disable SPI
  //    NRF_SPI2 -> ENABLE = 0; //disable SPI
  //    NRF_NFCT->TASKS_DISABLE = 1; //disable NFC, confirm this is the right way

  Serial.begin(230400);
  Serial.println("FSR Sensor Board - Wireless BLE UART");
  Serial.println("---------------------------\n");

  analogReference(AR_INTERNAL_1_2); //AR_INTERNAL_1_2);
  analogReadResolution(12);
  pinMode(MuxDC_PIN, OUTPUT);
  pinMode(MuxADC_PIN, OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(DC_PINS[ i ], OUTPUT);
  for (int i = 0; i < 5; i++) pinMode(ADC_PINS[ i ], INPUT);

  SPI.begin();

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Left FSR 956");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Configure and Start Device Information Service
  bledis.setManufacturer("Hermann Elektro");
  bledis.setModel("521S");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();

  // Set up and start advertising
  startAdv();

  msgsl.length = MAX_SERIAL;
  msgble.length = MAX_BLE;
}

int pins[2] = {0, 0};
unsigned long previousMillis = 0;
const long second = 1000;
int counter = 0;

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

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

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 5 * second) {
    previousMillis = currentMillis;
    //    Serial.println(counter);
    //    counter = 0;
    get_battery_charge();
  }
  //  if (read_serial()) {
  //    parse_serial( pins );
  //    ready = true;
  //  }
  //  if (ready) {
  //    Serial.println(read_sole(pins[0], pins[1]));
  //    delay(200);
  //  }

  msgsl.length = read_sole(msgsl.data);

  for (uint16_t& val : msgble.data) val = 0;

  if (Serial.available()) send_to_serial(&msgsl);

  for (int i = 0; i < msgsl.length; i++) {
    if ( i <= 428 ) {
      if ( msgble.data[ 0 ] < msgsl.data[ i ] ) msgble.data[ 0 ] = msgsl.data[ i ];
    }
    else {
      if ( msgble.data[ 1 ] < msgsl.data[ i ] ) msgble.data[ 1 ] = msgsl.data[ i ];
    }
  }

  if (ready) {
    //bleuart.println(String(constrain(msgble.data[ 0 ], 0, 4095)) + ";" + String(constrain(msgble.data[ 1 ], 0, 4095)));
    send_to_central(&msgble);//transmit values
  } else {
    // Request CPU to enter low-power mode until an event/interrupt occurs
    waitForEvent();
  }
  //counter++;
}

bool read_serial()
{
  static byte index;

  while (Serial.available())
  {
    char c = Serial.read();

    if (c >= 32 && index < 21 - 1)
    {
      serial_buffer[index++] = c;
    }
    else if (c == '\n' && index > 0)
    {
      serial_buffer[index] = '\0';
      index = 0;
      return true;
    }
  }
  return false;
}

void parse_serial( int * pos)
{
  int adc = atoi(strtok(serial_buffer, ";:,"));
  int dc = atoi(strtok(NULL, ";:,"));

  Serial.print(F("row = ")); Serial.println(adc);
  Serial.print(F("col = ")); Serial.println(dc);
  pins[ 0 ] = adc;
  pins[ 1 ] = dc;
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  ready = true;

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

  ready = false;

  Serial.println();
  Serial.println("Disconnected");
}

int readVBAT(void) {
  int raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(VBAT_PIN);

  // Set the ADC back to the default settings
  analogReference(AR_INTERNAL_1_2);
  //analogReadResolution(12);

  return raw;
}

uint8_t mvToPercent(float mvolts) {
  uint8_t battery_level;

  if (mvolts >= 3000)
  {
    battery_level = 100;
  }
  else if (mvolts > 2900)
  {
    battery_level = 100 - ((3000 - mvolts) * 58) / 100;
  }
  else if (mvolts > 2740)
  {
    battery_level = 42 - ((2900 - mvolts) * 24) / 160;
  }
  else if (mvolts > 2440)
  {
    battery_level = 18 - ((2740 - mvolts) * 12) / 300;
  }
  else if (mvolts > 2100)
  {
    battery_level = 6 - ((2440 - mvolts) * 6) / 340;
  }
  else
  {
    battery_level = 0;
  }

  return battery_level;
}

void get_battery_charge() {
  // Get a raw ADC reading
  int vbat_raw = readVBAT();

  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_raw * VBAT_MV_PER_LSB);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095),
  // VBAT voltage divider is 2M + 0.806M, which needs to be added back
  float vbat_mv = (float)vbat_raw * VBAT_MV_PER_LSB * VBAT_DIVIDER_COMP;
  // Display the results
  //  Serial.print("ADC = ");
  //  Serial.print(vbat_raw * VBAT_MV_PER_LSB);
  //  Serial.print(" mV (");
  //  Serial.print(vbat_raw);
  //  Serial.print(") ");
  //  Serial.print("LIPO = ");
  //  Serial.print(vbat_mv);
  //  Serial.print(" mV (");
  //  Serial.print(vbat_per);
  //  Serial.println("%)");
  blebas.write(vbat_per);
}
