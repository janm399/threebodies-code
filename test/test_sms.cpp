#include <unity.h>
#include "pins.h"
#include "sms.h"

void test_foo() {
  transport t;
  Serial.begin(9600);

  auto serial = software_serial_mock::instance().get(pins::sms_rx);
  serial->add_auto_response("AT\n", "OK\n");
  serial->add_auto_response("AT+CSQ\n", "OK\n");
  serial->add_auto_response("AT+CREG=2\n", "OK\n");
  serial->add_auto_response("AT+CPIN?\n", "1,2,3\n");
  serial->add_auto_response("AT+CNMI=1,2,0,0,0\n", "OK\n");
  serial->add_auto_response("AT+CMGF=1\n", "OK\n");
  t.begin();
  t.send("1", "body");
  t.loop();

  std::cout << "Test end " << serial->get_tx() << std::endl;
}

extern "C" int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_foo);
  UNITY_END();
  return 0;
}
