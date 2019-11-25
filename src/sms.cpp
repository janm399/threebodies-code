// #include "sms.h"
// #include <algorithm>
// #include <memory>
// #include <string>
// #include <vector>
// #include "pins.h"

// payload transport::last_payload() const { return this->lp; }

// message transport::last_message() const { return this->lm; }

// void transport::print(const payload payload) {
//   serial.print(payload.c_str());
//   loop();
// }

// void transport::println(const payload payload) {
//   serial.println(payload.c_str());
//   loop();
// }

// void transport::begin() {
//   serial.begin(9600, pins::sms_rx, pins::sms_tx);  // D1 D2
//   println("AT");
//   println("AT+CSQ");
//   println("AT+CNUM");
//   println("AT+CREG=2");
//   println("AT+CPIN?");
//   println("AT+CNMI=1,2,0,0,0");
//   println("AT+CMGF=1");

//   delay(10000);  // magic delay for network conn
// }

// void transport::send(const phone_number phone_number, const payload payload)
// {
//   auto cmd = "AT+CMGS=\"" + phone_number + "\"";
//   println(cmd);
//   print(payload.c_str());
//   serial.write(26);
// }

// void transport::loop() {
//   delay(500);
//   // while (Serial.available())
//   // {
//   //     serial.write(Serial.read());
//   // }
//   this->lp = "";
//   while (serial.available()) {
//     char c = serial.read();
//     this->lp += c;
//     Serial.write(c);
//   }

//   if (this->lp.length() > 3) {
//     auto cmd = this->lp.substr(this->lp.length() - 4, 2);
//     this->lm.code = cmd[0];
//     this->lm.param = 0x30 - cmd[1];
//   } else {
//     this->lm.code = 0;
//   }
// }
