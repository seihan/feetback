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
