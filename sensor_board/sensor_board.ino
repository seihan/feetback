int duration = 0;

// 2d array to store values
int values[16][5] = {}; //[rows][columns]

//mux control pins
int s0 = 15;
int s1 = 7;
int s2 = 11;
int s3 = 31;

//mux in "SIG" pin
int SIG_pin = A3;

//not correctly working digital output pins
/*const int columnPins[5] = {
  27, 30, 16, 6, 20
};*/

int readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}

void setup(){
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);
  /*pinMode(27, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(20, OUTPUT);*/
  pinMode(SIG_pin, INPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  /*digitalWrite(27, LOW);
  digitalWrite(30, LOW);
  digitalWrite(16, LOW);
  digitalWrite(6, LOW);
  digitalWrite(20, LOW);*/

  //initialize array with zeros
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 5; j++){
      values[i][j] = 0;
      }
   }

  Serial.begin(115200);
}

void loop(){
  //read values
  duration = millis();
  for(int i = 0; i < 5; i++){
    //digitalWrite(columnPins[i], HIGH);
    for(int j = 0; j < 16; j++){
      values[ j ][ i ] = readMux(j);
      delay(1);
    }
    //digitalWrite(columnPins[i], LOW);
  }
  duration = millis() - duration;
  
  //print values on serial monitor
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 5; j++){
      Serial.print(values[ i ][ j ]);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.print("duration = ");
  Serial.print(duration);
  Serial.println("ms ---------------------");
  delay(300);
  //write values via serial to processing
      /*Serial.print (values[ i ][ j ], DEC );
      if(j == 4){
        Serial.print(";");
      }
      else{
        Serial.print(",");
      }
    }
    if(i == 15){
      Serial.print("|");
    }
  }*/

  //reset values to zero
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 5; j++){
      values[i][j] = 0;
      }
   }
}
