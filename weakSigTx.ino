#include <TimerOne.h>
#include "TimeLib.h"

#define BITS_PER_SYMBOL 8
#define SYMBOL_PERIOD 1000000

int ledPin = 14;
char * msg = "Hello World!";
time_t timeNow;
int binData[BITS_PER_SYMBOL] = {0, 1, 0, 0, 0, 1, 1, 0}; // 70=F

void emit_bit() {
  int secId = timeNow % BITS_PER_SYMBOL;
  digitalWrite(ledPin, binData[secId]);
  Serial.print(timeNow);
  Serial.print(":");
  Serial.print(secId);
  Serial.print(" ");
  Serial.println(binData[secId]);
}

void setup() { 
  pinMode(ledPin, OUTPUT); 
  Serial.begin(9600);
  Timer1.initialize(SYMBOL_PERIOD);
  Timer1.attachInterrupt(emit_bit); 
}

void loop() {
  timeNow = now();
}
