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
