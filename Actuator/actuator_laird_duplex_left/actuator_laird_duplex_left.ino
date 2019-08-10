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


uint32_t dutycycle = 0;
uint32_t last = 0;

uint32_t inc_last = 0;

int pwm_values[ 2 ] = {0, 0};

bool ready = false;

enum constants {
  DUTY_MIN = 0,
  DUTY_MAX = 300,
};
const int OUTPUT_PINS[ 4 ] = { 28, // OUT1 *
                               29, // OUT2 *
                               30, // OUT3
                               31 // OUT4
                             };

/*
   This sketch demonstrate the central API(). A additional bluefruit
   that has bleuart as peripheral is required for the demo.
*/

#include "OneButton.h"

// Setup a new OneButton on pin 1.  
OneButton button(1, true);

#include <bluefruit.h>

// Peripheral uart service
BLEUart bleuart;
BLEClientDis  clientDis;
BLEClientUart clientUart;

#define MAX_BLE 2

#include "protocol.h"

struct message_t msgble;

void setup()
{
  // link the button 1 functions.
  button.attachClick(click1);
  button.attachDoubleClick(doubleclick1);
  button.attachLongPressStart(longPressStart1);
  button.attachLongPressStop(longPressStop1);
  button.attachDuringLongPress(longPress1);

  
  for (int i = 0; i < 4; i++) {
    pinMode(OUTPUT_PINS[ i ], OUTPUT);
    digitalWrite(OUTPUT_PINS[ i ], LOW);
  }
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);

  intro();

  Serial.begin(115200);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Left Duplex Bracelet");
  Serial.println("-----------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);

  Bluefruit.setName("Left Duplex");

  // Callbacks for Peripheral
  //  Bluefruit.setConnectCallback(prph_connect_callback);
  //  Bluefruit.setDisconnectCallback(prph_disconnect_callback);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);

  // Configure and Start BLE Uart Service
  //  bleuart.begin();
  //  bleuart.setRxCallback(prph_bleuart_rx_callback);

  // Init BLE Central Uart Serivce
  clientUart.begin();
  clientUart.setRxCallback(cent_bleuart_rx_callback);


  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Filter only accept bleuart service
     - Don't use active scan
     - Start(timeout) with timeout = 0 will scan forever (until connected)
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(bleuart.uuid);
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  // Set up and start advertising
  //  startAdv();

  msgble.length = MAX_BLE;
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
  Bluefruit.Advertising.start(30);                // 0 = Don't stop advertising after n seconds
}

/*------------------------------------------------------------------*/
/* Peripheral
  ------------------------------------------------------------------*/
void prph_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Prph] Connected to ");
  Serial.println(peer_name);
  ready = true;
}

void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("[Prph] Disconnected");
  ready = false;
}

void prph_bleuart_rx_callback(void)
{
  // Forward data from Mobile to our peripheral
  /*char str[20+1] = { 0 };
    bleuart.read(str, 20);

    Serial.print("[Prph] RX: ");
    Serial.println(str);

    if ( clientUart.discovered() )
    {
    clientUart.print(str);
    }else
    {
    bleuart.println("[Prph] Central role not connected");
    }*/
}

/*------------------------------------------------------------------*/
/* Central
  ------------------------------------------------------------------*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with bleuart service advertised
  // Connect to the device with bleuart service in advertising packet
  Bluefruit.Central.connect(report);
}

void cent_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Cent] Connected to ");
  Serial.println(peer_name);

  if ( (clientUart.discover(conn_handle)) && (peer_name[0] == 'R') )
  {
    // Enable TXD's notify
    clientUart.enableTXD();
  } else
  {
    // disconect since we couldn't find bleuart service
    Bluefruit.Central.disconnect(conn_handle);
  }
}

void cent_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("[Cent] Disconnected");
  intro();
}

/**
   Callback invoked when uart received data
   @param cent_uart Reference object to the service where the data
   arrived. In this example it is clientUart
*/
void cent_bleuart_rx_callback(BLEClientUart& cent_uart)
{
  /*char str[20 + 1] = { 0 };
    cent_uart.read(str, 20);

    Serial.print("[Cent] RX: ");
    Serial.println(str);

    if ( bleuart.notifyEnabled() )
    {
    // Forward data from our peripheral to Mobile
    bleuart.print( str );
    } else
    {
    // response with no prph message
    clientUart.println("[Cent] Peripheral role not connected");
    }*/
}

void loop()
{
   button.tick();
 
  //uint32_t now = millis();
  //duty_cycle(now);
  pwm_values[ 1 ] = 0;
  if ( Bluefruit.Central.connected() )
  {
    // Not discovered yet
    if ( clientUart.discovered() )
    {
      //      measure(now);
      int bytesread = receive_message(&msgble);
      for (int i = 0; i < 2; i++) pwm_values[ i ] = constrain(msgble.data[ i ], 0, 4095);
      //dutycycle = map(pwm_values[ 0 ], 0, 1000, DUTY_MIN, DUTY_MAX);
      analogWrite(OUTPUT_PINS[ 0 ], map(pwm_values[ 0 ], 0, 1000, 0, 240));
      analogWrite(OUTPUT_PINS[ 1 ], map(pwm_values[ 1 ], 0, 1000, 0, 240));
      Serial.print(msgble.data[ 0 ]); Serial.print("\t");
      Serial.print(msgble.data[ 1 ]); Serial.print("\t");
      Serial.println(dutycycle);
      if ( ready ) {
        bleuart.print(msgble.data[0]);
        bleuart.print(",");
        bleuart.println(msgble.data[1]);
      }
      // Discovered means in working state
      // Get Serial input and send to Peripheral
      /*if ( Serial.available() )
        {
        uint32_t inc_dur = now - inc_last;
        if (inc_dur >= 300) {
          inc_last = now;

          char str[20 + 1] = { 0 };
          Serial.readBytes(str, 20);

          clientUart.print( str );
        }
        }*/
    }
  }
}
void measure(uint32_t now) {
  uint32_t inc_dur = now - inc_last;
  if (inc_dur >= 900) {
    inc_last = now;
    // "measure"
    dutycycle = (dutycycle + 10) % (DUTY_MAX + 1);
  }
}

void duty_cycle(uint32_t now) {
  uint32_t duration = now - last;

  if (dutycycle && duration >= DUTY_MAX) {
    last = now;
    digitalWrite(OUTPUT_PINS[ 0 ], HIGH);
    digitalWrite(LED_RED, HIGH);
  } else if (duration >= dutycycle) {
    digitalWrite(OUTPUT_PINS[ 0 ], LOW);
    digitalWrite(LED_RED, LOW);
  }
}

void intro() {
  for (int i  = 0; i < 2; i++) {
    for (int i = 0; i < 4; i++) analogWrite(OUTPUT_PINS[ i ], 200);
    delay(200);
    for (int i = 0; i < 4; i++) analogWrite(OUTPUT_PINS[ i ], 0);
    delay(100);
  }
}
// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1() {
  Serial.println("click.");
} // click1


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("System Reset...");
  NVIC_SystemReset();
} // doubleclick1

// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStart1() {
//  Serial.println("Button longPress start");
} // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
//  Serial.println("Button longPress...");
} // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  Serial.println("Entering Over The Air Update...");
  enterOTADfu();
} // longPressStop1
