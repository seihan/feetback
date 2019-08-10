const byte interruptPin = 0;
volatile byte state = HIGH;
const long countdown = 5000; //ms
unsigned long last = 0;

void setup() {
  //NRF_POWER ->DCDCEN = 0;
  /*NRF_PWM0  ->ENABLE = 0; //disable all pwm instance
    NRF_PWM1  ->ENABLE = 0;
    NRF_PWM2  ->ENABLE = 0;
    NRF_TWIM1 ->ENABLE = 0; //disable TWI Master
    NRF_TWIS1 ->ENABLE = 0; //disable TWI Slave
    NRF_SPI1 -> ENABLE = 0; //disable SPI
    NRF_SPI2 -> ENABLE = 0; //disable SPI
    NRF_NFCT->TASKS_DISABLE = 1; //disable NFC, confirm this is the right way*/
  Serial.begin(115200);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(interruptPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
}

void loop() {
  unsigned long now = millis();
  digitalWrite(LED_BLUE, state);
  if (now - last >= countdown) {
    last = now;
    Serial.println("Going to sleep...");
    // Request CPU to enter low-power mode until an event/interrupt occurs
    //waitForEvent();
    //NRF_POWER->SYSTEMOFF = 1;
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
  }
}

void blink() {
  Serial.println("Interrupt");
  state = !state;
  last = millis();
}
