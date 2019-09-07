#include <SPI.h>

//SPISettings settings(20000000, MSBFIRST, SPI_MODE0);

#define HWFC  true
#define MAX_VALUES 8
#define MEASURED_VALUES 956

#include "protocol.h"
#include "toplist.h"
#include "sole_fsr956_bl652.h"
#include "sole.h"

struct message_t msg;
uint16_t data[MEASURED_VALUES];
const int LED2_PIN = 19;

struct smaller_measurement {
  bool operator()(measure_t& e, measure_t& other) {
    return e.value < other.value;
  }
};

toplist<MAX_VALUES, measure_t, smaller_measurement> top;

void setup()
{
  Serial.begin(230400);//Serial.begin(115200);
  Serial.print("Message size (bytes): ");
  Serial.println(sizeof(msg));
  digitalWrite(LED2_PIN, HIGH);
  analogReference(AR_INTERNAL_1_2);
  analogReadResolution(14);
  pinMode(MuxDC_PIN, OUTPUT);
  pinMode(MuxADC_PIN, OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(DC_PINS[ i ], OUTPUT);
  for (int i = 0; i < 5; i++) pinMode(ADC_PINS[ i ], INPUT);
  SPI.begin();
}

void loop()
{
  uint16_t nval = read_sole(data); // read values

  top.clear();
  for (uint16_t i = 0; i != nval; i++) {
    top.add(measure_t{i, data[i]});
  }
  msg.length = MAX_VALUES;
  nval = 0;
  for (const measure_t& val : top) {
     msg.data[nval++] = val;
  }
  send_to_serial(&msg); // transmit values
}
