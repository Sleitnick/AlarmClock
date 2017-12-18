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

#include "Alarm.h"
#include <EEPROM.h>
#include <time.h>

bool eepromBegan = false;

const int BUZZER_PIN = 15;

// Sound pattern:
AlarmSoundFragment soundFragments[] = {
	AlarmSoundFragment(255, 50),
	AlarmSoundFragment(0, 50),
	AlarmSoundFragment(255, 50),
	AlarmSoundFragment(0, 50),
	AlarmSoundFragment(255, 50),
	AlarmSoundFragment(0, 50),
	AlarmSoundFragment(255, 50),
	AlarmSoundFragment(0, 300)
};

std::vector<AlarmSoundFragment> soundFragmentsVec(soundFragments, soundFragments + sizeof(soundFragments) / sizeof(soundFragments[0]));
AlarmSound *alarmSound = new AlarmSound(soundFragmentsVec);

void initEEPROM() {
	if (eepromBegan) return;
	eepromBegan = true;
	EEPROM.begin(512);
}

Alarm::Alarm(byte mem) : hour(0), minute(0), enabled(false), memLoc(mem) {
	initEEPROM();
	hour = EEPROM.read(memLoc);
	minute = EEPROM.read(memLoc + 1);
	enabled = EEPROM.read(memLoc + 2) == 0x1;
	if (hour > 23) hour = 0;
	if (minute > 59) minute = 0;
	isBuzzing = false;
	isDismissed = false;
}

int Alarm::getHour() {
	return hour;
}

int Alarm::getMinute() {
	return minute;
}

bool Alarm::isEnabled() {
	return enabled;
}

void Alarm::setHour(int h) {
	hour = h;
}

void Alarm::setMinute(int m) {
	minute = m;
}

void Alarm::setEnabled(bool e) {
	enabled = e;
}

void Alarm::setBuzzer(int value) {
	analogWrite(BUZZER_PIN, value);
}

void Alarm::startBuzzing() {
	alarmSound->reset();
	setBuzzer(255);
}

void Alarm::stopBuzzing() {
	setBuzzer(0);
}

bool Alarm::isCurrentlyBuzzing() {
	return isBuzzing;
}

bool Alarm::update(struct tm* timeInfo, bool buttonDown) {
	if (isBuzzing) {
		if (buttonDown) {
			isDismissed = true;
			isBuzzing = false;
			stopBuzzing();
		} else {
			int amount = alarmSound->update();
			setBuzzer(amount);
		}
	} else {
		int curHour = timeInfo->tm_hour;
		int curMin = timeInfo->tm_min;
		if (curHour == hour && curMin == minute) {
			if ((!isDismissed) && enabled) {
				isBuzzing = true;
				startBuzzing();
			}
		} else if (isDismissed) {
			isDismissed = false;
		}
	}
	return isBuzzing;
}

void Alarm::save() {
	EEPROM.write(memLoc, hour);
	EEPROM.write(memLoc + 1, minute);
	EEPROM.write(memLoc + 2, enabled ? 0x1 : 0x0);
	EEPROM.commit();
}
