
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
 * to multiple peripherals boards (Bluefruit nRF52 in peripheral mode, or
 * any Bluefruit nRF51 boards).
 *
 * One or more Bluefruit boards, configured as a peripheral with the
 * bleuart service running are required for this demo.
 *
 * This sketch will: 
 *  - Read data from the HW serial port (normally USB serial, accessible
 *    via the Serial Monitor for example), and send any incoming data to
 *    all other peripherals connected to the central device.
 *  - Forward any incoming bleuart messages from a peripheral to all of
 *    the other connected devices.
 * 
 * It is recommended to give each peripheral board a distinct name in order
 * to more easily distinguish the individual devices.
 * 
 * Connection Handle Explanation
 * -----------------------------
 * The total number of connections is BLE_MAX_CONN = BLE_PRPH_MAX_CONN + MAX_CONNECTIONS.
 * 
 * The 'connection handle' is an integer number assigned by the SoftDevice
 * (Nordic's proprietary BLE stack). Each connection will receive it's own
 * numeric 'handle' starting from 0 to BLE_MAX_CONN-1, depending on the order
 * of connection(s).
 *
 * - E.g If our Central board connects to a mobile phone first (running as a peripheral),
 *   then afterwards connects to another Bluefruit board running in peripheral mode, then
 *    the connection handle of mobile phone is 0, and the handle for the Bluefruit
 *    board is 1, and so on.
 */

/* LED PATTERNS
 * ------------
 * LED_RED   - Blinks pattern changes based on the number of connections.
 * LED_BLUE  - Blinks constantly when scanning
 */

#define MAX_CONNECTIONS 2
#define MAX_VALUES 2
#define ROWS 16
#define COLUMNS 5

#include <bluefruit.h>
#include "protocol.h"

static struct message_t msg;
int duration = 0;
// array to store values
uint16_t values[ ROWS ][ COLUMNS ] = {}; //[rows][columns]
// array to store mean values
uint16_t rowmean[ ROWS ]  = {};
// array to store mean values
uint16_t rowmax[ ROWS ]  = {};
//temporary variable
uint16_t maxi = 0;

// column pins to set column high
const int columnPins[ COLUMNS ] = {
  27, 30, 16, 6, 20
};

//mux control pins
int s0 = 15;
int s1 = 7;
int s2 = 11;
int s3 = 31;

//mux in "SIG" pin
int SIG_pin = A3;

// Struct containing peripheral info
typedef struct
{
  char name[32];

  uint16_t conn_handle;

  // Each prph need its own bleuart client service
  BLEClientUart bleuart;
} prph_info_t;

/* Peripheral info array (one per peripheral device)
 * 
 * There are 'MAX_CONNECTIONS' central connections, but the
 * the connection handle can be numerically larger (for example if
 * the peripheral role is also used, such as connecting to a mobile
 * device). As such, we need to convert connection handles <-> the array
 * index where appropriate to prevent out of array accesses.
 * 
 * Note: One can simply declares the array with BLE_MAX_CONN and use connection
 * handle as index directly with the expense of SRAM.
 */
prph_info_t prphs[MAX_CONNECTIONS];

// Software Timer for blinking the RED LED
SoftwareTimer blinkTimer;
uint8_t connection_num = 0; // for blink pattern

void setup() 
{
  Serial.begin(115200);

  // Initialize blinkTimer for 100 ms and start it
  blinkTimer.begin(100, blink_timer_callback);
  blinkTimer.start();

  Serial.println("Left Sensor Sole Central UART");
  Serial.println("-----------------------------------------\n");
  
  // Enable both peripheral and central
  Bluefruit.begin(false, true);
  Bluefruit.setName("LeftSensorSole");
  
  // Init peripheral pool
  for (uint8_t idx=0; idx<MAX_CONNECTIONS; idx++)
  {
    // Invalid all connection handle
    prphs[idx].conn_handle = BLE_CONN_HANDLE_INVALID;
    
    // All of BLE Central Uart Serivce
    prphs[idx].bleuart.begin();
    prphs[idx].bleuart.setRxCallback(bleuart_rx_callback);
  }

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Don't use active scan (used to retrieve the optional scan response adv packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false);       // Don't request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds
  
  pinMode(s0, OUTPUT); //muxing control pins
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);
  pinMode(SIG_pin, INPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  
  msg.length = MAX_VALUES;
}

/**
 * Helper function to send a string to all connected peripherals
 */
void sendAll(unsigned char* str, uint16_t len)
{
 /* Serial.print("[Send to All]: ");
  Serial.println(str);*/

  for(uint8_t id=0; id < MAX_CONNECTIONS; id++)
  {
    prph_info_t* peer = &prphs[id];

    if ( peer->bleuart.discovered() )
    {
      peer->bleuart.write(str, len);
    }
  }
}

/**
 * Callback invoked when scanner picks up an advertising packet
 * @param report Structural advertising data
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
 * Callback invoked when an connection is established
 * @param conn_handle
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
  
    if ( peer->bleuart.discover(conn_handle) )
    {
      Serial.println("Found it");
      Serial.println("Enabling TXD characteristic's CCCD notify bit");
      peer->bleuart.enableTXD();
  
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
 * Callback invoked when a connection is dropped
 * @param conn_handle
 * @param reason
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

/**
 * Callback invoked when BLE UART data is received
 * @param uart_svc Reference object to the service where the data 
 * arrived.
 */
void bleuart_rx_callback(BLEClientUart& uart_svc)
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
 * Find the connection handle in the peripheral array
 * @param conn_handle Connection handle
 * @return array index if found, otherwise -1
 */
int findConnHandle(uint16_t conn_handle)
{
  for(int id=0; id<MAX_CONNECTIONS; id++)
  {
    if (conn_handle == prphs[id].conn_handle)
    {
      return id;
    }
  }

  return -1;  
}

/**
 * Software Timer callback is invoked via a built-in FreeRTOS thread with
 * minimal stack size. Therefore it should be as simple as possible. If
 * a periodically heavy task is needed, please use Scheduler.startLoop() to
 * create a dedicated task for it.
 * 
 * More information http://www.freertos.org/RTOS-software-timer.html
 */
void blink_timer_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;

  // Period of sequence is 10 times (1 second). 
  // RED LED will toggle first 2*n times (on/off) and remain off for the rest of period
  // Where n = number of connection
  static uint8_t count = 0;

  if ( count < 2*connection_num) digitalToggle(LED_RED);

  count++;
  if (count >= 10) count = 0;
}

