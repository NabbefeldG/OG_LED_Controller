#ifndef AnalogRamp_h
#define AnalogRamp_h

#include "Arduino.h"

class AnalogRamp {
  public:
    AnalogRamp(int PIN_IN, int PIN_OUT, int PIN_GATE);  // constructor
    void update(); // realize the ramp
    void switchCWMode(bool new_mode);

    float intensity = 1;
    float rise_duration = 100; // duration for the ramp-up in ms
    float fall_duration = 500; // duration for the ramp-down in ms

  private:
    void _update_intensity(float new_value);
    
    int _PIN_IN;
    int _PIN_OUT;
    int _PIN_GATE;

    // state vars
    int _cycle_phase = 0;  // 0: off, 1:on, 2:ramping down
    bool _CW_mode = 0;

    float _current_intensity = 1;
//    float _temp_new_intensity = 1;

	  // Clocks
    unsigned long _clock = 0;
    unsigned long _Serial_clock = 0;
};

#endif
