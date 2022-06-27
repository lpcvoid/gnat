//
// Created by lpcvoid on 25/06/2022.
//

#pragma once

#include "mosquitto.h"
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
namespace gnat {

using namespace std::chrono_literals;

using mosquitto_version_t = std::tuple<int32_t, int32_t, int32_t>;
using gnat_callback_on_message_t = std::function<void(int32_t, const std::string &, std::vector<uint8_t>)>;

enum class mosquitto_qos_t { qos_at_most_once = 0,
                             qos_at_least_once = 1,
                             qos_exactly_once = 2 };

class gnat {

 private:
  static constexpr std::chrono::seconds DEFAULT_MQTT_KEEPALIVE = 60s;

  inline static void on_mosq_connect(mosquitto *m, void *object, int return_code) {
    ((gnat *) object)->_connected = true;
  };

  inline static void on_mosq_disconnect(mosquitto *m, void *object, int return_code) {
    ((gnat *) object)->_connected = false;
  };

  inline static void on_mosq_message(mosquitto *m, void *object, const mosquitto_message *msg) {
    if (((gnat *) object)->_on_message_callback) {
      std::vector<uint8_t> data(msg->payloadlen);
      std::memcpy(data.data(), msg->payload, msg->payloadlen);
      ((gnat *) object)->_on_message_callback(msg->mid, msg->topic, data);
    }
  };

  inline static void on_mosq_subscribe(mosquitto *m, void *userdata, int message_id, int count_granted_subs, const int *granted_qos){

  };

  inline static void on_mosq_log_message(mosquitto *m, void *userdata, int log_level, const char *msg){
      //std::cout << "MOSQUITTO LOG: " << msg << std::endl;
  };

  static std::error_condition get_last_error() {
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
        return std::errc::protocol_error;//debatable
    }
  }

  gnat() = default;

 protected:
  std::atomic<bool> _connected = false;
  std::string _client_id;
  mosquitto *_mosquitto_obj{};
  gnat_callback_on_message_t _on_message_callback;

 public:
  explicit gnat(const std::string &client_id) {
    _client_id = client_id;
    static bool mosquitto_initialized = false;
    if ((!mosquitto_initialized) && (mosquitto_lib_init() == MOSQ_ERR_SUCCESS)) {
      mosquitto_initialized = true;
    }
  }

  inline static mosquitto_version_t get_mosquitto_version() {
    int major, minor, patch;
    mosquitto_lib_version(&major, &minor, &patch);
    return mosquitto_version_t{major, minor, patch};
  }

  inline bool is_valid() {
    return _mosquitto_obj;
  }

  inline bool is_connected() {
    return _connected;
  }

  inline std::error_condition init() {

    _mosquitto_obj = mosquitto_new(_client_id.c_str(), true, this);
    if (!_mosquitto_obj) {
      return get_last_error();
    }

    mosquitto_connect_callback_set(_mosquitto_obj, on_mosq_connect);
    mosquitto_disconnect_callback_set(_mosquitto_obj, on_mosq_disconnect);
    mosquitto_message_callback_set(_mosquitto_obj, on_mosq_message);
    mosquitto_subscribe_callback_set(_mosquitto_obj, on_mosq_subscribe);
    mosquitto_log_callback_set(_mosquitto_obj, on_mosq_log_message);

    return {};
  }

  inline void set_on_message_cb(gnat_callback_on_message_t cb) {
    _on_message_callback = std::move(cb);
  }

  inline bool set_credentials(const std::string &username, const std::string &password) {
    if (!is_valid()) {
      return false;
    }
    return mosquitto_username_pw_set(_mosquitto_obj, username.c_str(), password.c_str()) == MOSQ_ERR_SUCCESS;
  }

  inline void disconnect() {
    mosquitto_disconnect(_mosquitto_obj);
    mosquitto_destroy(_mosquitto_obj);
    _connected = false;
  }

  inline std::error_condition connect(const std::string &host, uint16_t port, std::chrono::milliseconds timeout) {

    if (!_mosquitto_obj) {
      return std::errc::operation_not_supported;
    }

    int res = mosquitto_connect(_mosquitto_obj,
                                host.c_str(),
                                port,
                                DEFAULT_MQTT_KEEPALIVE.count());
    if (res) {
      return convert_mosquitto_error(res);
    }
    //we need to wait for CONNACK
    auto time_start = std::chrono::steady_clock::now();
    while (true) {
      process(10ms);
      if (is_connected()) {
        return {};
      }
      auto time_now = std::chrono::steady_clock::now();
      std::chrono::milliseconds ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>((time_now - time_start));
      if (ms_taken > timeout) {
        disconnect();
        return std::errc::timed_out;
      }
    }
  }

  inline std::error_condition subscribe(const std::string &topic, mosquitto_qos_t qos) {
    int res = mosquitto_subscribe(_mosquitto_obj, nullptr, topic.c_str(), static_cast<int>(qos));
    return convert_mosquitto_error(res);
  }

  inline std::error_condition publish(const std::string &topic, const std::string& data, mosquitto_qos_t qos) {
    return publish(topic, std::vector<uint8_t>(data.begin(), data.end()), qos);
  }

  inline std::error_condition publish(const std::string &topic, std::vector<uint8_t> data, mosquitto_qos_t qos) {
    int res = mosquitto_publish(_mosquitto_obj,
                                nullptr,
                                topic.c_str(),
                                data.size(),
                                data.data(),
                                static_cast<int>(qos),
                                true);
    return convert_mosquitto_error(res);
  }

  inline std::error_condition process(std::chrono::milliseconds timeout) {
    return convert_mosquitto_error(mosquitto_loop(_mosquitto_obj, timeout.count(), 1));
  };
};

}// namespace gnat