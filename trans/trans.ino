#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";
byte x=0,n=0;;

int PulseSensorPurplePin = 0; 
int Threshold = 550, Signal, adds;            // Determine which Signal to "count as a beat", and which to ingore.
boolean sta = false;

int LED = 128;

unsigned char data[16];


const unsigned char MDS[4][4] = {
  {4,  1, 2, 2},
  {8,  6, 5, 6},
  {11,14,10, 9},
  {2,  2,15,11},
};

const unsigned char sbox[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};
const unsigned char WORDFILTER = 0xF;
void ledEncrypt128(unsigned char state[4][4], unsigned char key1[4][4], unsigned char key2[4][4]);

/**
* Example data that would be sent
*/
  unsigned char key1[4][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
  unsigned char key2[4][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
  //unsigned char state[4][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};

  unsigned char state[4][4];
  /*
  unsigned char key1[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  unsigned char key2[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  unsigned char state[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  */



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
    unsigned char temp[10];
  unsigned int digit[10], len, second=Signal;
  itoa(Signal, temp, 10);
  len=strlen(temp);

  for(int i=(len-1); i>=0; i--){
   if (x%16==0 && x!=0){
    if (i<0) {
      for(int b=(16-(len+1)); b<16; b++){data[b] = 10;}
      Serial.println();
      for(int b=0; b<16; b++){
        Serial.print(data[b]);
        Serial.print(" ");
      }
      Serial.println();
      delay(2000);
      broadCast(state, key1, key2, data);
      x=0;
      for(int n=(len-1); n>=0; n--){
         digit[n] = second%10;
         second/=10;
      }
      for(int n=0; n<strlen(temp); n++){
        data[x] = digit[n];
        x++;
      }
      data[x] = 10;
      x++;
      Serial.print(x);
      break;
    } else {
      Serial.println();
      for(int b=0; b<16; b++){
        Serial.print(data[b]);
        Serial.print(" ");
      }
      Serial.println();
      broadCast(state, key1, key2, data);x=0;
      Serial.print(x);
     // break;
    } 
   }
    digit[i] = Signal%10;Signal/=10;
  }
  for(int i=0; i<strlen(temp); i++) {data[x] = digit[i];x++;}
  data[x] = 10;x++;
  delay(500);
}

/**
* TRANSMITTING DATA WITH LIGHTWEIGHT ENCRYPTION DEVICE IS AVAILABLE IN THIS SCOPE
* all the functions belew are for encryption process, we should focused above function and do broadcasting
* 
*/

void iLoad(unsigned int signals, unsigned char data[16]){
  
  unsigned char temp[10];
  unsigned int digit[10], len, second=signals;
  itoa(signals, temp, 10);
  len=strlen(temp);

  for(int i=(len-1); i>=0; i--){
   if (x%16==0 && x!=0){
    if (i<0) {
      for(int b=(16-(len+1)); b<16; b++){data[b] = 10;}
      Serial.println();
      for(int b=0; b<16; b++){
        Serial.print(data[b]);
        Serial.print(" ");
      }
      Serial.println();
      delay(2000);
      broadCast(state, key1, key2, data);
      x=0;
      for(int n=(len-1); n>=0; n--){
         digit[n] = second%10;
         second/=10;
      }
      for(int n=0; n<strlen(temp); n++){
        data[x] = digit[n];
        x++;
      }
      data[x] = 10;
      x++;
      Serial.print(x);
      break;
    } else {
      Serial.println();
      for(int b=0; b<16; b++){
        Serial.print(data[b]);
        Serial.print(" ");
      }
      Serial.println();
      broadCast(state, key1, key2, data);x=0;
      Serial.print(x);
//      break;
    } 
   }
    digit[i] = signals%10;signals/=10;
  }
  for(int i=0; i<strlen(temp); i++) {data[x] = digit[i];x++;}
  data[x] = 10;x++;
}

void broadCast(unsigned char state[4][4], unsigned char key1[4][4], unsigned char key2[4][4], unsigned char data[16]){
  iDidension(state, data);
  ledEncrypt128(state, key1, key2);
  sends(state);
  memset(data, 0, sizeof(data));
}
void AddConstants(unsigned char state[4][4], int r)
{
  const unsigned char RC[48] = {
    0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F,
    0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E, 0x1D, 0x3A, 0x35, 0x2B,
    0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E,
    0x1C, 0x38, 0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A,
    0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
  };
  
  state[1][0] ^= 1;
  state[2][0] ^= 2;
  state[3][0] ^= 3;

  state[0][0] ^= (LED>>4)&0xf;
  state[1][0] ^= (LED>>4)&0xf;
  state[2][0] ^= LED & 0xf;
  state[3][0] ^= LED & 0xf;

  unsigned char tmp = (RC[r] >> 3) & 7;
  state[0][1] ^= tmp;
  state[2][1] ^= tmp;
  tmp =  RC[r] & 7;
  state[1][1] ^= tmp;
  state[3][1] ^= tmp;
}

void SubCell(unsigned char state[4][4])
{
  int i,j;
  for(i = 0; i < 4; i++)
    for(j = 0; j <  4; j++)
      state[i][j] = sbox[state[i][j]];
}

unsigned char FieldMult(unsigned char a, unsigned char b)
{
  const unsigned char ReductionPoly = 0x3;
  unsigned char x = a, ret = 0;
  int i;
  for(i = 0; i < 4; i++) {
    if((b>>i)&1) ret ^= x;
    if(x&0x8) {
      x <<= 1;
      x ^= ReductionPoly;
    }
    else x <<= 1;
  }
  return ret & WORDFILTER;
}

void MixColumn(unsigned char state[4][4])
{ 
  int i, j, k;
  unsigned char tmp[4];
  for(j = 0; j < 4; j++){
    for(i = 0; i < 4; i++) {
      unsigned char sum = 0;
      for(k = 0; k < 4; k++)
        sum ^= FieldMult(MDS[i][k], state[k][j]);
      tmp[i] = sum;
    }
    for(i = 0; i < 4; i++)
      state[i][j] = tmp[i];
  }
}


void ShiftRow(unsigned char state[4][4])
{
  int i, j;
  unsigned char tmp[4];
  for(i = 1; i < 4; i++) {
    for(j = 0; j < 4; j++)
      tmp[j] = state[i][j];
    for(j = 0; j < 4; j++)
      state[i][j] = tmp[(j+i)%4];
  }
}


void ledRound(unsigned char state[4][4], int round) {
   AddConstants(state, round);
   SubCell(state);
   ShiftRow(state);
   MixColumn(state);
}

void keyXOR(unsigned char state[4][4], unsigned char key[4][4]) 
{
  int length = sizeof(state[0]), i, y;
  for(i=0; i<length; i++)
    for(y=0; y<length; y++)
      state[i][y] ^= key[i][y];
}

void Show(unsigned char state[4][4]);
void ledEncrypt128(unsigned char state[4][4], unsigned char key1[4][4], unsigned char key2[4][4])
{ 
  const int S = 12;
  int length = sizeof(state[0]), i, y;

  keyXOR(state, key1);
  for(i=0; i <S; i++) {
    for(y=0; y<length; y++)
      ledRound(state, i*4 + y);
    
    if(i%2==0)
      keyXOR(state, key1);
    else
      keyXOR(state, key2);
  }
}

void sends(unsigned char state[4][4]) {
  unsigned char data[16];
  byte i,y, len=sizeof(state[0]), x=0;
  for(i=0; i<len; i++){
    for(y=0; y<len; y++){
      data[x] = state[i][y]; x++;
    }
  }
  radio.write(&data, sizeof(data));
}

void iDidension(unsigned char state[4][4], unsigned char data[16]) {
  byte i,y, len=sizeof(state[0]), x=0;
  for(i=0; i<len; i++){
    for(y=0; y<len; y++) {
      state[i][y] = data[x];
      x++;
    }
  }
}
void Show(unsigned char state[4][4]) {
  int i,y, length = sizeof(state[0]);
  Serial.print("\nCIPHERTEXT : ");
  for(i=0; i<length; i++){
    for(y=0; y<length; y++) {
      Serial.print(state[i][y]);
      Serial.print(" ");
    }
  }
}
