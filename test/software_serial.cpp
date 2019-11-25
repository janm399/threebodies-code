#include <EspSoftwareSerial/SoftwareSerial.h>

std::shared_ptr<software_serial_control> software_serial_mock::get(int rx_pin) {
  return get(-1, rx_pin, -1);
}

std::shared_ptr<software_serial_control> software_serial_mock::get(int baud,
                                                                   int rx_pin,
                                                                   int tx_pin) {
  auto found = std::find_if(
      std::begin(mock_controls), std::end(mock_controls), [&](auto &&c) {
        auto c_baud = std::get<0>(c);
        auto c_rx_pin = std::get<1>(c);
        auto c_tx_pin = std::get<2>(c);
        return (c_baud == baud || baud == -1 || c_baud == -1) &&
               (c_rx_pin == rx_pin || rx_pin == -1 || c_rx_pin == -1) &&
               (c_tx_pin == tx_pin || tx_pin == -1 || c_tx_pin == -1);
      });
  if (found != std::end(mock_controls)) {
    // if (std::get<0>(*found) != baud || std::get<1>(*found) != rx_pin ||
    //     std::get<2>(*found) != tx_pin)
    //   throw duplicate_software_serial{
    //       .baud = baud, .rx_pin = rx_pin, .tx_pin = tx_pin};
    return std::get<3>(*found);
  }
  std::cout << "Creating new control for baud = " << baud
            << ", rx_pin = " << rx_pin << ", tx_pin = " << tx_pin << std::endl;
  auto control = std::make_shared<software_serial_control>();
  mock_controls.emplace_back(std::make_tuple(baud, rx_pin, tx_pin, control));
  return control;
};

// software_serial_control::software_serial_control() { tx_position = 0; }

void software_serial_control::write(payload payload) {
  auto found = auto_responses.find(payload);
  if (found != std::end(auto_responses)) {
    rx_position = 0;
    rx = found->second;
    std::cout << "found " << rx << std::endl;
  }

  tx = tx + payload;
}

void software_serial_control::add_auto_response(
    payload request, payload response,
    std::chrono::duration<uint, std::milli> timeout) {
  auto_responses[request] = response;
}

bool software_serial_control::available() const {
  return rx.length() > rx_position;
}

char software_serial_control::read() {
  char c = rx[rx_position];
  rx_position++;
  return c;
}

payload software_serial_control::get_tx() const { return tx; }