/*

# -Sensor Board-
# board: Adafruit Bluefruit nRF52 Feather central / peripheral bluetooth low energy (ble)

# central role
# reading, processing, storing and transmitting of 64 sensor values

# 8x8 sensor matrix input

# (reading time from realtime clockwork)

# store 64 values on sdcard

# downmix to 2 median values front / back

# transmit front-, back-values via ble UART service as central device


# peripheral role
# reading and transmitting of 64 sensor values

# 8x8 sensor matrix input

# transmit 64 sensor values via ble UART or custom service

*/


#include <Arduino.h>
#include "protocol.h"

static struct message_t msg;

void setup() {
  Serial.begin(9600);

  // 'Clear' the message buffer
  msg.length = MAX_VALUES;
  for (uint16_t& val : msg.data) {
    val = 0xdead;
  }
}

void loop() {
  // read sensor values into message buffer
  // TODO: remove this fake delay
  delay(10);

  // compute buffer size to transmit
  // TODO: this check will be moved to some common code path later
  if (msg.length > MAX_VALUES) {
    msg.length = MAX_VALUES;
  }
  unsigned length = (msg.length * 2) + 3;

  // write message to serial out
  Serial.write(reinterpret_cast<const char *>(&msg), length);
}

