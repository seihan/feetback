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

// BLE Service
BLEUart bleuart;

#define MAX_VALUES 80
#define ROWS 16
#define COLUMNS 5

#include "protocol.h"

static struct message_sl msg;
int duration = 0;

// input pins
const int inputPins[ COLUMNS ] = {
  A0, A1, A2, A3, A4
};

// column pins to set column high
const int columnPins[ COLUMNS ] = {
  27, 30, 16, 6, 20
};

//mux control pins
int s0 = 15;
int s1 = 7;
int s2 = 11;
int s3 = 31;

// Software Timer for blinking RED LED
SoftwareTimer blinkTimer;

void setup()
{
  // ADC config
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  // message length
  msg.length = MAX_VALUES;

  Serial.begin(115200);
  Serial.println("LeftSensorSole");
  Serial.println("---------------------------\n");

  // Initialize blinkTimer for 1000 ms and start it
  blinkTimer.begin(1000, blink_timer_callback);
  blinkTimer.start();

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("LefSensorSole");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Configure and Start BLE Uart Service
  bleuart.begin();

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
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
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
  digitalToggle(LED_RED);
}

/**
 * RTOS Idle callback is automatically invoked by FreeRTOS
 * when there are no active threads. E.g when loop() calls delay() and
 * there is no bluetooth or hw event. This is the ideal place to handle
 * background data.
 * 
 * NOTE: It is recommended to call waitForEvent() to put MCU into low-power mode
 * at the end of this callback. You could also turn off other Peripherals such as
 * Serial/PWM and turn them back on if wanted
 * 
 * e.g
 * 
 * void rtos_idle_callback(void)
 * {
 *    Serial.stop(); // will lose data when sleeping
 *    waitForEvent();
 *    Serial.begin(115200); 
 * }
 * 
 * NOTE2: If rtos_idle_callback() is not defined at all. Bluefruit will force
 * waitForEvent() to save power. If you don't want MCU to sleep at all, define
 * an rtos_idle_callback() with empty body !
 * 
 * WARNING: This function MUST NOT call any blocking FreeRTOS API 
 * such as delay(), xSemaphoreTake() etc ... for more information
 * http://www.freertos.org/a00016.html
 */
void rtos_idle_callback(void)
{
  // Don't call any other FreeRTOS blocking API()
  // Perform background task(s) here

  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}

/**
 * digitalWrite through a 16 channel muxer
 */
void writeMux(int channel) {
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
}

void sendAll(unsigned char* str, uint16_t len){
  bleuart.write(str, len);  
}

void loop(){
  duration = millis();
  int k = 0;
  for(int i = 0; i < ROWS; i++){
    writeMux( i ); //set column HIGH
    for(int j = 0; j < COLUMNS; j++){
      msg.data[ k ] = k; // analogRead( inputPins[ j ] ); //read values
      delay(1);
      //print values
      //Serial.print(msg.data[ k ]);
      //Serial.print(" ");
      k++;
    }
    //Serial.println();
  }
  //Serial.println("-------------------------------------------------");

  send_to_central(&msg);//transmit values
  
  for (uint16_t& val : msg.data) { //reset values to zero
    val = 0;
  }
}