/**
 * analogRead through a 16 channel muxer
 */
uint16_t readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[ ROWS ][ 4 ] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  uint16_t val = analogRead(SIG_pin);

  //return the value
  return val;
}

void loop()
{
  //read values
  duration = millis();
  for(int i = 0; i < COLUMNS; i++){
    //digitalWrite(columnPins[i], HIGH);
    for(int j = 0; j < ROWS; j++){
      values[ j ][ i ] = readMux(j);
    //  delay(1);
    }
    //digitalWrite(columnPins[i], LOW);
  }
  
  //compute mean and max values
  for(int i = 0; i < ROWS; i++){
    for(int j = 0; j < COLUMNS; j++){
      //rows[ i ] += values[ i ][ j ]; //sum all columns in one row
      if(maxi < values[ i ][ j ]){ // get maximum
        maxi = values[ i ][ j ];
      }
     }
     //rows[ i ] = rows[ i ] / COLUMNS //compute mean
     rowmax[ i ] = maxi; //maximum value in row
     maxi = 0;
     if(i < 7){ //maximum value between row 1 to 7 -> FRONT
      if(msg.data[ 0 ] < rowmax[ i ]){
        msg.data[ 0 ] = rowmax[ i ];
      }
     }
     if(i > 8){ //maximum value between row 8 to 16 -> REAR
      if(msg.data[ 1 ] < rowmax[ i ]){
        msg.data[ 1 ] = rowmax[ i ];
      }
     }
  }  
   duration = millis() - duration;
 
  //map values for pwm
  msg.data[ 0 ] = map(msg.data[ 0 ], 0, 400, 0, 255);
  msg.data[ 1 ] = map(msg.data[ 1 ], 0, 400, 0, 255);
  
  //debug print
  /*  for(int i = 0; i < 16; i++){
    Serial.print(values[i][0]);
    Serial.print("\t");
    Serial.println(rowmax[i]);
  }
  Serial.println("----------\n");
  for(int i = 0; i < 2; i++){
    Serial.print(msg.data[ i ]);
    Serial.print("\t");
  }
  Serial.println("\n----------\n");
  Serial.print("duration = ");
  Serial.print(duration);  
  Serial.println("ms\n");*/
  
  // First check if we are connected to any peripherals
  if ( Bluefruit.Central.connected() )
  {
    // Serial.println("Sending message");
    send_message(&msg);
    Serial.print(msg.data[ 0 ]);
    Serial.print("\t");
    Serial.println(msg.data[ 1 ]);
    delay(50);
  }
  
  for (uint16_t& val : msg.data) { //reset values to zero
    val = 0;
  }
}
