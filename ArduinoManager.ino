#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Chrono.h>

// Sensoren
const int rightIrSensor = 13;

const int leftIrSensor = 9;

const int topTrigPin = A0;
const int topEchoPin = A1;

const int bottomIrSensor = A2;

// Obere Straße der Kreuzung
const int topRedLedPin = 2;
const int topYellowLedPin = 3;
const int topGreenLedPin = 4;

// Untere Straße der Kreuzung
const int bottomRedLedPin = A3;
const int bottomYellowLedPin = A4;
const int bottomGreenLedPin = A5;

//Rechte Straße der Kreuzung
const int rightRedLedPin = 7;
const int rightYellowLedPin = 6;
const int rightGreenLedPin = 5;

// Linke Straße der Kreuzung
const int leftRedLedPin = 12;
const int leftYellowLedPin = 11;
const int leftGreenLedPin = 10;

// Stopwatch für Automatische Schaltung der Ampel
Chrono myTestChrono;
const int trafficLightSwitchTimeLimit = 30000;



void setup() {
  // Init
  Serial.begin(9600);
  setTime(9, 02, 0, 10, 10, 2024); // Setze die Uhrzeit (Stunde, Minute, Sekunde, Tag, Monat, Jahr)

  // Pin-Modes

  // OUTPUT
  pinMode(topTrigPin, OUTPUT);
  pinMode(topRedLedPin, OUTPUT);
  pinMode(topYellowLedPin, OUTPUT);
  pinMode(topGreenLedPin, OUTPUT);

  pinMode(rightRedLedPin, OUTPUT);
  pinMode(rightYellowLedPin, OUTPUT);
  pinMode(rightGreenLedPin, OUTPUT);

  pinMode(bottomRedLedPin, OUTPUT);
  pinMode(bottomYellowLedPin, OUTPUT);
  pinMode(bottomGreenLedPin, OUTPUT);

  pinMode(leftRedLedPin, OUTPUT);
  pinMode(leftYellowLedPin, OUTPUT);
  pinMode(leftGreenLedPin, OUTPUT);


  // INPUT
  pinMode(rightIrSensor, INPUT);

  pinMode(leftIrSensor, INPUT);

  pinMode(bottomIrSensor, INPUT);

  pinMode(topEchoPin, INPUT);

  // Setzt zum Start die obere und untere Ampel auf Grün und die seitlichen Ampeln auf Rot
  digitalWrite(topRedLedPin, LOW);
  digitalWrite(topYellowLedPin, LOW);
  digitalWrite(topGreenLedPin, HIGH);

  digitalWrite(bottomRedLedPin, LOW);
  digitalWrite(bottomYellowLedPin, LOW);
  digitalWrite(bottomGreenLedPin, HIGH);

  digitalWrite(rightRedLedPin, HIGH);
  digitalWrite(rightYellowLedPin, LOW);
  digitalWrite(rightGreenLedPin, LOW);

  digitalWrite(leftRedLedPin, HIGH);
  digitalWrite(leftYellowLedPin, LOW);
  digitalWrite(leftGreenLedPin, LOW);

  triggerSendData(10);
 
}

void loop() {
 
  digitalWrite(topTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(topTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(topTrigPin, LOW);

  float topUltraSonicData = pulseIn(topEchoPin, HIGH);
  float distanceTop = topUltraSonicData * 0.017015;

  //Serial.println("Distanz Oben");
  //Serial.println(distanceTop);
  delay(500);

  int rightIrData = digitalRead(rightIrSensor);
  int rightGreenLedState = digitalRead(rightGreenLedPin);
  int rightYellowLedState = digitalRead(rightYellowLedPin);
  int rightRedLedState = digitalRead(rightRedLedPin);

  int topGreenLedState = digitalRead(topGreenLedPin);
  int topYellowLedState = digitalRead(topYellowLedPin);
  int topRedLedState = digitalRead(topRedLedState);

  int bottomIrData = digitalRead(bottomIrSensor);
 
  int leftIrData = digitalRead(leftIrSensor);


  if(distanceTop < 2.98){
    triggerSendData(1);
  }
 
  if(rightIrData == LOW){
    triggerSendData(2);
  }

  if(bottomIrData == LOW){
    triggerSendData(3);
  }
 
  if(leftIrData == LOW){
    triggerSendData(4);
  }
 
  // Wenn der rechte/linke Sensor etwas misst und der obere/untere nicht
  if(rightIrData == LOW ||leftIrData == LOW && distanceTop > 2.98 && bottomIrData != LOW || myTestChrono.hasPassed(trafficLightSwitchTimeLimit)){
    // Wenn obere Ampel nicht schon grün ist oder gelb und wechselt, dann schalte Ampel um
    if(rightGreenLedState != HIGH && rightYellowLedState != HIGH && rightRedLedState != LOW){
      switchFromGreenToRed(topGreenLedPin, topYellowLedPin, topRedLedPin, bottomGreenLedPin, bottomYellowLedPin, bottomRedLedPin);
      switchFromRedToGreen(rightGreenLedPin, rightYellowLedPin, rightRedLedPin, leftGreenLedPin, leftYellowLedPin, leftRedLedPin);
     
      myTestChrono.restart();
   
     
    }
  }

  // Wenn der obere/untere Sensor etwas misst und der rechte/linke nicht
  if(distanceTop < 2.98 || bottomIrData == LOW && rightIrData != LOW && leftIrData != LOW || myTestChrono.hasPassed(trafficLightSwitchTimeLimit)){
    if(topGreenLedState != HIGH && topYellowLedState != HIGH && topRedLedState != LOW){
      switchFromGreenToRed(rightGreenLedPin, rightYellowLedPin, rightRedLedPin, leftGreenLedPin, leftYellowLedPin, leftRedLedPin);
      switchFromRedToGreen(topGreenLedPin, topYellowLedPin, topRedLedPin, bottomGreenLedPin, bottomYellowLedPin, bottomRedLedPin);
     
      myTestChrono.restart();
    }
  }


}

void switchFromGreenToRed(int greenLamp, int yellowLamp, int redLamp, int greenLampOppSite, int yellowLampOppSite, int redLampOppSite){
  digitalWrite(greenLamp, LOW);
  digitalWrite(greenLampOppSite, LOW);
  digitalWrite(yellowLamp, HIGH);
  digitalWrite(yellowLampOppSite, HIGH);
  delay(4000);
  digitalWrite(yellowLamp, LOW);
  digitalWrite(yellowLampOppSite, LOW);
  digitalWrite(redLampOppSite, HIGH);
  digitalWrite(redLamp, HIGH);
}

void switchFromRedToGreen(int greenLamp, int yellowLamp, int redLamp, int greenLampOppSite, int yellowLampOppSite, int redLampOppSite){
  digitalWrite(yellowLamp, HIGH);
  digitalWrite(yellowLampOppSite, HIGH);
  delay(2500);
  digitalWrite(redLamp, LOW);
  digitalWrite(redLampOppSite, LOW);
  digitalWrite(yellowLamp, LOW);
  digitalWrite(yellowLampOppSite, LOW);
  digitalWrite(greenLamp, HIGH);
  digitalWrite(greenLampOppSite, HIGH);

}

void triggerSendData(int Crossroad){

  int actualCrossroad = Crossroad;
 
  String currentDateTime = " test";
  String emptyString = "test ";

  StaticJsonDocument<200> doc;
  doc["zahl"] = emptyString;
  doc["zeichenfolge"] = currentDateTime ;
  doc["kreuzung"] = actualCrossroad;

  String output;
  serializeJson(doc, output);

  // json string über Serial ausgeben
  Serial.println(output);

}