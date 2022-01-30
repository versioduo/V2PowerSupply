// © Kay Sievers <kay@versioduo.com>, 2020-2022
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <Arduino.h>

class V2PowerSupply {
public:
  const struct Config {
    float min;
    float max;
  } config{};

  constexpr V2PowerSupply(const Config &conf, uint8_t pin) : config(conf), _pin(pin) {}

  // Switch power on
  bool on();

  // Return if we lost our power connection, so the hardware can be reinitialized.
  bool on(bool &continuous);

  void off();
  void loop();
  float getVoltage() {
    return _voltage;
  };

  // The number of power interruptions.
  uint32_t getInterruptions() {
    return _n_disconnects;
  };

protected:
  virtual void handleOn(){};
  virtual void handleOff(){};
  virtual void handleNotify(float voltage){};

private:
  // The analog pin to measure the voltage.
  uint8_t _pin;

  // The last time 'loop()' did its work.
  unsigned long _usec{};

  // The state of the monitored voltage.
  enum class State { Off, Settle, On } _state{};

  // The start of the period to detect a valid voltage range.
  unsigned long _settle_usec{};

  // The currently measured voltage.
  float _voltage{};

  // The last reported voltage, significant changes call 'handleNotify()'.
  float _on_voltage{};

  // There was no power interruption since the last call to 'on()'.
  bool _continuous{};

  // The number of power interruptions.
  uint32_t _n_disconnects{};
};
