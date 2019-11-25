#include <SoftwareSerial.h>

#define HIGH 1
#define LOW 0

#define INPUT_PULLUP 0
#define OUTPUT 0
#define INPUT 0

int analogRead(int);

void delay(int);
void pinMode(int, int);
void digitalWrite(int, int);
int digitalRead(int);
unsigned long millis();

extern SoftwareSerial Serial;
