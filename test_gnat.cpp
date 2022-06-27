#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <iostream>
#include <thread>
#include "doctest/doctest/doctest.h"
#include "gnat.hpp"

static const std::string TEST_INSTANCE_HOST = "test.mosquitto.org";

TEST_CASE("Gnat: connect to test instance"){
  gnat::gnat client("mosquitto - gnat");
  auto res = client.init();
  CHECK_FALSE(res);
  res = client.connect(TEST_INSTANCE_HOST, 1883, std::chrono::milliseconds(1000));
  CHECK_FALSE(res);
  client.disconnect();
}

TEST_CASE("Gnat: connect to test instance, get subscription count"){
  gnat::gnat client("mosquitto - gnat");
  auto res = client.init();
  CHECK_FALSE(res);
  res = client.connect(TEST_INSTANCE_HOST, 1883, std::chrono::milliseconds(1000));
  CHECK_FALSE(res);
  uint32_t sub_count = 0;
  client.set_on_message_cb([&](int32_t mid, const std::string& topic, std::vector<uint8_t> payload) {
    std::string count_string(payload.begin(), payload.end());
    sub_count = std::stol(count_string);
  });
  res = client.subscribe("$SYS/broker/subscriptions/count", gnat::mosquitto_qos_t::qos_at_most_once);
  while (!sub_count) {
    client.process(std::chrono::milliseconds(100));
  }
  CHECK_GT(sub_count, 0);
  client.disconnect();
}

TEST_CASE("Gnat: connect to test instance, publish and subscribe to string"){
  gnat::gnat client("mosquitto - gnat");
  auto res = client.init();
  CHECK_FALSE(res);
  res = client.connect(TEST_INSTANCE_HOST, 1883, std::chrono::milliseconds(1000));
  CHECK_FALSE(res);
  const std::string TEST_PATTERN = "test_string";
  std::string test_string;
  client.set_on_message_cb([&](int32_t mid, const std::string& topic, std::vector<uint8_t> payload) {
    test_string = std::string(payload.begin(), payload.end());
  });
  res = client.subscribe("gnat/test/testtopic", gnat::mosquitto_qos_t::qos_at_most_once);
  res = client.publish("gnat/test/testtopic", TEST_PATTERN, gnat::mosquitto_qos_t::qos_at_most_once);
  while (test_string.empty()) {
    client.process(std::chrono::milliseconds(100));
  }
  CHECK_EQ(test_string, TEST_PATTERN);
  client.disconnect();
}

