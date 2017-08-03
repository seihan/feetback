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

#define MAX_VALUES 2

// BLE Service
BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

#include "protocol.h" //serial communication protocol, includes receive_message()

static struct message_t msg;

const int outputPins[2] = { 15, 16 }; //FRONT, REAR motor

// Software Timer for blinking RED LED
SoftwareTimer blinkTimer;
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

void intro(){
  for (int i = 0; i < 3; i++)
  {
    delay(250);
    analogWrite(outputPins[ 0 ], 200);
    analogWrite(outputPins[ 1 ], 200);
    delay(250);
    analogWrite(outputPins[ 0 ], 0);
    analogWrite(outputPins[ 1 ], 0);
  }
}

void setup()
{
  intro();
  Serial.begin(115200);
  Serial.println("Haptic Feedback Actuator");
 
 for(uint16_t i = 0; i < 2; i++){
    msg.data[i] = i + 1;
   }

  // Initialize blinkTimer for 1000 ms and start it
  blinkTimer.begin(1000, blink_timer_callback);
  //blinkTimer.start();

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  Bluefruit.begin();
  Bluefruit.setName("Left2VibActuator");
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

  Serial.println("The device receive two pwm values via UART");
  Serial.println("Using serial message protocol defined in 'protocol.h'");
}

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

void loop()
{
  // Forward data from HW Serial to BLEUART
  /*while (Serial.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);

    uint8_t buf[64];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write( buf, count );
  }*/

  // Forward from BLEUART to HW Serial
  while ( bleuart.available() ){
    
    //receive data via bleuart
    int bytesread = receive_message(&msg);
    
    for(int i = 0; i < 2; i++){
      
      //workaround to get the origin values
      msg.data[ i ] = msg.data[ i ] / 256;
    
      //pwm motor control  
      analogWrite(outputPins[ i ], min(msg.data[ i ], 255)); //upper limit 255
    
      Serial.print(msg.data[ i ]); 
      Serial.print("\t"); 
    }
    Serial.println();
    // Request CPU to enter low-power mode until an event/interrupt occurs
    waitForEvent();
  }
}

void connect_callback(void)
{
  Serial.println("Connected");
}

void disconnect_callback(uint8_t reason)
{
  (void) reason;
  intro();
  Serial.println();
  Serial.println("Disconnected");
  Serial.println("Bluefruit will auto start advertising (default)"); 
}
