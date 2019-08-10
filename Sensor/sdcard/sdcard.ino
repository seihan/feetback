/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>

// define hexadecimal data
uint8_t data1[10] = { 0x30, 0x31, 0x32, 0x33, 0x00, 0x35, 0x36, 0x37, 0x38, 0x39};
uint8_t data2[10] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};

File dataFile;
bool append = false;

void setup() {
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(12)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  if (SD.exists("data.txt")) {
    Serial.println("data.txt exists.");
    append = true;
  } else {
    Serial.println("data.txt doesn't exist.");
  }

  if (!append) {
    Serial.println("Creating data.txt...");
  }
  dataFile = SD.open("data.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (dataFile) {
    if (append) {
      Serial.print("Append to data.txt...");
      dataFile.write(data2, 10);
    } else {
      Serial.print("Writing to data.txt...");
      dataFile.write(data1, 10);
    }
    // close the file:
    dataFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data.txt");
  }

  // re-open the file for reading:
  dataFile = SD.open("data.txt");
  if (dataFile) {
    Serial.println("data.txt:");

    // read from the file until there's nothing else in it:
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    // close the file:
    dataFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data.txt");
  }
}

void loop() {
  // nothing happens after setup
}
