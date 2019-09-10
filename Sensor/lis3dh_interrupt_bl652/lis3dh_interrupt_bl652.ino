// Basic demo for tap/doubletap readings from Adafruit LIS3DH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

// software SPI
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);
// hardware SPI
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();


// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 80

// Interrupt
const byte interruptPin = 0;
//volatile byte state = HIGH;
const long countdown = 5000; //ms
unsigned long last = 0;
bool state = true;

void setup(void) {
  pinMode(LED_BLUE, OUTPUT);
  pinMode(interruptPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, FALLING);
  Serial.begin(115200);
  Serial.println("Adafruit LIS3DH Tap Test!");

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!

  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  lis.setClick(1, CLICKTHRESHHOLD);
  delay(100);
  digitalWrite(LED_BLUE, HIGH);
}

void loop() {
  //  uint8_t click = lis.getClick();
  //  if (click == 0) return;
  //  if (! (click & 0x30)) return;
  //  Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
  //  if (click & 0x10) Serial.print(" single click");
  //  if (click & 0x20) Serial.print(" double click");
  //  Serial.println();

  //  delay(100);
  //  return;
  unsigned long now = millis();
  if (now - last >= countdown) {
    last = now;
    state = true;
    Serial.println("Going to sleep...");
    Serial.flush();
    digitalWrite(LED_BLUE, LOW);
    // Request CPU to enter low-power mode until an event/interrupt occurs
    waitForEvent();
    //NRF_POWER->SYSTEMOFF = 1;
    //sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    //    __WFI();
    suspendLoop();
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    sd_app_evt_wait();
    /*  while (state) {
        sd_power_system_off();
      }*/
  }
}


void blink() {
  digitalWrite(LED_BLUE, HIGH);
  Serial.println("Interrupt");
  Serial.flush();
  state = false;
  last = millis();
  resumeLoop();
}
