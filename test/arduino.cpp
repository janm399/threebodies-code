#include "Arduino.h"
#define HIGH 1
#define LOW 0

int analogRead(int) { return 0; }

void delay(int) {}
void pinMode(int, int) {}
void digitalWrite(int, int) {}
int digitalRead(int) { return LOW; }
unsigned long millis() { return 0; }

SoftwareSerial Serial;
