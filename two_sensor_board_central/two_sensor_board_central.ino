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

/* This sketch demonstrates the central API() that allows you to connect
   to multiple peripherals boards (Bluefruit nRF52 in peripheral mode, or
   any Bluefruit nRF51 boards).

   One or more Bluefruit boards, configured as a peripheral with the
   bleuart service running are required for this demo.

   This sketch will:
    - Read data from the HW serial port (normally USB serial, accessible
      via the Serial Monitor for example), and send any incoming data to
      all other peripherals connected to the central device.
    - Forward any incoming bleuart messages from a peripheral to all of
      the other connected devices.

   It is recommended to give each peripheral board a distinct name in order
   to more easily distinguish the individual devices.

   Connection Handle Explanation
   -----------------------------
   The total number of connections is BLE_MAX_CONN = BLE_PRPH_MAX_CONN + BLE_CENTRAL_MAX_CONN.

   The 'connection handle' is an integer number assigned by the SoftDevice
   (Nordic's proprietary BLE stack). Each connection will receive it's own
   numeric 'handle' starting from 0 to BLE_MAX_CONN-1, depending on the order
   of connection(s).

   - E.g If our Central board connects to a mobile phone first (running as a peripheral),
     then afterwards connects to another Bluefruit board running in peripheral mode, then
      the connection handle of mobile phone is 0, and the handle for the Bluefruit
      board is 1, and so on.
*/

/* LED PATTERNS
   ------------
   LED_RED   - Blinks pattern changes based on the number of connections.
   LED_BLUE  - Blinks constantly when scanning
*/
#define MAX_CONNECTIONS 1 // number of peripherals
#define MAX_VALUES 2  // arraysize for bleuart
#define SENSORS 2 // adc
#define READINGS 3 // smooth adc reading 
#define FILENAME    "/calibration.txt" // calibration data

#include <bluefruit.h>
#include <Nffs.h>

NffsFile file;
// Peripheral uart service
BLEUart bleuart; // peripheral role <-> central device

#include "protocol.h"

static struct message_t msg;

struct {
  int maxValue0 = 500; //default values
  int maxValue1 = 350;
} calibrationData;

unsigned long previousMillis = 0;
const long interval = 250; // Serial.print interval

// input pins
const int inputPins[ SENSORS ] = {
  A0, A1
};

// Struct containing peripheral info
typedef struct
{
  char name[32];

  uint16_t conn_handle;

  // Each prph need its own central uart client
  BLEClientUart clientUart; // central role <-> peripheral devices
} prph_info_t;

/* Peripheral info array (one per peripheral device)

   There are 'MAX_CONNECTIONS' central connections, but the
   the connection handle can be numerically larger (for example if
   the peripheral role is also used, such as connecting to a mobile
   device). As such, we need to convert connection handles <-> the array
   index where appropriate to prevent out of array accesses.

   Note: One can simply declares the array with BLE_MAX_CONN and use connection
   handle as index directly with the expense of SRAM.
*/
prph_info_t prphs[MAX_CONNECTIONS];

// Software Timer for blinking the RED LED
SoftwareTimer blinkTimer;
uint8_t connection_num = 0; // for blink pattern

void setup()
{
  for (int i = 0; i < SENSORS; i++) {
    pinMode(inputPins[ i ], INPUT);
  }
  // ADC config
  analogReference(AR_INTERNAL_3_0); //AR_VDD4);
  analogReadResolution(12);
  Serial.begin(115200);

  Serial.println("Left Two Sensor Sole Central UART");
  Serial.println("-----------------------------------------\n");

  // Enable both peripheral and central
  Bluefruit.begin(true, true);
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("LeftSensorSole");

  // Callbacks for Peripheral
  Bluefruit.setConnectCallback(prph_connect_callback);
  Bluefruit.setDisconnectCallback(prph_disconnect_callback);

  // Configure and Start BLE Uart Service
  bleuart.begin();
  bleuart.setRxCallback(prph_bleuart_rx_callback);

  // Init peripheral pool
  for (uint8_t idx = 0; idx < MAX_CONNECTIONS; idx++)
  {
    // Invalid all connection handle
    prphs[idx].conn_handle = BLE_CONN_HANDLE_INVALID;

    // All of BLE Central Uart Serivce
    prphs[idx].clientUart.begin();
    prphs[idx].clientUart.setRxCallback(clientUart_rx_callback);
  }

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Don't use active scan (used to retrieve the optional scan response adv packet)
     - Start(0) = will scan forever since no timeout is given
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false);       // Don't request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  msg.length = MAX_VALUES;

  // Initialize Nffs
  Nffs.begin();
  if (Nffs.readFile(FILENAME, &calibrationData, sizeof(calibrationData) )) {  // file existed
    Serial.println("Calibration data available.");
  }
  else {
    Serial.println("No calibration data available, using default values.");
  }
  Serial.print(calibrationData.maxValue0);
  Serial.print("\t");
  Serial.println(calibrationData.maxValue1);
  // Set up and start advertising
  startAdv();
}

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

