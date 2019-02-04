#include <TimerOne.h>
#include "TimeLib.h"
#include "SchedulerARMAVR.h"

#define BITS_PER_SYMBOL 8
#define SYMBOL_PERIOD 1000000 // in micro sec
#define SAMPLES_PER_BIT 4
#define SAMPLES_PER_SYMBOL BITS_PER_SYMBOL*SAMPLES_PER_BIT

int sensorPin = A7; 
int sensorValue = 0;
int blockCnt = 0;
int th = 0;
int truth[BITS_PER_SYMBOL] = {0, 1, 0, 0, 0, 1, 1, 0};
int sensed[SAMPLES_PER_SYMBOL];
int sensedId = 0;
int sumSensed[BITS_PER_SYMBOL];
bool compress = false;
int minValue = 255;
int maxValue = 0;
int memSensed[SAMPLES_PER_SYMBOL];
bool findEdge = false;
//time_t timeNow;

void sample_signal() {
  if(blockCnt == 0) {
    int trash = analogRead(sensorPin);
    if (sensedId >= 2) {
      Serial.println("trashing complete!");
      sensedId = SAMPLES_PER_SYMBOL;
    }
  }
  else if(blockCnt == 1) {
    sensorValue = analogRead(sensorPin);
    sensed[sensedId] = sensorValue;
    th += sensorValue;
  }
  else {
    // compression of data
    if(compress && sensedId == 0) {
      Serial.println("compressed");
//      for(int i=0;i<SAMPLES_PER_SYMBOL;i++) sensed[i] = map(sensed[i], minValue, maxValue, 0, 2*maxValue);
//      for(int i=0;i<SAMPLES_PER_SYMBOL;i++) sensed[i] /= 0.5*(maxValue-sensed[i])+1;
      for(int i=0;i<SAMPLES_PER_SYMBOL;i++) sensed[i] /= 20;
      compress = false;
    }
    sensorValue = analogRead(sensorPin)-th;
    sensed[sensedId] += sensorValue;
  }
  sensedId++;
  if(sensedId >= SAMPLES_PER_SYMBOL) {
    if(blockCnt == 1) {
      th = th/(SAMPLES_PER_SYMBOL)-20;
      Serial.print("The thresh is: ");
      Serial.println(th);
      for(int i=0;i<SAMPLES_PER_SYMBOL;i++) sensed[i] = 0;
    }
    blockCnt++;
    sensedId=0;
  }

  for(int i=0;i<SAMPLES_PER_SYMBOL;i++) {
    Serial.print(sensed[i]);
    Serial.print(" ");
  }
  Serial.print(" : ");
  Serial.print(sensedId);
  Serial.print(" ");
  Serial.print(minValue);
  Serial.println();
}

void setup() { 
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  Scheduler.startLoop(loop2);
  Timer1.initialize(SYMBOL_PERIOD/SAMPLES_PER_BIT); //1200 bauds oversampled by factor 4
  Timer1.attachInterrupt(sample_signal);
}

void loop() {
//  timeNow = now();
//  bool compress = false;
//  int minValue;
//  int maxValue;

  if(sensedId == 0 && blockCnt >= 2) {
    minValue = 255;
    maxValue = 0;
    //   check if compressing or not
    for(int i=0;i<SAMPLES_PER_SYMBOL;i++) {
      if(sensed[i] > 127 && blockCnt >= 2) compress = true;
      if(sensed[i] < minValue) minValue = sensed[i];
      if(sensed[i] > maxValue) maxValue = sensed[i];
    }
    memcpy(memSensed, sensed, sizeof(sensed));
    findEdge = true;
    delay(SAMPLES_PER_BIT*SYMBOL_PERIOD/1000);
  }
  yield();
}

