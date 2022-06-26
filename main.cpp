#include <iostream>
#include <thread>
#include "gnat.hpp"

int main() {
  gnat::gnat gnat("mosquitto, gnatt wrapper");
  gnat::mosquitto_version_t version = gnat::gnat::get_mosquitto_version();
  std::cout << "Gnat test - mosquitto version " <<
      std::get<0>(version) << "." <<
      std::get<1>(version) << "." <<
      std::get<2>(version) << std::endl;

  std::cout << "Connecting..." << std::endl;

  auto error = gnat.init();

  if (error) {
    std::cerr << error.message() << std::endl;
    return 1;
  }


  gnat.set_on_message_cb([&](int32_t mid, const std::string& topic, std::vector<uint8_t> payload) {
    std::cout << "Got message with id " << mid << " from topic " << topic << " with len " << payload.size() << ", payload: ";
    std::for_each(payload.begin(), payload.end(), [](uint8_t b) {
      std::cout << b;
    });
    std::cout << std::endl;
  });

  std::cout << "Setting credentials: " << gnat.set_credentials(SECRET_USER, SECRET_PASS) << std::endl;

  error = gnat.connect("gnat", SECRET_IP, 1883, std::chrono::milliseconds(10000));

  if (error) {
    std::cerr << error.message() << std::endl;
    return 1;
  }

  //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  gnat.subscribe("$SYS/broker/uptime", gnat::mosquitto_qos::qos_at_least_once);

  while (true) {
    gnat.process(std::chrono::milliseconds(100));
  }

  return 0;
}