void prph_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Prph] Connected to ");
  Serial.println(peer_name);
}

void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("[Prph] Disconnected");
}

void sendtoPeripherals(unsigned char* str, uint16_t len)
{
  for (uint8_t id = 0; id < MAX_CONNECTIONS; id++)
  {
    prph_info_t* peer = &prphs[id];

    if ( peer->clientUart.discovered() )
    {
      peer->clientUart.write(str, len);
    }
  }
}

/**
   Callback invoked when scanner picks up an advertising packet
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising data contains the BleUart service UUID
  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
  {
    Serial.print("BLE UART service detected. Connecting ... ");

    // Connect to the device with bleuart service in advertising packet
    Bluefruit.Central.connect(report);
  }
}

/**
   Callback invoked when an connection is established
   @param conn_handle
*/
void connect_callback(uint16_t conn_handle)
{
  // Find an available ID to use
  int id  = findConnHandle(BLE_CONN_HANDLE_INVALID);

  // Eeek: Exceeded the number of connections !!!
  if ( id < 0 ) return;

  prph_info_t* peer = &prphs[id];
  peer->conn_handle = conn_handle;

  Bluefruit.Gap.getPeerName(conn_handle, peer->name, 32);
  Serial.print("Connected to ");
  Serial.println(peer->name);

  if (peer->name[0] != 'L') {
    Serial.println("Not the device you are looking for...");
    Bluefruit.Central.disconnect(conn_handle);
  } else {
    Serial.print("Discovering BLE UART service ... ");

    if ( peer->clientUart.discover(conn_handle) )
    {
      Serial.println("Found it");
      Serial.println("Enabling TXD characteristic's CCCD notify bit");
      peer->clientUart.enableTXD();

    } else
    {
      Serial.println("Found ... NOTHING!");

      // disconect since we couldn't find bleuart service
      Bluefruit.Central.disconnect(conn_handle);
    }
  }

  // will be decremented on disconnect
  connection_num++;

  Serial.print("Connections: ");
  Serial.print(connection_num);
  Serial.print("/");
  Serial.println(MAX_CONNECTIONS);

  if (connection_num < MAX_CONNECTIONS) {
    Serial.println("Continue scanning for more peripherals");
    Bluefruit.Scanner.start(0);
  } else {
    Serial.println("All peripherals connected");
    // Scanner stopped
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

  connection_num--;

  // Mark the ID as invalid
  int id  = findConnHandle(conn_handle);

  // Non-existant connection, something went wrong, DBG !!!
  if ( id < 0 ) return;

  // Mark conn handle as invalid
  prphs[id].conn_handle = BLE_CONN_HANDLE_INVALID;

  Serial.print(prphs[id].name);
  Serial.println(" disconnected!");

  Serial.print("Connections: ");
  Serial.print(connection_num);
  Serial.print("/");
  Serial.println(MAX_CONNECTIONS);
  Serial.println("Continue scanning for more peripherals");
  Bluefruit.Scanner.start(0);
}

void prph_bleuart_rx_callback(void)
{
  // Forward data from Mobile to our peripheral
  char str[20 + 1] = { 0 };
  bleuart.read(str, 20);

  Serial.print("[Prph] RX: ");
  Serial.println(str);

  if (str[ 0 ] == 'C') {
    Serial.println("Starting calibration...");
    calibration();
  }

  /*  if ( clientUart.discovered() )
    {
      clientUart.print(str);
    } else
    {
      bleuart.println("[Prph] Central role not connected");
    }*/
}

/**
   Callback invoked when BLE UART data is received
   @param uart_svc Reference object to the service where the data
   arrived.
*/

void clientUart_rx_callback(BLEClientUart& uart_svc)
{
  // uart_svc is prphs[conn_handle].bleuart
  uint16_t conn_handle = uart_svc.connHandle();

  int id = findConnHandle(conn_handle);
  prph_info_t* peer = &prphs[id];

  // Print sender's name
  /*  Serial.printf("[From %s]: ", peer->name);

    // Read then forward to all peripherals
    while ( uart_svc.available() )
    {
      // default MTU with an extra byte for string terminator
      char buf[20+1] = { 0 };


      if ( uart_svc.read(buf,sizeof(buf)-1) )
      {
        Serial.println(buf);
        //sendAll(buf, sizeof(buf));
      }
    } */
}


/**
   Find the connection handle in the peripheral array
   @param conn_handle Connection handle
   @return array index if found, otherwise -1
*/
int findConnHandle(uint16_t conn_handle)
{
  for (int id = 0; id < MAX_CONNECTIONS; id++)
  {
    if (conn_handle == prphs[id].conn_handle)
    {
      return id;
    }
  }

  return -1;
}

/**
   Software Timer callback is invoked via a built-in FreeRTOS thread with
   minimal stack size. Therefore it should be as simple as possible. If
   a periodically heavy task is needed, please use Scheduler.startLoop() to
   create a dedicated task for it.

   More information http://www.freertos.org/RTOS-software-timer.html
*/
void blink_timer_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;

  // Period of sequence is 10 times (1 second).
  // RED LED will toggle first 2*n times (on/off) and remain off for the rest of period
  // Where n = number of connection
  static uint8_t count = 0;

  if ( count < 2 * connection_num) digitalToggle(LED_RED);

  count++;
  if (count >= 10) count = 0;
}

