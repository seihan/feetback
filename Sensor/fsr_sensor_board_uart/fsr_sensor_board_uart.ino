#include <SPI.h>

//SPISettings settings(20000000, MSBFIRST, SPI_MODE0);

#define HWFC  true
#define MAX_VALUES 956
#include "protocol.h"

#include "sole_fsr956.h"
#include "sole.h"

struct message_t msg;
const int LED2_PIN = 19;

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
  msg.length = read_sole(msg.data); // read values
  send_to_serial(&msg); // transmit values
}
