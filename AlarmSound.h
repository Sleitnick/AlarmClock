/*
 * Author: Stephen Leitnick
 * Date: November 2017
 */

#include <Arduino.h>
#include <vector>
#include "AlarmSoundFragment.h"

class AlarmSound {
  private:
    std::vector<AlarmSoundFragment> fragments;
    int numFragments;
    int curFragmentIndex;
    long lastFragmentStart;
  public:
    AlarmSound(std::vector<AlarmSoundFragment> alarmSoundFragments);
    int update();
    void reset();
};

