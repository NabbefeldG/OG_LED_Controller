#include "Arduino.h"
#include "AnalogRamp.h"


AnalogRamp::AnalogRamp(int PIN_IN, int PIN_OUT, int PIN_GATE) {
  _PIN_IN = PIN_IN;
  _PIN_OUT = PIN_OUT;
  _PIN_GATE = PIN_GATE;

  analogWriteResolution(12);
  pinMode(_PIN_IN, INPUT);
  pinMode(_PIN_GATE, OUTPUT);
}

void AnalogRamp::update() {
  if (!_CW_mode) {
    if (_cycle_phase == 0) {
      if (digitalReadFast(_PIN_IN)) {
        // the first iteration with the input line one. Switch the line on and now wait for the input line to switch off again.
        // _update_intensity((int) (4095 * intensity));
        _cycle_phase = 1;
        _clock = millis();  // keep track of the start of the current pules to handle the up-ramp according to this clock
      }
    } else if (_cycle_phase == 1) {
      unsigned long dt = (millis() - _clock);  // I store this in an additional variable so the signal can't overshoot
      if (dt < rise_duration) {
        _update_intensity((int) (4095*intensity) * ((float) dt / rise_duration));
      } else {
        _update_intensity((int) (4095*intensity));
        _cycle_phase = 2;
      }
    } else if (_cycle_phase == 2) {
      if (not digitalReadFast(_PIN_IN)) {
        // this is the input line switch off, that starts the down ramp
        _cycle_phase = 3;
        _clock = millis();  // keep track of the end of the current pules to handle the down-ramp according to this clock
      }
    } else if (_cycle_phase == 3) {
      if (millis() < _clock + fall_duration) {
        // still ramping down
        _update_intensity((int) (4095*intensity) - (int) (4095*intensity * ((float) (millis() - _clock)) / fall_duration));
      } else {
        _cycle_phase = 0;
        _update_intensity(0);
      }
    }
  } else {
    _update_intensity((int) (4095 * intensity));
  }
}

void AnalogRamp::switchCWMode(bool new_mode) {
  _CW_mode = new_mode;
  if (_CW_mode) {
    _update_intensity((int) (4095 * intensity));
  } else {
    _update_intensity(0);
  }
}

void AnalogRamp::_update_intensity(float new_value) {
  if (_current_intensity != new_value) {
    _current_intensity = new_value;
    analogWrite(_PIN_OUT, _current_intensity);
  }
  if (_current_intensity > 0) {
    digitalWriteFast(_PIN_GATE, 1);
  } else {
    digitalWriteFast(_PIN_GATE, 0);
  }
}
