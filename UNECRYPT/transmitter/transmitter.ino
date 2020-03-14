#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";
int PulseSensorPurplePin = 0; 
int Signal;                // holds the incoming raw data. Signal value can range from 0-1024
int Threshold = 550;            // Determine which Signal to "count as a beat", and which to ingore.

void setup() {
  Serial.begin(9600);
  pinMode(7, INPUT); // Setup for leads off detection LO +
  pinMode(6, INPUT); // Setup for leads off detection LO -
  
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  Signal = analogRead(PulseSensorPurplePin);
  Serial.println(Signal); 
  radio.write(&Signal, sizeof(Signal));
  delay(1000);
}
