uint32_t dutycycle = 0;
uint32_t last = 0;

enum constants {
  DUTY_MIN = 0,
  DUTY_MAX = 300,
};
const int OUTPUT_PINS[ 4 ] = { 28, // OUT1
                               29, // OUT2
                               30, // OUT3
                               31 // OUT4
                             };

void setup() {
  /*if ((NRF_UICR->NFCPINS & UICR_NFCPINS_PROTECT_Msk) == (UICR_NFCPINS_PROTECT_NFC << UICR_NFCPINS_PROTECT_Pos)) {
    Serial.println("Fix NFC pins");
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_UICR->NFCPINS &= ~UICR_NFCPINS_PROTECT_Msk;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    Serial.println("Done");
    delay(500);
    NVIC_SystemReset();
  }*/
  for (int i = 0; i < 4; i++) {
    pinMode(OUTPUT_PINS[ i ], OUTPUT);
    digitalWrite(OUTPUT_PINS[ i ], LOW);
  }
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
}

uint32_t inc_last = 0;

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
    analogWrite(OUTPUT_PINS[ 2 ], map(dutycycle, DUTY_MIN, DUTY_MAX, 70, 200));
  } else if (duration >= dutycycle) {
    digitalWrite(OUTPUT_PINS[ 0 ], LOW);
  }
}

void loop() {
  uint32_t now = millis();

  duty_cycle(now);
  measure(now);
}
