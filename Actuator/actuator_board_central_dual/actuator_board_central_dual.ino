#include <bluefruit.h>

// Peripheral uart service
BLEUart bleuart;

// Central uart client
BLEClientUart clientUart;

#define MAX_VALUES 165
#include "protocol.h" //serial communication protocol, includes receive_message()

static struct message_t msg;

const int outputPins[ 4 ] = { 2, 3, 4, 5 };
unsigned long previousMillis = 0;
const long interval = 200;
bool battery_warning = false;

void setup()
{
  pinMode(outputPins[ 0 ], OUTPUT);
  pinMode(A7, INPUT);
  digitalWrite(LED_RED, LOW);
  analogReadResolution(12);
  intro();
  Serial.begin(115200);
  // BLE
  // Initialize Bluefruit with max concurrent connections as Peripheral = 1, Central = 1
  // SRAM usage required by SoftDevice will increase with number of connections
  Bluefruit.begin(true, true);
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Left Haptic Actuator");

  // Callbacks for Peripheral
  Bluefruit.setConnectCallback(prph_connect_callback);
  Bluefruit.setDisconnectCallback(prph_disconnect_callback);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);

  // Configure and Start BLE Uart Service
  bleuart.begin();
  bleuart.setRxCallback(prph_bleuart_rx_callback);

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
  startAdv();

  Serial.println("Haptic Feedback Actuator");
  Serial.println("The device receive two pwm values via UART");
  Serial.println("Using serial message protocol defined in 'protocol.h'");
}

void intro() {
  for (int i = 0; i < 3; i++)
  {
    delay(200);
    for (int j = 0; j < 4; j++) {
      analogWrite(outputPins[ j ], 255);
    }
    delay(200);
    for (int j = 0; j < 4; j++) {
      analogWrite(outputPins[ j ], 0);
    }
  }
}

int read_battery(void) {
  int val = 0;
  for (int i = 0; i < 5; i++) {
    val += analogRead(A7);
  }
  return val / 5;
}

// BLE functions

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


/*------------------------------------------------------------------*/
/* Peripheral
  ------------------------------------------------------------------*/
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
  intro();
  Serial.println();
  Serial.println("[Prph] Disconnected");
}

void prph_bleuart_rx_callback(void)
{
  // Forward data from Mobile to our peripheral
  /*char str[20 + 1] = { 0 };
    bleuart.read(str, 20);

    Serial.print("[Prph] RX: ");
    Serial.println(str);

    if ( clientUart.discovered() )
    {
    clientUart.print(str);
    } else
    {
    bleuart.println("[Prph] Central role not connected");
    }*/
}

/*------------------------------------------------------------------*/
/* Central
  ------------------------------------------------------------------*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising contain BleUart service
  if ( Bluefruit.Scanner.checkReportForService(report, clientUart) )
  {
    Serial.println("BLE UART service detected. Connecting ... ");

    // Connect to device with bleuart service in advertising
    Bluefruit.Central.connect(report);
  }
}

void cent_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Cent] Connected to ");
  Serial.println(peer_name);;

  if ( clientUart.discover(conn_handle) )
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
  //set values to zero
  for (uint16_t& val : msg.data) {
    val = 0;
  }

  //if ( battery_warning == false ) {
    if ( Bluefruit.Central.connected() )
    {
      // Not discovered yet
      if ( clientUart.discovered() )
      {
        // receive all values
        uint16_t bytesread = receive_message(&msg);

        // get maximum values
        int maximum[ 2 ] = { 0, 0 };
        for (int i = 0; i < MAX_VALUES; i++) {
          if ( i <= 66 ) {
            if ( maximum[ 0 ] < msg.data[ i ] ) maximum[ 0 ] = msg.data[ i ];
          }
          else {
            if ( maximum[ 1 ] < msg.data[ i ] ) maximum[ 1 ] = msg.data[ i ];
          }
        }
        // output values
        int k = 0;
        for (int i = 0; i < 15; i++) {
          for (int j = 0; j < 11; j++) {
            Serial.print(msg.data[ k ]);
            Serial.print("\t");
            k++;
          }
          Serial.println();
        }
        maximum[ 0 ] = constrain( maximum[ 0 ], 0, 4095 );
        maximum[ 1 ] = constrain( maximum[ 1 ], 0, 4095 );
        Serial.println(maximum[ 0 ]);
        Serial.print("\t");
        Serial.println(maximum[ 1 ]);
        analogWrite( outputPins[ 0 ], map( maximum[ 0 ], 0, 4095, 0, 255 ) ); // rear vib
        analogWrite( outputPins[ 2 ], map( maximum[ 1 ], 0, 4095, 0, 255 ) ); // front vib
      }
    }
  //}
  /*else {
    analogWrite( outputPins[ 0 ], 0);
    analogWrite( outputPins[ 1 ], 0);
    digitalWrite(LED_RED, HIGH);
    waitForEvent();
    delay(400);
    digitalWrite(LED_RED, LOW);
    waitForEvent();
    delay(400);
  }*/
  unsigned long currentMillis = millis();

  if ( currentMillis - previousMillis >= interval ) {
    previousMillis = currentMillis;
    // print all values via bleuart on smartphone
    int k = 0;
    for (int i = 0; i < 15; i++) {
      for (int j = 0; j < 11; j++) {
        bleuart.print(String(msg.data[ k ] + "\t"));
        k++;
      }
      bleuart.println();
    }
    bleuart.println("-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-"); 
    if ( read_battery() < 2140 ) battery_warning = true;
  }


  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}
