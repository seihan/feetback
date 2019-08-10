#include <SPI.h>
const int COLUMNS = 21;

const int MuxDC_PIN = 15;
const int MuxADC_PIN = 16;
// const int ADC_Mux = A0;
const int LED2_PIN = 19;
const int ADC_PINS[ ] = {A0, A1, A2, A3, A6};
const int DC_PINS[ ] = {7, 11, 18, 22, 27, 28, 29};
byte MuxDC_channel = 0;
byte MuxADC_channel = 17;

struct line {
  int adc_pin;
  int dc_pin [COLUMNS];
};

struct line sole[] = {
  {  0, { -1, -1, -1, -1, -1, -1, -1,  4,  5,  6,  7,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1}},
  {  1, { -1, -1, -1, -1, -1, -1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, -1, -1, -1, -1, -1}},
  {  2, { -1, -1, -1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1}},
  {  3, { -1, -1, -1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1}},
  {  4, { -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1}},
  {  5, { -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  {  6, { -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  {  7, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  {  8, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  {  9, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 10, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 11, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 12, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 13, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 14, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 15, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 16, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 17, { -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 19, { -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 20, { -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 21, { -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 22, { -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 24, { -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 25, { -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 26, { -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 27, { -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 29, { -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 30, { -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 31, { 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 33, { 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 34, { 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  { 35, { 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}},
  {  8, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1}},
  {  9, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 10, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 11, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 12, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 13, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 14, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 15, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 16, { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 17, { -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 18, { -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 19, { -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 20, { -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 21, { -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16}},
  { 22, { -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 23, { -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 24, { -1, -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 25, { -1, -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 26, { -1, -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1}},
  { 27, { -1, -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 28, { -1, -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 29, { -1, -1, -1, -1, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 30, { -1, -1, -1, -1, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1}},
  { 31, { -1, -1, -1, -1, -1, -1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1}},
  { 32, { -1, -1, -1, -1, -1, -1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1}},
  { 33, { -1, -1, -1, -1, -1, -1, -1, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, -1, -1, -1, -1}},
  { 33, { -1, -1, -1, -1, -1, -1, -1, -1, 28, 27, 26, 25, 24, 23, 22, 21, 20, -1, -1, -1, -1}},
  { 33, { -1, -1, -1, -1, -1, -1, -1, -1, -1, 27, 26, 25, 24, 23, 22, 21, -1, -1, -1, -1, -1}}
};

SPISettings settings(20000000, MSBFIRST, SPI_MODE0);

SoftwareTimer switchTimer;

/*
    AR_INTERNAL (0.6V Ref * 6 = 0..3.6V) <-- DEFAULT
    AR_INTERNAL_3_0 (0.6V Ref * 5 = 0..3.0V)
    AR_INTERNAL_2_4 (0.6V Ref * 4 = 0..2.4V)
    AR_INTERNAL_1_8 (0.6V Ref * 3 = 0..1.8V)
    AR_INTERNAL_1_2 (0.6V Ref * 2 = 0..1.6V)
    AR_VDD4 (VDD/4 REF * 4 = 0..VDD)
*/

const int ROWS = sizeof(sole) / sizeof(*sole);
uint16_t values[COLUMNS * ROWS] = {};

int select_adc(int adc_idx) {
  if (adc_idx < 32) {
    // SPI MUX pin w/ A0
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( adc_idx );
    digitalWrite(MuxADC_PIN, HIGH);
    return ADC_PINS[0];
  } else {
    return ADC_PINS[adc_idx-31];
  }
}

uint16_t read_pin(int adc_pin, int dc_idx) {
  int dc_pin = 0;
  if (dc_idx < 32) {
    // SPI MUX pin
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( dc_idx );
    digitalWrite(MuxDC_PIN, HIGH);
    return analogRead(adc_pin);
  } else {
    // GPIO pin
    dc_pin = DC_PINS[dc_idx-32];
    digitalWrite(dc_pin, HIGH);
    uint16_t res = analogRead(A0);
    digitalWrite(dc_pin, LOW);    
    return res;
  }
};

int read_sole()
{
  int k = 0;
  for (int row = 0; row < ROWS; row++) {
    int adc_pin = select_adc(sole[row].adc_pin);
    for (int col = 0; col < COLUMNS; col++)
      if (sole[row].dc_pin[col] != -1)
        values[k++] = read_pin(adc_pin, sole[row].dc_pin[col]);
  }

  return k;
}


void switch_timer_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;
  MuxADC_channel++;
  if (MuxADC_channel > 31) MuxADC_channel = 0;
  Serial.println("\nZeile " + String(MuxADC_channel + 1));
  digitalWrite(MuxADC_PIN, LOW);
  SPI.beginTransaction(settings);
  SPI.transfer(MuxADC_channel);
  SPI.endTransaction();
  digitalWrite(MuxADC_PIN, HIGH);
}

/*
void readFSR( int result [ 429 ] ) {
  int k = 0;
  SPI.beginTransaction(settings);

  // Block 1
  digitalWrite(MuxADC_PIN, LOW);
  SPI.transfer( 0 );
  digitalWrite(MuxADC_PIN, HIGH);
  for (byte j = 4; j < 11; j++) {
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( j );
    digitalWrite(MuxDC_PIN, HIGH);
    result [ k ] = analogRead(A0);
    k++;
  }
  // Block 2
  digitalWrite(MuxADC_PIN, LOW);
  SPI.transfer( 1 );
  digitalWrite(MuxADC_PIN, HIGH);
  for (byte j = 3; j < 13; j++) {
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( j );
    digitalWrite(MuxDC_PIN, HIGH);
    result [ k ] = analogRead(A0);
    k++;
  }
  // Block 3
  for (byte i = 2; i < 4; i++) {
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( i );
    digitalWrite(MuxADC_PIN, HIGH);
    for (byte j = 2; j < 14; j++) {
      digitalWrite(MuxDC_PIN, LOW);
      SPI.transfer( j );
      digitalWrite(MuxDC_PIN, HIGH);
      result [ k ] = analogRead(A0);
      k++;
    }
  }
  // Block 4
  digitalWrite(MuxADC_PIN, LOW);
  SPI.transfer( 4 );
  digitalWrite(MuxADC_PIN, HIGH);
  for (byte j = 1; j < 14; j++) {
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( j ); //MuxDC_channel );
    digitalWrite(MuxDC_PIN, HIGH);
    result [ k ] = analogRead(A0);
    k++;
  }
  // Block 5
  for (byte i = 5; i < 7; i++) {
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( i );
    digitalWrite(MuxADC_PIN, HIGH);
    for (byte j = 1; j < 15; j++) {
      digitalWrite(MuxDC_PIN, LOW);
      SPI.transfer( j );
      digitalWrite(MuxDC_PIN, HIGH);
      result [ k ] = analogRead(A0);
      k++;
    }
  }
  // Block 6
  for (byte i = 7; i < 17; i++) {
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( i );
    digitalWrite(MuxADC_PIN, HIGH);
    for (byte j = 0; j < 15; j++) {
      digitalWrite(MuxDC_PIN, LOW);
      SPI.transfer( j );
      digitalWrite(MuxDC_PIN, HIGH);
      result [ k ] = analogRead(A0);
      k++;
    }
  }
  // Block 7
  digitalWrite(MuxADC_PIN, LOW);
  SPI.transfer( 17 );
  digitalWrite(MuxADC_PIN, HIGH);
  digitalWrite(DC_PIN[ 4 ], HIGH);
  result [ k ] = analogRead(A0);
  k++;
  digitalWrite(DC_PIN[ 4 ], LOW);
  for (byte j = 0; j < 15; j++) {
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( j );
    digitalWrite(MuxDC_PIN, HIGH);
    result [ k ] = analogRead(A0);
    k++;
  }
  for (byte i = 19; i < 25; i++) {
    if ( i == 23 ) i = 24;
    digitalWrite(MuxADC_PIN, LOW);
    SPI.transfer( i );
    digitalWrite(MuxADC_PIN, HIGH);
    digitalWrite(DC_PIN[ 4 ], HIGH);
    result [ k ] = analogRead(A0);
    k++;
    digitalWrite(DC_PIN[ 4 ], LOW);
    for (byte j = 0; j < 15; j++) {
      digitalWrite(MuxDC_PIN, LOW);
      SPI.transfer( j );
      digitalWrite(MuxDC_PIN, HIGH);
      result [ k ] = analogRead(A0);
      k++;
    }
  }
  // Block 8
  digitalWrite(MuxADC_PIN, LOW);
  SPI.transfer( 31 );
  digitalWrite(MuxADC_PIN, HIGH);
  for (byte h = 4; h < 6; h++) {
    digitalWrite(DC_PIN[ h ], HIGH);
    result [ k ] = analogRead(A0);
    digitalWrite(DC_PIN[ h ], LOW);
    k++;
    for (byte j = 0; j < 15; j++) {
      digitalWrite(MuxDC_PIN, LOW);
      SPI.transfer( j );
      digitalWrite(MuxDC_PIN, HIGH);
      result [ k ] = analogRead(A0);
      k++;
    }
  }
  // Block 9
  digitalWrite(MuxADC_PIN, LOW);
  SPI.transfer( 31 );
  digitalWrite(MuxADC_PIN, HIGH);
  for (byte h = 4; h < 6; h++) {
    digitalWrite(DC_PIN[ h ], HIGH);
    result [ k ] = analogRead(A0);
    digitalWrite(DC_PIN[ h ], LOW);
    k++;
  }
  for (byte j = 0; j < 15; j++) {
    digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer( j );
    digitalWrite(MuxDC_PIN, HIGH);
    result [ k ] = analogRead(A0);
    k++;
  }
  for (int i = 1; i < 4; i++) {
    for (byte h = 4; h < 6; h++) {
      digitalWrite(DC_PIN[ h ], HIGH);
      result [ k ] = analogRead(ADC_PIN[ i ]);
      digitalWrite(DC_PIN[ h ], LOW);
      k++;
    }
    for (byte j = 0; j < 15; j++) {
      digitalWrite(MuxDC_PIN, LOW);
      SPI.transfer( j );
      digitalWrite(MuxDC_PIN, HIGH);
      result [ k ] = analogRead(ADC_PIN[ i ]);
      k++;
    }
  }
  SPI.endTransaction();
}
*/

void setup() {
  switchTimer.begin(4000, switch_timer_callback);
  //switchTimer.start();
  Serial.begin(115200);
  analogReference(AR_INTERNAL_1_2);
  analogReadResolution(14);
  pinMode(MuxDC_PIN, OUTPUT);
  pinMode(MuxADC_PIN, OUTPUT);
  digitalWrite(LED2_PIN, HIGH); // on if right plugged
  SPI.begin();

  
  digitalWrite(MuxADC_PIN, LOW);
  SPI.beginTransaction(settings);
  //  SPI.transfer(MuxADC_channel);
  digitalWrite(MuxADC_PIN, HIGH);
  
  /*digitalWrite(MuxDC_PIN, LOW);
    SPI.transfer(MuxDC_channel);
    digitalWrite(MuxDC_PIN, HIGH);*/
  //SPI.endTransaction();
}

void loop() {
  /*int input = Serial.read();
    if (input > 0) {
    Serial.println("Got something");
    ADCMuxChSelect(input);
    delay(1);
    }
    else {
    digitalWrite(MuxDC_PIN, LOW);
    //SPI.beginTransaction(settings);
    SPI.transfer( 29 ); //MuxDC_channel );
    //SPI.endTransaction();
    digitalWrite(MuxDC_PIN, HIGH);
    Serial.print(analogRead(A0));
    for (byte i = 0; i < 15; i++) {
      digitalWrite(MuxDC_PIN, LOW);
      //SPI.beginTransaction(settings);
      SPI.transfer( i ); //MuxDC_channel );
      //SPI.endTransaction();
      digitalWrite(MuxDC_PIN, HIGH);

      delay(10);

      Serial.print(analogRead(A0));
      Serial.print(".\t.");
    }
    Serial.println();
    }*/
  int elems = read_sole();
  Serial.println();
  Serial.print("--- new values --- ");
  Serial.println(elems);
  for (int i = 0; i < elems; i++) {
    Serial.print(String(values[ i ]) + "-\t-");
  }
}
/* ADC                                                      DC                                                       Block
    0 :                                    - 04 - 05 - 06 - 07 - 08 - 09 - 10 -                                         1
    1 :                               - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 -                               2
    2 :                          - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 -                          3
    3 :                          - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 -                          3
    4 :                     - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 -                          4
    5 :                     - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     5
    6 :                     - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     5
    7 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
    8 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
    9 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   10 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   11 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   12 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   13 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   14 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   15 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
   16 :                - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     6
  ! 17 :           - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     7
   19 :           - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     7
   20 :           - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     7
   21 :           - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     7
  ! 22 :           - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     7
   24 :           - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     7
   25 :      - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     8
   26 :      - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     8
  ! 27 :      - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     8
   29 :      - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     8
   30 :      - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     8
   31 : - 39 - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     9
  ! A2 : - 39 - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     9
   A3 : - 39 - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     9
   A6 : - 39 - 38 - 37 - 00 - 01 - 02 - 03 - 04 - 05 - 06 - 07 - 08 - 09 - 10 - 11 - 12 - 13 - 14 -                     9
      -------------------------------------------------------------------------------------------------------428----------
    8 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 -                     10
    9 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                11
   10 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                11
   11 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                11
   12 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           12
   13 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           12
   14 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           12
   15 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      13
   16 : - 36 - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      13
   17 :      - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      14
   18 :      - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      14
   19 :      - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      14
   20 :      - 35 - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      14
   21 :           - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 - 16 -      15
   22 :           - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           16
   23 :           - 34 - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           16
   24 :                - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           17
   25 :                - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           17
   26 :                - 33 - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 - 17 -           17
   27 :                     - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                18
   28 :                     - 32 - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                19
   29 :                          - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                20
   30 :                          - 31 - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 - 18 -                20
   31 :                               - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 -                     21
   A1 :                               - 30 - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 - 19 -                     21
   A2 :                                    - 29 - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 -                          22
   A3 :                                         - 28 - 27 - 26 - 25 - 24 - 23 - 22 - 21 - 20 -                          22
   A6 :                                              - 27 - 26 - 25 - 24 - 23 - 22 - 21 -                               23
*/

