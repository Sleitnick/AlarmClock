/*
 * Author: Stephen Leitnick
 * Date: November 2017
 */

#include <Arduino.h>
#include "AlarmSound.h"

class Alarm {
  private:
    int hour;
    int minute;
    bool enabled;
    byte memLoc;
    bool isBuzzing;
    bool isDismissed;
    void setBuzzer(int value);
    void startBuzzing();
    void stopBuzzing();
  public:
    Alarm(byte memLoc);
    int getHour();
    int getMinute();
    void setHour(int hour);
    void setMinute(int minute);
    void setEnabled(bool enabled);
    bool isEnabled();
    bool isCurrentlyBuzzing();
    bool update(struct tm* timeInfo, bool buttonDown);
    void save();
};

