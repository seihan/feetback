# feetback

:feet::back:
An embedded sensor sole project.

### Objective

Creating a new sense of balance by applying a live feedback of pressure sensors under the feet to alternative parts of the human body.

### Overview

We use microcontroller platforms, pressure sensitive textiles, small vibrating motors, wireless communication, and a lot of creativity. Prototypes are all based on the Arduino platform and breakout boards.

The project implements interaction between two basic components:
 * Pressure sensitive soles generate a live stream of sensor values.
 * Vibrating actuators react to a live input stream of values.

Different kinds of generators (sensors) and consumers (actuators) can be combined using a common protocol. The value stream can be captured, intercepted, filtered, enhanced, augmented, visualized, or relayed using a smartphone or dedicated hardware to create arbitrary audio-visual feedback for therapeutic use cases or performance art.

### Communication

In the first prototype the sensors and actuators communicate via serial link. A wireless link is provided over a Bluetooth LE serial connection. At first the connection is unidirectional from sensors to actuators. The sensors send a continuous stream of values. There is currently no need for a more sophisticated approach and this simplifies prototyping a lot.

#### Protocol

The sensor value stream is sectioned off in packets. A packet has a header, followed by the raw `uint16_t` data values. The header consists of a magic value + a length field. Consumers (loggers/actuators) can scan the serial input for the magic value to synchronize on data start.

 | Magic   | Value   | #values | value 1  | ... | value n  |
 |---------|---------|---------|----------|-----|----------|
 |`uint8_t`|`uint8_t`|`uint8_t`|`uint16_t`| ... |`uint16_t`|

The length of a single packet is therefore `3 + (2 * #values)` bytes.

#### Bandwidth and Latency

The serial interface is usually limited to 115200 baud (bits per second) on fast interfaces. On lower end platforms this might be limited to 9600 baud. Assuming 64 values in a packet (131 bytes per packet), this translates to ~109 packets per second on a fast or ~9 packets per second on a slow serial connection. Accordingly we expect a best case packet latency of ~10ms (fast) / ~110ms (slow). This ignores all other overhead and marks expected lower bounds due to the communication interface.

### Pressure Sensitive Soles

#### Hardware

#### Software

### Vibrating Actuator Wristband

#### Hardware

#### Software

### Serial Interface

Thanks to the simple serial communication, we can use commodity hardware and very basic software to generate or consume the data streams.

