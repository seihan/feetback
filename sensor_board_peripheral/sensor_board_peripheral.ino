#include <bluefruit.h>

// BLE Service
BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

#define MAX_VALUES 80
#define ROWS 16
#define COLUMNS 5

#include "protocol.h"

static struct message_t msg;
int duration = 0;

// array to store values
uint16_t values[ ROWS ][ COLUMNS ] = {}; //[rows][columns]

//mux control pins
int s0 = 15;
int s1 = 7;
int s2 = 11;
int s3 = 31;

//mux in "SIG" pin
int SIG_pin = A3;


uint16_t readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  uint16_t val = analogRead(SIG_pin);

  //return the value
  return val;
}

// Software Timer for blinking RED LED
SoftwareTimer blinkTimer;
void setupAdv(void)
{
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // There is no room for Name in Advertising packet
  // Use Scan response for Name
  Bluefruit.ScanResponse.addName();
}



void connect_callback(void)
{
  Serial.println("Connected");
}

void disconnect_callback(uint8_t reason)
{
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
  Serial.println("Bluefruit will auto start advertising (default)");
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

void setup()
{
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);
  pinMode(SIG_pin, INPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  
  //initialize array with zeros
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 5; j++){
      values[i][j] = 0;
      }
   }

  Serial.begin(115200);
  Serial.println("Bluefruit52 BLEUART Example");

  // Initialize blinkTimer for 1000 ms and start it
  blinkTimer.begin(1000, blink_timer_callback);
  //blinkTimer.start();

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  Bluefruit.begin();
  Bluefruit.setName("SensorSoleRight");
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up Advertising Packet
  setupAdv();

  // Start Advertising
  Bluefruit.Advertising.start();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");
}


void loop()
{
//read values
  duration = millis();
  for (int i = 0; i < COLUMNS; i++) {
    //digitalWrite(columnPins[i], HIGH);
    for (int j = 0; j < ROWS; j++) {
      values[ j ][ i ] = readMux(j);
      //  delay(1);
    }
    //digitalWrite(columnPins[i], LOW);
  }
  
  //print values and prepare the msg
  for (int h = 0; h < MAX_VALUES; h++){
    for (int i = 0; i < ROWS; i++) {
      for (int j = 0; j < COLUMNS; j++) {
        msg.data[ h ] = values[ i ][ j ];
        Serial.print(msg.data[ h ]);
        Serial.print("\t");
      }
    Serial.println();
    }
    Serial.println("--------------------------------\n");
  }
  duration = millis() - duration;

  // transmit message via bleuart
  send_message(&msg);

  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}
