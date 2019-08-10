#pragma once
byte alloff = B10000000;

int select_adc(int adc_idx) {
  if (adc_idx < 32) {
    // SPI MUX pin w/ A0
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( adc_idx );
    delayMicroseconds(AXTIME);
    digitalWrite(MuxADC_PIN, HIGH);
    return ADC_PINS[0];
  } else {
    return ADC_PINS[adc_idx - 31];
  }
}

uint16_t read_pin(int adc_pin, int dc_idx) {
  int dc_pin = 0;
  uint16_t res = 0;
  if (dc_idx < 32) {
    // SPI MUX pin
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( dc_idx );
    digitalWrite(MuxDC_PIN, HIGH);
    delayMicroseconds(DXTIME);
    for (int i = 0; i < ADCOUNTS; i++) res = analogRead(adc_pin);
    return res;
    delayMicroseconds(ADCTIME);
    //uint8_t res = analogRead(adc_pin);
    //return res;
  } else {
    // GPIO pin
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( alloff ); // workaround to prevent multiple highlines
    digitalWrite(MuxDC_PIN, HIGH);
    delayMicroseconds(DXTIME);
    dc_pin = DC_PINS[dc_idx - 32];
    digitalWrite(dc_pin, HIGH);
    for (int i = 0; i < ADCOUNTS; i++) res = analogRead(adc_pin);
    return res;
    delayMicroseconds(ADCTIME);
    //uint8_t res = analogRead(adc_pin);
    //delayMicroseconds( 13 ); // 2.5pF ADC
    digitalWrite(dc_pin, LOW);
    delayMicroseconds(DXTIME);
    //return res;
  }
}

int read_sole(uint16_t * values)
{
  int k = 0;
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLUMNS; col++) {
      if (sole[row].dc_pin[col] != -1) {
        int adc_pin = select_adc(sole[row].adc_pin);
        values[k++] = read_pin(adc_pin, sole[row].dc_pin[col]);
      }
    }
  }
  return k;
}
