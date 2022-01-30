// © Kay Sievers <kay@versioduo.com>, 2020-2022
// SPDX-License-Identifier: Apache-2.0

#include "V2PowerSupply.h"

bool V2PowerSupply::on() {
  if (_state == State::On) {
    handleOn();
    return true;
  }

  handleNotify(_voltage);
  return false;
}

bool V2PowerSupply::on(bool &continuous) {
  if (on()) {
    continuous  = _continuous;
    _continuous = true;
    return true;
  }

  return false;
}

void V2PowerSupply::off() {
  handleOff();
}

void V2PowerSupply::loop() {
  if ((unsigned long)(micros() - _usec) < 100 * 1000)
    return;

  _usec = micros();

  // A voltage 10/100k divider measures the main power
  const uint16_t analog = analogRead(_pin);
  const float fraction  = (float)analog / 1023.f;
  _voltage              = 36.f * fraction;

  switch (_state) {
    case State::Off:
      if (_voltage >= config.min && _voltage <= config.max) {
        _state       = State::Settle;
        _settle_usec = micros();
      }
      break;

    case State::Settle:
      if (_voltage < config.min || _voltage > config.max) {
        _state = State::Off;
        break;
      }

      if ((unsigned long)(micros() - _settle_usec) < 500 * 1000)
        break;

      _state = State::On;
      handleNotify(_voltage);
      break;

    case State::On:
      if (_voltage < config.min || _voltage > config.max) {
        _state = State::Off;
        handleOff();
        handleNotify(_voltage);
        _n_disconnects++;
        _continuous = false;
        break;
      }

      // Show splash when the voltage changes significantly from the last notification.
      const float change    = abs(_on_voltage - _voltage);
      const float tolerance = float(config.max - config.min) / 10.f;
      if (change > tolerance) {
        _on_voltage = _voltage;
        handleNotify(_voltage);
      }
      break;
  }
}
