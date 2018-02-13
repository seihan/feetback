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

const int outputPins[ 4 ] = { 2, 3, 4, 5 };

unsigned long previousMillis = 0;
int outState = LOW;

// Software Timer for blinking RED LED
SoftwareTimer blinkTimer;
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
  digitalToggle(LED_RED);
}

/**
   RTOS Idle callback is automatically invoked by FreeRTOS
   when there are no active threads. E.g when loop() calls delay() and
   there is no bluetooth or hw event. This is the ideal place to handle
   background data.

   NOTE: It is recommended to call waitForEvent() to put MCU into low-power mode
   at the end of this callback. You could also turn off other Peripherals such as
   Serial/PWM and turn them back on if wanted

   e.g

   void rtos_idle_callback(void)
   {
      Serial.stop(); // will lose data when sleeping
      waitForEvent();
      Serial.begin(115200);
   }

   NOTE2: If rtos_idle_callback() is not defined at all. Bluefruit will force
   waitForEvent() to save power. If you don't want MCU to sleep at all, define
   an rtos_idle_callback() with empty body !

   WARNING: This function MUST NOT call any blocking FreeRTOS API
   such as delay(), xSemaphoreTake() etc ... for more information
   http://www.freertos.org/a00016.html
*/
void rtos_idle_callback(void)
{
  // Don't call any other FreeRTOS blocking API()
  // Perform background task(s) here

  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}


void intro() {
  for (int i = 0; i < 3; i++)
  {
    delay(200);
    digitalWrite(outputPins[ 0 ], HIGH);
    analogWrite(outputPins[ 1 ], 255);
    analogWrite(outputPins[ 2 ], 255);
    analogWrite(outputPins[ 3 ], 255);
    delay(200);
    digitalWrite(outputPins[ 0 ], LOW);
    analogWrite(outputPins[ 1 ], 0);
    analogWrite(outputPins[ 2 ], 0);
    analogWrite(outputPins[ 3 ], 0);
  }
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

bool pwm_ready = true;
int pwm_period_ms = 50;
int pwm_duty_cycle_ms = 25;

SoftwareTimer pwmDutyCycleTimer;
void pwm_duty_cycle_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;
  pwmDutyCycleTimer.stop();

  // switch motor off
  digitalWrite(outputPins[ 0 ], LOW);
  digitalWrite(LED_RED, HIGH);

  // pwm can be reset
  pwm_ready = true;
}

SoftwareTimer pwmPeriodTimer;
void pwm_period_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;

  // switch motor on
  digitalWrite(outputPins[ 0 ], HIGH);
  digitalWrite(LED_RED, LOW);

  // start duty cycle timeout
  pwmDutyCycleTimer.setPeriod(pwm_duty_cycle_ms);
  pwmDutyCycleTimer.start();
}
void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
  pwmPeriodTimer.start();
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
  pwmPeriodTimer.stop();
  intro();
}

void setup()
{
  pinMode(outputPins[ 0 ], OUTPUT);
  intro();
  Serial.begin(115200);
  Serial.println("Haptic Feedback Actuator");

  for (uint16_t i = 0; i < 2; i++) {
    msg.data[i] = i + 1;
  }

  // Initialize blinkTimer for 1000 ms and start it
  //blinkTimer.begin(1000, blink_timer_callback);
  //blinkTimer.start();
  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);
  Serial.println("AutoConnLED TRUE");

  Bluefruit.begin();
  Serial.println("Bluefruit begin");
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
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

  // Set up and start advertising
  startAdv();

  // Start Advertising
  Bluefruit.Advertising.start();

  Serial.println("The device receive two pwm values via UART");
  Serial.println("Using serial message protocol defined in 'protocol.h'");

  pwmPeriodTimer.begin(pwm_period_ms, pwm_period_callback);
  pwmDutyCycleTimer.begin(pwm_duty_cycle_ms, pwm_duty_cycle_callback);

}

void loop()
{
  while (bleuart.available() ) {

    int bytesread = receive_message(&msg);

    //  Serial.print("Received value ");
    //  Serial.println(msg.data[0]);
    pwm_period_ms = map(msg.data[ 0 ], 0, 255, 4, 300);
    pwm_period_ms = constrain(pwm_period_ms, 4, 300);

  /*  if ((pwm_period_ms < 4) || (pwm_period_ms > 300) || (not pwm_ready)) {
      waitForEvent();
      return;
    }*/
    int old_duty_cycle = pwm_duty_cycle_ms;
    pwm_duty_cycle_ms = pwm_period_ms / 2;

    Serial.print("Setting period/duty cycle ");
    Serial.print(pwm_period_ms);
    Serial.print(" ms / ");
    Serial.print(pwm_duty_cycle_ms);
    Serial.println(" ms");

    if (old_duty_cycle == pwm_duty_cycle_ms) {
      //Serial.println("(not resetting period)");
    } else {
      pwmPeriodTimer.setPeriod(pwm_period_ms);
      pwm_ready = false;
    }
  }
  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}
