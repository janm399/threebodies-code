#pragma once
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class SoftwareSerial;

using payload = std::string;

struct duplicate_software_serial {
  int baud;
  int rx_pin;
  int tx_pin;
};

struct auto_response_record {
  payload response;
  int count;
};

class software_serial_control {
 private:
  payload tx;
  payload rx;
  unsigned int rx_position;
  std::map<payload, payload> auto_responses;
  std::map<payload, auto_response_record> auto_response_records;

 protected:
  void append(payload payload);

 public:
  // void wait_tx(payload content,
  //              std::chrono::duration<uint, std::milli> timeout);
  void add_auto_response(payload request, payload response,
                         std::chrono::duration<uint, std::milli> timeout =
                             std::chrono::milliseconds(0));
  void write(payload payload);
  bool available() const;
  char read();

  payload get_tx() const;
};

class software_serial_mock {
 private:
  std::vector<
      std::tuple<int, int, int, std::shared_ptr<software_serial_control>>>
      mock_controls;
  software_serial_mock() = default;

 public:
  static software_serial_mock &instance() {
    static software_serial_mock instance;
    return instance;
  };
  software_serial_mock(software_serial_mock const &) = delete;
  void operator=(software_serial_mock const &) = delete;

  std::shared_ptr<software_serial_control> get(int baud, int rx_pin,
                                               int tx_pin);
  std::shared_ptr<software_serial_control> get(int rx_pin);
};

class SoftwareSerial {
 private:
  std::shared_ptr<software_serial_control> control;

 public:
  void print(const char *line) { control->write(line); };
  void println(const char *line) { control->write(std::string(line) + "\n"); };
  void println(const int x) { control->write(std::to_string(x) + "\n"); };
  void println(const char c) { control->write(std::to_string(c) + "\n"); };
  void write(const char c) { control->write(std::to_string(c)); };
  char read() { return control->read(); };
  bool available() { return control->available(); }

  void begin(int baud) { begin(baud, -1, -1); }
  void begin(int baud, int rx_pin, int tx_pin) {
    control = software_serial_mock::instance().get(baud, rx_pin, tx_pin);
  }
};
