#pragma once
byte alloff = B10000000;

int select_adc(int adc_idx) {
  if (adc_idx < 32) {
    // SPI MUX pin w/ A0
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( adc_idx );
    // delayMicroseconds( 1 );
    digitalWrite(MuxADC_PIN, HIGH);
    return ADC_PINS[0];
  } else {
    return ADC_PINS[adc_idx - 31];
  }
}

uint8_t map_n_cast( uint16_t value ) {
  uint16_t tmp = 0;
  tmp = constrain(value, 100, 4000);
  tmp = map(tmp, 100, 4000, 0, 255);
  return (uint8_t)tmp;
}

uint16_t read_pin(int adc_pin, int dc_idx) {
  int dc_pin = 0;
  if (dc_idx < 32) {
    // SPI MUX pin
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( dc_idx );
    delayMicroseconds( 3 );
    digitalWrite(MuxDC_PIN, HIGH);
    uint16_t res = analogRead(adc_pin);
    //delayMicroseconds( 13 );
    return res;
  } else {
    // GPIO pin
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( alloff );
    delayMicroseconds( 3 );
    digitalWrite(MuxDC_PIN, HIGH);
    dc_pin = DC_PINS[dc_idx - 32];
    digitalWrite(dc_pin, HIGH);
    uint16_t res = analogRead(adc_pin);
    //delayMicroseconds( 13 );
    digitalWrite(dc_pin, LOW);
    return res;
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
