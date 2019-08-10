/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

const int outputPins[4] = { 2, 3, 4, 5 };

void intro(){
  delay(1000);
  analogWrite(outputPins[ 0 ], 200);
  analogWrite(outputPins[ 1 ], 200);
  analogWrite(outputPins[ 2 ], 200);
  analogWrite(outputPins[ 3 ], 200);
  delay(1000);
  analogWrite(outputPins[ 0 ], 0);
  analogWrite(outputPins[ 1 ], 0);
  analogWrite(outputPins[ 2 ], 0);
  analogWrite(outputPins[ 3 ], 0);
}

#define ledPin 17

void setup()
{
  pinMode(ledPin, OUTPUT); // Ausgabe LED festlegen
}


void loop()
{
  intro();  
}
