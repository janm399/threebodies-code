// #include "vote.h"
// #include "pins.h"

// void vote::begin() {
//   pinMode(pins::vote_0, INPUT_PULLUP);
//   pinMode(pins::vote_now, OUTPUT);
//   digitalWrite(pins::vote_now, LOW);
//   // LEDS.addLeds<WS2812, D9, RGB>(leds, num_leds);
//   // LEDS.setBrightness(10);
// }

// vote_result vote::loop() {
//   int x = analogRead(pins::vote_0);
//   bool v0 = x > 200 && x < 300;
//   bool v1 = x > 500 && x < 600;
//   bool v2 = x > 800 && x < 900;

//   vote_result result;
//   result.value = -1;
//   if (v0 || v1 || v2) {
//     show_vote_now = false;
//     if (v0) result.value = 0;
//     if (v1) result.value = 1;
//     if (v2) result.value = 2;
//   }

//   if (show_vote_now) {
//     digitalWrite(pins::vote_now, HIGH);
//   } else {
//     digitalWrite(pins::vote_now, LOW);
//   }

//   return result;
// }

// void vote::vote_now() { show_vote_now = true; }