/*
 * Author: Stephen Leitnick
 * Date: November 2017
 *
 * An Alarm object manages a single alarm at
 * a given time. It informs the main program
 * if the alarm should be buzzing, and can
 * be told to be silenced or disabled. The
 * alarm time is persistent using EEPROM.
 */

#ifndef ALARM_H
#define ALARM_H

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

#endif