// find first edge and decode message
void loop2() {
  if(findEdge && blockCnt >= 2) {
    // find first edge
    int valueBefore = memSensed[SAMPLES_PER_SYMBOL-1];
    int valueAfter = 0;
    int edge = 0;
    int edgeId = 0;
    int average = 0; for(int i=0;i<SAMPLES_PER_SYMBOL;i++) average += memSensed[i]; average/=SAMPLES_PER_SYMBOL;
    for(int i=0;i<SAMPLES_PER_SYMBOL;i++) {
      int aveSamples = 0;
      valueAfter = memSensed[i];
      if(edge < valueAfter - valueBefore) {
        for(int j=0;j<SAMPLES_PER_BIT;j++) aveSamples +=  memSensed[i+j];
        if (aveSamples/SAMPLES_PER_BIT > average) {
          edge = valueAfter - valueBefore;
          edgeId = i;
        }
      }
    }
    Serial.print("edgeId: ");
    Serial.println(edgeId);
    // sum up samples and order data from the first edge
    int orderedData[BITS_PER_SYMBOL]; for(int i=0;i<BITS_PER_SYMBOL;i++) orderedData[i] = 0;
    for(int i=0;i<SAMPLES_PER_SYMBOL;i++) {
      orderedData[i/SAMPLES_PER_BIT] += memSensed[(i+edgeId)%int(SAMPLES_PER_SYMBOL)]/SAMPLES_PER_BIT;
    }
    Serial.print("orderedData: ");
    for(int i=0;i<BITS_PER_SYMBOL;i++) {
      Serial.print(orderedData[i]);
      Serial.print(" ");
    }
    Serial.println();
    // change data into bits, LSB first?
    int orderedBits[BITS_PER_SYMBOL];
//    unsigned long int orderedBits = BITS_PER_SYMBOL*8;
    for(int i=0;i<BITS_PER_SYMBOL;i++) {
      if(orderedData[i] > average) orderedBits[i] = 1;
      else orderedBits[i] = 0;
    }
    Serial.print("orderedBits: ");
    for(int i=0;i<BITS_PER_SYMBOL;i++) {
      Serial.print(orderedBits[i]);
      Serial.print(" ");
    }
    Serial.println();
//    Serial.print("orderedBits: ");
//    Serial.println(orderedBits,BIN);
    // decode message
    int message[2*BITS_PER_SYMBOL]; // concatenate of two orderedBits in line
    for(int i=0;i<BITS_PER_SYMBOL;i++) {
      message[i] = orderedBits[i];
      message[i+BITS_PER_SYMBOL] = orderedBits[i];
    }
    Serial.print("message: ");
    for(int i=0;i<2*BITS_PER_SYMBOL;i++) {
      Serial.print(message[i]);
      Serial.print(" ");
    }
    Serial.println();
    int checkArray[BITS_PER_SYMBOL]; for(int i=0;i<BITS_PER_SYMBOL;i++) checkArray[i] = 0;
    int matchCnt = 0;
    for(int i=0;i<BITS_PER_SYMBOL;i++) {
      for(int j=0;j<BITS_PER_SYMBOL;j++) checkArray[j] += message[i+j] - truth[j];
      Serial.print("checkArray: ");
      for(int j=0;j<BITS_PER_SYMBOL;j++) {
        Serial.print(checkArray[j]);
        Serial.print(" ");
      }
      Serial.println();
      for(int j=0;j<BITS_PER_SYMBOL;j++) {
        if(checkArray[j] == 0) {
          matchCnt++;
          if (matchCnt == BITS_PER_SYMBOL) break;
        }
        else {
          matchCnt = 0;
          for(int i=0;i<BITS_PER_SYMBOL;i++) checkArray[i] = 0;
          break;
        }
      }
      if(matchCnt == BITS_PER_SYMBOL) {
        Serial.println();
        Serial.println();
        Serial.println("Found key char!");
        for(int k=0;k<BITS_PER_SYMBOL;k++) Serial.print(message[i+k]);
        Serial.println();
        Serial.println();
        Serial.println();
        break; // if bits are correctly lined up, answer must be somewhere in line
      }
    }
    delay(SAMPLES_PER_BIT*SYMBOL_PERIOD/1000);
    findEdge = false;
  }
  yield();
}
