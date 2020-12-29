#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string>

#include "glog/logging.h"

using arduino_short = int16_t;
using arduino_ushort = int16_t;

using arduino_int = int32_t;
using arduino_uint = uint32_t;

using arduino_long = int32_t;
using arduino_ulong = uint32_t;

enum ArduinoPinMode {
  INPUT_PULLUP,
  INPUT,
  OUTPUT,
};

namespace arduino_internals {

// TODO(james): Note that this "design" is upside down. It would be better to port glog to work on top of Arduino's
// Serial mechanism and use glog instead of Serial. But this approach accomplishes the main goal of getting debug info.
class SerialT final {
 public:
  void print(const std::string& msg) { LOG(INFO) << msg; }
  void println(const std::string& msg) { LOG(INFO) << msg << "\n"; }

  void print(uint8_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(uint8_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }
  void print(int8_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(int8_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }

  void print(uint16_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(uint16_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }
  void print(int16_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(int16_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }

  void print(uint32_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(uint32_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }
  void print(int32_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(int32_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }

  void print(uint64_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(uint64_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }
  void print(int64_t msg) { LOG(INFO) << std::to_string(msg); }
  void println(int64_t msg) { LOG(INFO) << std::to_string(msg) << "\n"; }
};

// Global state to support the random(), randomSeed(), etc.
struct RandomT final {
  std::mt19937 randomNumberGenerator{};
};
extern RandomT RandomGlobals;

class Pin {
 public:
  virtual ~Pin() = default;

  ArduinoPinMode mode;

  // This is just the last value written to the pin. Other behaviors are possible by overriding the value() method
  // below.
  arduino_int last_value;

  virtual arduino_int value() const { return last_value; }
};

template <arduino_int NUM_PINS>
class Pins final {
 private:
  std::array<std::unique_ptr<Pin>, NUM_PINS> pins_{};

 public:
  Pins() {
    for (arduino_int i = 0; i < NUM_PINS; ++i) {
      pins_[i] = std::make_unique<Pin>();
    }
  }

  void put(arduino_int pin_id, std::unique_ptr<Pin> pin) { pins_[pin_id] = std::move(pin); }
  Pin& get(arduino_int pin_id) { return *pins_[pin_id]; }
};

// Teensy 4.1 has 42 pins. This is our default for now.
// TODO(james): Add structure for supporting multiple boards, MCUs, and configurations.
extern Pins<42> pins;

extern std::chrono::time_point<std::chrono::system_clock> start_time;

}  // namespace arduino_internals

extern arduino_internals::SerialT Serial;

void randomSeed(arduino_ulong seed) { arduino_internals::RandomGlobals.randomNumberGenerator.seed(seed); }

arduino_long random(arduino_long minimum, arduino_long maximum) {
  arduino_long low = std::max(minimum, static_cast<arduino_long>(std::numeric_limits<arduino_long>::min()));
  arduino_long high = std::min(maximum, static_cast<arduino_long>(std::numeric_limits<arduino_long>::max()));
  std::uniform_int_distribution<> distribution{low, high};
  return distribution(arduino_internals::RandomGlobals.randomNumberGenerator);
}

arduino_long random(arduino_long maximum) { return random(0, maximum); }

template <typename T>
T constrain(T value, T minimum, T maximum) {
  return std::max(minimum, std::min(maximum, value));
}

void pinMode(arduino_int pin, ArduinoPinMode mode) { arduino_internals::pins.get(pin).mode = mode; }

void analogWrite(arduino_int pin, arduino_int intensity) {
  arduino_internals::pins.get(pin).last_value = constrain(intensity, 0, 255);
}
arduino_int analogRead(arduino_int pin) { return constrain(arduino_internals::pins.get(pin).last_value, 0, 1023); }

arduino_ulong millis();
arduino_ulong micros();
