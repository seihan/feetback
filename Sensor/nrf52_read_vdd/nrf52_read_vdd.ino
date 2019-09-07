int vdd_pin = 32;                // new imaginary pin
int vbat_pin = A6;               // voltage divider
#define V_MV_PER_LSB (0.87890625F)  // 3.6V ADC range and 12-bit ADC resolution = 3600mV/4096
#define VBAT_DIVIDER      (0.705882353F)   // 1.8M + 0.750M voltage divider on VBAT = (1.8M / (0.750M + 1.8M))
#define VBAT_DIVIDER_COMP (1.416666667F)   // Compensation factor for the VBAT divider ((0.750M + 1.8M) / 1.8M)

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * V_MV_PER_LSB)


float readVDD(void) {
  float raw;
  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vdd_pin);

  return raw * V_MV_PER_LSB;
}

float readVBAT(void) {
  float raw;
  raw = analogRead(vbat_pin);

  return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
  if(mvolts<3300)
    return 0;

  if(mvolts <3600) {
    mvolts -= 3300;
    return mvolts/30;
  }

  mvolts -= 3600;
  return 10 + (mvolts * 0.15F );  // thats mvolts /6.66666666
}

void setup() {
  Serial.begin(115200);
  // Set the analog reference to 3.6V (default = 3.6V)
  analogReference(AR_INTERNAL);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  readVDD();
}

void loop() {
  Serial.println( "Internal voltage = " + String(readVDD()) + "mV" );
  float vbat_mv = readVBAT();
  Serial.println( "Battery voltage = " + String(vbat_mv) + "mV" );
  Serial.println( "Battery level = " + String(mvToPercent(vbat_mv)) + "%" );
  delay(1000);
}
