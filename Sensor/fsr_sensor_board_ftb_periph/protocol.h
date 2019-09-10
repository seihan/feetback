/*
 * Common message format for value streams.
 */

#pragma once

#ifndef MAX_VALUES
#define MAX_VALUES 2
#endif


static const unsigned char PROTOCOL_HDR1 = 'M';
static const unsigned char PROTOCOL_HDR2 = 'V';
static const int BLE_MTU = 60; // max bytes per packet
static const int SERIAL_MTU = 1024; // max bytes per packet

struct measure_t {
  uint16_t offset;
  uint16_t value;
};

struct balance_t {
  uint16_t front;
  uint16_t rear;
};

struct message_t {
  unsigned char protocol_header[2] {PROTOCOL_HDR1, PROTOCOL_HDR2}; // data start marker
  uint16_t length; // number of measurements 
  measure_t data[MAX_VALUES];
  balance_t bala[1];
};

void sendtoPeripherals(unsigned char* str, uint16_t len);

/* Send a message out through the UART.
 */
void send_to_prphs(message_t *msg) {
  if (msg->length > MAX_VALUES) {
    msg->length = MAX_VALUES;
  }

  sendtoPeripherals(reinterpret_cast<unsigned char *>(msg), sizeof(*msg));
}

/*void send_to_central(message_t *msg){
  if (msg->length > MAX_VALUES) {
    msg->length = MAX_VALUES;
  }

  int to_send = sizeof(*msg);
  int sent = 0;
  while (to_send > 0) {
    int len = to_send > BLE_MTU ? BLE_MTU : to_send;
    int bytesSent = bleuart.write(reinterpret_cast<unsigned char *>(msg) + sent, len);
    to_send -= bytesSent;
    sent += bytesSent;
  }
}*/

void send_to_serial(message_t *msg) {
  if (msg->length > MAX_VALUES) {
    msg->length = MAX_VALUES;
  }

  unsigned to_send = sizeof(*msg);
  unsigned sent = 0;
  while (to_send > 0) {
    unsigned len = to_send > SERIAL_MTU ? SERIAL_MTU : to_send;
    int bytesSent = Serial.write(reinterpret_cast<unsigned char *>(msg) + sent, len);
    to_send -= bytesSent;
    sent += bytesSent;
  }
}

/* Read a message into a buffer
 * @return number of data bytes read, 0 on error.
 */
/*int receive_message(message_t *msg) {
  int ok = 0;

  // find message start
  do {
    while (not ok || msg->protocol_header[0] != PROTOCOL_HDR1)
      ok = bleuart.readBytes(&msg->protocol_header[0], 1);
    ok = bleuart.readBytes(&msg->protocol_header[1], 1);
  } while (not ok || msg->protocol_header[1] != PROTOCOL_HDR2);

  // read length
  ok = bleuart.readBytes(reinterpret_cast<unsigned char *>(&msg->length), sizeof(msg->length));
  if (not ok)
    return 0;

  if (msg->length > MAX_VALUES)
    msg->length = MAX_VALUES;

  // read data
  return bleuart.readBytes(reinterpret_cast<unsigned char *>(msg->data), sizeof(msg->data));
}*/
