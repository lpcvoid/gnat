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

class gnat {

 private:
  static constexpr std::chrono::seconds DEFAULT_MQTT_KEEPALIVE = 60s;
  static bool MOSQUITTO_INITIALIZED;

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

 protected:
  std::string _client_id;
  mosquitto* _mosquitto_obj{};
 public:

  explicit gnat(const std::string& client_id) {
    if ((!MOSQUITTO_INITIALIZED) && (mosquitto_lib_init() == MOSQ_ERR_SUCCESS)) {
      MOSQUITTO_INITIALIZED = true;
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
    int result =  mosquitto_connect(_mosquitto_obj, host.c_str(), port, DEFAULT_MQTT_KEEPALIVE.count());
    if (result == MOSQ_ERR_ERRNO) {
      return get_last_error();
    } else if (result == MOSQ_ERR_INVAL) {
      return std::errc::invalid_argument;
    }
    return {};
  }

};

}