void calibration(void) { // calibration: LED_RED HIGH -> 1st sensor, LED_RED LOW -> break, LED_RED HIGH -> 2nd sensor
  unsigned long duration = 0;
  previousMillis = millis();
  while (duration < 3000) {
    digitalWrite(LED_RED, HIGH);
    int tmp = analogRead(inputPins[ 0 ]);
    if (calibrationData.maxValue0 != 0 || calibrationData.maxValue0 < tmp) {
      calibrationData.maxValue0 = tmp;
    }
    duration = millis() - previousMillis;
  }
  Serial.print("New max value for sensor 1 = ");
  Serial.println(calibrationData.maxValue0);
  while (duration < 6000) {
    digitalWrite(LED_RED, LOW);
    duration = millis() - previousMillis;
  }
  while (duration < 9000) {
    digitalWrite(LED_RED, HIGH);
    int tmp = analogRead(inputPins[ 1 ]);
    if (calibrationData.maxValue1 != 0 || calibrationData.maxValue1 < tmp) {
      calibrationData.maxValue1 = tmp;
    }
    duration = millis() - previousMillis;
  }
  digitalWrite(LED_RED, LOW);
  Serial.print("New max value for sensor 2 = ");
  Serial.println(calibrationData.maxValue1);
  Serial.print("Open " FILENAME " file to write ... ");

  if (Nffs.writeFile(FILENAME, &calibrationData, sizeof(calibrationData) )) {
    Serial.println("OK");
  }
  while (duration < 10000){
    duration = millis() - previousMillis;
  }
}

void loop()
{
  for (uint16_t& val : msg.data) { //reset values to zero
    val = 0;
  }
  unsigned long currentMillis = millis();

  int buttonState = 0;         // variable for reading the pushbutton status
  buttonState = analogRead(A2);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if ((buttonState == 4095) || (Serial.read() == 'C')) {
    Serial.println("Starting calibration...");
    calibration();
  }

  for (int i = 0; i < SENSORS; i++) {
    uint16_t tmp = 0;
    for (int r = 0; r < READINGS; r++) {
      tmp += analogRead( inputPins[ i ] );
    }
    delay(1);
    tmp = tmp / READINGS;
    msg.data[ i ] = tmp;
  }
  //set constrains
  msg.data[ 0 ] = constrain(msg.data[ 0 ], 0, 4095);
  msg.data[ 1 ] = constrain(msg.data[ 1 ], 0, 4095);

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.print(msg.data[ 0 ]);
    Serial.print("\t");
    Serial.println(msg.data[ 1 ]);
  }

  //map values for pwm
  msg.data[ 0 ] = map(msg.data[ 0 ], 0, calibrationData.maxValue0, 0, 255);
  msg.data[ 1 ] = map(msg.data[ 1 ], 0, calibrationData.maxValue1, 0, 255);

  // First check if we are connected to any peripherals
  if ( Bluefruit.Central.connected() )
  {
    send_to_prphs(&msg); // central -> peripherals
  }
}
