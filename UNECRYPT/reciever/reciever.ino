#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";
int PulseSensorPurplePin = 0; 
int Signal;                // holds the incoming raw data. Signal value can range from 0-1024
int Threshold = 550; 

void setup() {
  Serial.begin(9600);
  pinMode(7, INPUT); // Setup for leads off detection LO +
  pinMode(6, INPUT); // Setup for leads off detection LO -
  
  radio.begin();
  radio.openReadingPipe(0, address); // setting the address at which we will recieve the data
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  unsigned int data;
  while(radio.available()){
    radio.read(&data, sizeof(data));
    Serial.print(data);
    Serial.print(" ");
  }
}
