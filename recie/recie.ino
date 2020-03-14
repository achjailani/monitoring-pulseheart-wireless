#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";

int LED = 128;
const unsigned char MDSInv[4][4] = {
  {12, 12, 13, 4},
  {3,  8, 4, 5},
  {7, 6, 2, 14},
  {13, 9, 9, 13},
};

const unsigned char sbox_desc[16] = {5, 14, 15, 8, 12, 1, 2, 13, 11, 4, 6, 3, 0, 7, 9, 10};
const unsigned char WORDFILTER_DES = 0xF;
void ledDecrypt128(unsigned char state[4][4], unsigned char key1[4][4], unsigned char key2[4][4]);


/**
unsigned char key [16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char key1[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
unsigned char state[4][4] = {{3,13,14,12}, {11,2,10,0}, {8,5,0,12}, {13,11,10,1}};
*/

unsigned char key1[4][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
unsigned char key2[4][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
//unsigned char state[4][4] = {{3, 4, 3, 4,}, {3, 13, 0, 13}, {1, 9, 4, 10}, {4, 0, 9, 0 }};

unsigned char state[4][4];
unsigned char data[16];


void setup() 
{
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address); // setting the address at which we will recieve the data
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  
}

void loop() {
  while(radio.available()){
    radio.read(&data, sizeof(data));
//    recieve(state, data);
//    ledDecrypt128(state, key1, key1);
//    
//    for(int i=0; i<4; i++){
//      for(int y=0; y<4; y++){
//        Serial.print(state[i][y]);
//        Serial.print(" ");
//      }
//    }
//    Serial.println();
    recieve(state, data);
    ledDecrypt128(state, key1, key1);
    iLoad(state);
    memset(data, 0, sizeof(data));
  }
}



unsigned char FieldMultDesc(unsigned char a, unsigned char b)
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
  return ret & WORDFILTER_DES;
}
void MixColumnDesc(unsigned char state[4][4])
{ 
  int i, j, k;
  unsigned char tmp[4];
  for(j = 0; j < 4; j++){
    for(i = 0; i < 4; i++) {
      unsigned char sum = 0;
      for(k = 0; k < 4; k++)
        sum ^= FieldMultDesc(MDSInv[i][k], state[k][j]);
      tmp[i] = sum;
    }
    for(i = 0; i < 4; i++)
      state[i][j] = tmp[i];
  }
}


void shiftRowsDesc(unsigned char state[4][4]){
   int length=sizeof(state[0]), i, x, y, n, tmp;
   unsigned char arr[4];

   for(x=0; x<length; x++) {
        for(y=0; y<length; y++)
            arr[y] = state[x][y];

        n = abs(4 - x) % length;
        while(n>0){
            tmp = arr[0];
            for(i=0; i<length; i++){
                if(i != (length - 1))
                    arr[i] = arr[i+1];
                else
                    arr[i] = tmp;
            }
            n--;
        }

        for(y=0; y<length; y++)
            state[x][y] = arr[y];
   }
}

void SubCellDesc(unsigned char state[4][4])
{
  int i,j;
  for(i = 0; i < 4; i++)
    for(j = 0; j <  4; j++)
      state[i][j] = sbox_desc[state[i][j]];
}


void AddConstantsDesc(unsigned char state[4][4], int r)
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

void ledRoundDecrypt(unsigned char state[4][4], int round) {
   MixColumnDesc(state);
   shiftRowsDesc(state);
   SubCellDesc(state);
   AddConstantsDesc(state, round);
}

void AddKey(unsigned char state[4][4], unsigned char* keyBytes, int step)
{
  int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            state[i][j] ^= keyBytes[(4*i+j+step*16)%(LED/4)];
}

void keyXOR(unsigned char state[4][4], unsigned char key[4][4]) 
{
  int length = sizeof(state[0]), i, y;
  for(i=0; i<length; i++)
    for(y=0; y<length; y++)
      state[i][y] ^= key[i][y];
}

void Show(unsigned char state[4][4]);
void ledDecrypt128(unsigned char state[4][4], unsigned char key1[4][4], unsigned char key2[4][4])
{ 
  const int S = 12;
  int length = sizeof(state[0]), i, y;

  keyXOR(state, key1);
  for(i=0; i <S; i++) {
    for(y=0; y<length; y++)
      ledRoundDecrypt(state, (11-i)*4 + (3-y));
    
    if(i%2==0)
      keyXOR(state, key1);
    else
      keyXOR(state, key2);
  }

}


void recieve(unsigned char state[4][4], unsigned char recieve[16]) {
  byte i,y, len=sizeof(state[0]), x=0;

  for(i=0; i<len; i++){
    for(y=0; y<len; y++){
       state[i][y] = recieve[x]; 
        x++;
    }
  }
}

void iLoad(unsigned char state[4][4]) {
  unsigned int tmp[10], x=0,i,y,k=0;
  unsigned int data[16];
  for(i=0; i<4; i++){
    for(y=0; y<4; y++){
      data[x] = state[i][y]; 
      x++;
    }
  }

  x=0;
  byte len=(sizeof(data)/sizeof(data[0]));
  for(i=0; i<len; i++){
    if(data[i]==10){
      tmp[x]=k;x++;k=0;
    } else if(i==(len-1) && k!=0){
      tmp[x]=k; x++;
    }else {
      k = 10*k+data[i];
    }
  }

  for(i=0; i<x; i++){
    Serial.println(tmp[i]);
  }
}
