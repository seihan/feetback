uint32_t vdd_pin = 32;                // new imaginary pin

#define VDD_MV_PER_LSB (0.87890625F)  // 3.6V ADC range and 12-bit ADC resolution = 3600mV/4096

float readVDD(void) {
  float raw;

  // Set the analog reference to 3.6V (default = 3.6V)
  analogReference(AR_INTERNAL);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vdd_pin);

  return raw * VDD_MV_PER_LSB;
}


void setup() {
  Serial.begin(115200);
  readVDD();
}

void loop() {
  Serial.println( readVDD()) ;
  delay(1000);
}
