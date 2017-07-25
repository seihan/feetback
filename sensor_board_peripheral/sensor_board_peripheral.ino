int duration = 0;

// 2d array to store values
int values[16][5] = {};

//mux control pins
int s0 = 16;
int s1 = 15;
int s2 = 11;
int s3 = 31;

//mux in "SIG" pin
int SIG_pin = 30;

//analog input pins
const int columnPins[5] = {
  A0, A1, A2, A3, A4
};

//Set channel HIGH
void writeMux(int channel){
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

  //turn on the SIG pin
  //digitalWrite(SIG_pin, HIGH);
  //return the value
 // return val;
}

void setup(){
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 
  pinMode(SIG_pin, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  digitalWrite(SIG_pin, HIGH);

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
  for(int i = 0; i < 16; i ++){//rows
    writeMux(i); //set row HIGH
    for(int j = 0; j < 5; j++){//columns
      values[ i ][ j ] = analogRead(columnPins[j]);
    }
  }
  duration = millis() - duration;
  //print values on serial
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 5; j++){
      Serial.print (values[ i ][ j ] );
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.print("duration = ");
  Serial.print(duration);
  Serial.println("ms ---------------------");
  delay(1000);
   for(int i = 0; i < 16; i++){
    for(int j = 0; j < 5; j++){
      values[i][j] = 0;
      }
   }
}



