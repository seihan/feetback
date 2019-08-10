/*
   Common message format for value streams.
*/

#pragma once

#ifndef MAX_VALUES
#define MAX_VALUES 2
#endif
#ifndef MAX_VALUES_SL
#define MAX_VALUES_SL 64
#endif

static const unsigned char PROTOCOL_HDR1 = 'M';
static const unsigned char PROTOCOL_HDR2 = 'V';

struct message_t {
  unsigned char reserved; // for alignment
  unsigned char protocol_header[2] {PROTOCOL_HDR1, PROTOCOL_HDR2}; // data start marker
  unsigned char length; // #values in the message
  uint16_t data[MAX_VALUES];
};

struct message_sl {
  unsigned char reserved; // for alignment
  unsigned char protocol_header[2] {PROTOCOL_HDR1, PROTOCOL_HDR2}; // data start marker
  unsigned char length; // #values in the message
  uint16_t data[MAX_VALUES_SL];
};

/*void sendtoPeripherals(unsigned char* str, uint16_t len);

  // Send a message out through the UART.

  void send_to_prphs(message_t *msg) {
  if (msg->length > MAX_VALUES) {
    msg->length = MAX_VALUES;
  }

  unsigned len = (msg->length * sizeof(uint16_t)) + 3;
  sendtoPeripherals(reinterpret_cast<unsigned char *>(msg->protocol_header), len);
  }*/

void send_to_central(message_t *msg) {
  if (msg->length > MAX_VALUES) {
    msg->length = MAX_VALUES;
  }

  unsigned len = (msg->length * sizeof(uint16_t)) + 3;
  BLESerial.write(reinterpret_cast<unsigned char *>(msg->protocol_header), len);
}

void send_to_serial(message_sl *msgsl) {
  if (msgsl->length > MAX_VALUES_SL) {
    msgsl->length = MAX_VALUES_SL;
  }

  unsigned len = (msgsl->length * sizeof(uint16_t)) + 3;
  Serial.write(reinterpret_cast<unsigned char *>(msgsl->protocol_header), len);
}
/* Read a message into a buffer
   @return number of data bytes read, 0 on error.
*/
/*int receive_message(message_t *msg) {
  int ok = 0;

  // find message start
  do {
    while (not ok || msg->protocol_header[1] != PROTOCOL_HDR1)
      ok = bleuart.readBytes(&msg->protocol_header[1], 1);
    ok = bleuart.readBytes(&msg->protocol_header[2], 1);
  } while (not ok || msg->protocol_header[2] != PROTOCOL_HDR2);

  // read length
  ok = bleuart.readBytes(&msg->length, 1);
  if (not ok)
    return 0;

  if (msg->length > MAX_VALUES)
    msg->length = MAX_VALUES;

  // read data
  return bleuart.readBytes(reinterpret_cast<unsigned char *>(msg->data),
                           msg->length * sizeof(uint16_t));
  }*/
