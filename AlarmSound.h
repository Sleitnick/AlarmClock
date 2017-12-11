/*
 * Author: Stephen Leitnick
 * Date: November 2017
 *
 * AlarmSound is used to inform the Alarm at what level
 * the buzzer should currently be set. It does this by
 * making use of any given AlarmSoundFragments.
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

