#include <iostream>
#include "gnat.hpp"

int main() {
  gnat::gnat gnat("mosquitto, gnatt wrapper");
  gnat::mosquitto_version_t version = gnat::gnat::get_mosquitto_version();
  std::cout << "Gnat test - mosquitto version " <<
      std::get<0>(version) << "." <<
      std::get<1>(version) << "." <<
      std::get<2>(version) << std::endl;

  return 0;
}
