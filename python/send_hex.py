#!/usr/bin/env python

# AMB2621 BLE module operation script

import serial
import time

device = '/dev/ttyUSB0'
baudrate = 115200
ser = serial.Serial(device, baudrate)

# configure module
	# define role -> central / peripheral
	# load services and characteristics
	# start advertising

# functions

# scan_callback

# connect_callback

# disconnect_callback

# bleuart_rx_callback

# bleuart_write
def getCheckSum(val):
	"This function calculates the checksum from HEX values."
	int XOR = 0; 
  for (int i = 0; string[i] != '\0';  i+= 2) // <-- increment by 2
  {
    Serial.print("i.:");
    Serial.println(string[i]);
    Serial.print("");

    // make single byte value out of 2 characters from the string...
    byte b1,b2,b;

    // First byte: hex to bin
    b1 = string[i];
    if (b1 >= 'a')
      b1 = b1 - 'a' + 10;
    else if (b1 >= 'A')
      b1 = b1 - 'A' + 10;
    else
      b1 -= '0';

    // Second byte: hex to bin
    b2 = string[i + 1];
    if (b2 >= 'a')
      b2 = b2 - 'a' + 10;
    else if (b2 >= 'A')
      b2 = b2 - 'A' + 10;
    else
      b2 -= '0';

    // Combine the two
    b = 0x10 * b1 + b2;

    XOR = XOR ^ b;
  }

  return XOR; 



while True:
    try:
	# main loop
	# bleuart_write("Hello World")
        break
    except KeyboardInterrupt:
        print("User interrupt encountered. Exiting...")
        time.sleep(1)
        exit()
    except:
        # for all other kinds of error, but not specifying which one
        print("Unknown error...")
        time.sleep(1)
        exit()

# Samsung S3+ MAC 08 D4 2B 3B 77 
