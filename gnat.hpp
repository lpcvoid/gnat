//
// Created by lpcvoid on 25/06/2022.
//

#pragma once

#include <chrono>
#include <tuple>
#include <string>
#include <memory>
#include "mosquitto.h"
namespace gnat {

using namespace std::chrono_literals;

using mosquitto_version_t = std::tuple<int32_t, int32_t , int32_t>;

enum class mosquitto_qos { qos_at_most_once = 0,
                           qos_at_least_once = 1,
                           qos_exactly_once = 2};

class gnat {

 private:
  static constexpr std::chrono::seconds DEFAULT_MQTT_KEEPALIVE = 60s;

  inline static void on_mosq_connect(mosquitto* m, void* object, int return_code) {

  };

  inline static void on_mosq_disconnect(mosquitto* m, void* object, int return_code) {

  };

  inline static void on_mosq_message(mosquitto* m, void* object, const mosquitto_message*) {

  };

  static std::error_condition get_last_error()
  {
    std::error_code ec;
#ifdef _WIN32
    ec = std::error_code(::GetLastError(), std::system_category());
#else
    ec = std::error_code(errno, std::system_category());
#endif
    return ec.default_error_condition();
  }

  static std::error_condition convert_mosquitto_error(int mosquitto_error) {
    switch (mosquitto_error) {
      case MOSQ_ERR_SUCCESS:
        return {};
      case MOSQ_ERR_INVAL:
        return std::errc::invalid_argument;
      case MOSQ_ERR_NOMEM:
        return std::errc::not_enough_memory;
      case MOSQ_ERR_NO_CONN:
        return std::errc::not_connected;
      case MOSQ_ERR_MALFORMED_UTF8:
        return std::errc::illegal_byte_sequence;
      case MOSQ_ERR_OVERSIZE_PACKET:
        return std::errc::value_too_large;
      case MOSQ_ERR_ERRNO:
        return get_last_error();
      default:
        return std::errc::protocol_error; //debatable
    }
  }

  gnat() = default;

 protected:
  std::string _client_id;
  mosquitto* _mosquitto_obj{};
 public:

  explicit gnat(const std::string& client_id) {
    static bool mosquitto_initialized = false;
    if ((!mosquitto_initialized) && (mosquitto_lib_init() == MOSQ_ERR_SUCCESS)) {
      mosquitto_initialized = true;
    }
  }

  inline bool is_valid() {
    return _mosquitto_obj;
  }

  inline static mosquitto_version_t get_mosquitto_version() {
    int major, minor, patch;
    mosquitto_lib_version(&major, &minor, &patch);
    return mosquitto_version_t{major, minor, patch};
  }

  inline bool set_credentials(const std::string& username, const std::string& password) {
    if (!is_valid()) {
      return false;
    }

    return mosquitto_username_pw_set(_mosquitto_obj, username.c_str(), password.c_str()) == MOSQ_ERR_SUCCESS;
  }

  inline std::error_condition connect(const std::string& client_name, const std::string& host, uint16_t port) {
    _mosquitto_obj = mosquitto_new(_client_id.c_str(), true, this);
    mosquitto_connect_callback_set(_mosquitto_obj, on_mosq_connect);
    mosquitto_disconnect_callback_set(_mosquitto_obj, on_mosq_disconnect);
    mosquitto_message_callback_set(_mosquitto_obj, on_mosq_message);
    return convert_mosquitto_error(mosquitto_connect(_mosquitto_obj,
                                                     host.c_str(),
                                                     port,
                                                     DEFAULT_MQTT_KEEPALIVE.count()));
  }

  inline std::error_condition subscribe(const std::string& topic, mosquitto_qos qos) {
    return convert_mosquitto_error(mosquitto_subscribe(_mosquitto_obj, nullptr, topic.c_str(), static_cast<int>(qos)));
  }

};

}