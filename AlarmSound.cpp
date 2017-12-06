/*
 * Author: Stephen Leitnick
 * Date: November 2017
 */

#include "AlarmSound.h"

AlarmSound::AlarmSound(std::vector<AlarmSoundFragment> alarmSoundFragments) : fragments(alarmSoundFragments) {
  numFragments = fragments.size();
  curFragmentIndex = 0;
  lastFragmentStart = 0;
}

int AlarmSound::update() {
  long curTime = millis();
  int dt = (curTime - lastFragmentStart);
  AlarmSoundFragment *fragment = &fragments[curFragmentIndex];
  if (dt > fragment->duration) {
    curFragmentIndex++;
    if (curFragmentIndex >= numFragments) {
      curFragmentIndex = 0;
    }
    lastFragmentStart = curTime;
    fragment = &fragments[curFragmentIndex];
  }
  return fragment->value;
}

void AlarmSound::reset() {
  curFragmentIndex = 0;
  lastFragmentStart = millis();
}

