/*
 * Author: Stephen Leitnick
 * Date: November 2017
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include "Alarm.h"
#include "Network.h"
// Network.h simply defines NETWORK_SSID and NETWORK_PSK

const uint16_t SERVER_PORT = 80;

Adafruit_7segment matrix = Adafruit_7segment();

ESP8266WebServer server(SERVER_PORT);

uint16_t hour = 0;
uint16_t minute = 0;
int dst = 0;
int timezone = -5;

bool isDisplayOn = true;

int matrixBrightness = 15;
String matrixBrightnessStr = String(matrixBrightness);

const unsigned long COLON_BLINK_INTERVAL_MILLIS = 500L;

const int buzzerPin = 15;
const int buttonPin = 13;

bool btnDown = false;

Alarm alarms[] = {
//Alarm(memoryLocationStart)
	Alarm(0),
	Alarm(4),
	Alarm(8)
};

const int NUM_ALARMS = sizeof(alarms) / sizeof(alarms[0]);

typedef struct HttpAlarmParams {
	Alarm *alarm;
	int value;
	bool failed;
};

void setup() {
	Serial.begin(115200);
	Serial.println("Setting up now");
	pinMode(buzzerPin, OUTPUT);
	pinMode(buttonPin, INPUT);
	setupMatrixDisplay();
	setupRest();
	setupWiFi();
	setupServer();
	syncTime();
}

void setupMatrixDisplay() {
	Serial.println("Setting up matrix display...");
	matrix.begin(0x70);
	matrix.setBrightness(matrixBrightness);
	clearScreen();
	Serial.println("Matrix display ready.");
}

void setupWiFi() {
	Serial.print("Connecting to LAN...");
	WiFi.begin(NETWORK_SSID, NETWORK_PSK);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(1000);
	}
	Serial.println("\nConnected to LAN.");
}

void setupServer() {
	Serial.println("Starting server...");
	server.begin();
	Serial.println("Server started");
	Serial.println(WiFi.localIP());
}

void setupRest() {
	Serial.println("Setting up REST server settings...");

	server.on("/", []() {
		server.send(200, "application/json", "{\"success\": true}");
	});

	server.on("/test", []() {
		String message = "TEST\n\n";
		message += "Arguments: ";
		message += server.args();
		message += "\n";
		for (uint8_t i = 0; i < server.args(); i++) {
			message += "  " + server.argName(i) + ": " + server.arg(i) + "\n";
		}
		server.send(200, "text/plain", message);
	});

	server.on("/brightness", []() {
		if (server.method() == HTTP_GET) {
			server.send(200, "application/json", "{\"brightness\": " + String(matrixBrightness) + "}");
		} else {
			String amount = server.arg("amount");
			int b = amount.toInt();
			if (b == 0 && amount != "0") {
				server.send(400, "text/plain", "Bad Request");
				return;
			}
			b = (b < 0 ? 0 : b > 15 ? 15 : b);
			matrixBrightness = b;
			matrix.setBrightness(b);
			server.send(200, "application/json", "{\"brightness\": " + String(b) + "}");
		}
	});

	server.on("/alarm/hour", []() {
		String alarmIndexStr = server.arg("alarm");
		int alarmIndex = alarmIndexStr.toInt();
		if ((alarmIndex == 0 && alarmIndexStr != "0") || (alarmIndex < 0) || (alarmIndex >= NUM_ALARMS)) {
			server.send(400, "text/plain", "Bad Request");
			return;
		}
		Alarm* alarm = &alarms[alarmIndex];
		if (server.method() == HTTP_GET) {
			int hour = alarm->getHour();
			server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"hour\": " + String(hour) + "}");
		} else {
			String hourStr = server.arg("hour");
			int hour = hourStr.toInt();
			if (hour == 0 && hourStr != "0") {
				server.send(400, "text/plain", "Bad Request");
			} else {
				hour = (hour < 0 ? 0 : hour > 23 ? 23 : hour);
				alarm->setHour(hour);
				alarm->save();
				server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"hour\": " + String(hour) + "}");
			}
		}
	});

	server.on("/alarm/minute", []() {
		String alarmIndexStr = server.arg("alarm");
		int alarmIndex = alarmIndexStr.toInt();
		if ((alarmIndex == 0 && alarmIndexStr != "0") || (alarmIndex < 0) || (alarmIndex >= NUM_ALARMS)) {
			server.send(400, "text/plain", "Bad Request");
			return;
		}
		Alarm* alarm = &alarms[alarmIndex];
		if (server.method() == HTTP_GET) {
			int minute = alarm->getMinute();
			server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"minute\": " + String(minute) + "}");
		} else {
			String minuteStr = server.arg("minute");
			int minute = minuteStr.toInt();
			if (minute == 0 && minuteStr != "0") {
				server.send(400, "text/plain", "Bad Request");
			} else {
				minute = (minute < 0 ? 0 : minute > 59 ? 59 : minute);
				alarm->setMinute(minute);
				alarm->save();
				server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"minute\": " + String(minute) + "}");
			}
		}
	});

	server.on("/alarm/enabled", []() {
		String alarmIndexStr = server.arg("alarm");
		int alarmIndex = alarmIndexStr.toInt();
		if ((alarmIndex == 0 && alarmIndexStr != "0") || (alarmIndex < 0) || (alarmIndex >= NUM_ALARMS)) {
			server.send(400, "text/plain", "Bad Request");
			return;
		}
		Alarm* alarm = &alarms[alarmIndex];
		if (server.method() == HTTP_GET) {
			bool enabled = alarm->isEnabled();
			String enabledStr = (enabled ? "true" : "false");
			server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"enabled\": " + enabledStr + "}");
		} else {
			String enabledStr = server.arg("enabled");
			bool valid = (enabledStr == "true" || enabledStr == "false" || enabledStr == "0" || enabledStr == "1");
			bool enabled = (enabledStr == "true" || enabledStr == "1");
			if (!valid) {
				server.send(400, "text/plain", "Bad Request");
			} else {
				alarm->setEnabled(enabled);
				alarm->save();
				enabledStr = (enabled ? "true" : "false");
				server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"enabled\": " + enabledStr + "}");
			}
		}
	});

	server.on("/alarm", []() {
		String alarmIndexStr = server.arg("alarm");
		int alarmIndex = alarmIndexStr.toInt();
		if ((alarmIndex == 0 && alarmIndexStr != "0") || (alarmIndex < 0) || (alarmIndex >= NUM_ALARMS)) {
			server.send(400, "text/plain", "Bad Request");
			return;
		}
		Alarm* alarm = &alarms[alarmIndex];
		if (server.method() == HTTP_GET) {
			int hour = alarm->getHour();
			int minute = alarm->getMinute();
			bool enabled = alarm->isEnabled();
			String enabledStr = (enabled ? "true" : "false");
			server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"hour\": " + String(hour) + ", \"minute\": " + String(minute) + ", \"enabled\": " + enabledStr + "}");
		} else {
			String hourStr = server.arg("hour");
			int hour = hourStr.toInt();
			String minuteStr = server.arg("minute");
			int minute = minuteStr.toInt();
			String enabledStr = server.arg("enabled");
			bool enabledValid = (enabledStr == "true" || enabledStr == "false" || enabledStr == "0" || enabledStr == "1");
			bool enabled = (enabledStr == "true" || enabledStr == "1");
			if ((hour == 0 && hourStr != "0") || (minute == 0 && minuteStr != "0") || (!enabledValid)) {
				server.send(400, "text/plain", "Bad Request");
			} else {
				hour = (hour < 0 ? 0 : hour > 23 ? 23 : hour);
				minute = (minute < 0 ? 0 : minute > 59 ? 59 : minute);
				enabledStr = (enabled ? "true" : "false");
				alarm->setHour(hour);
				alarm->setMinute(minute);
				alarm->setEnabled(enabled);
				alarm->save();
				server.send(200, "application/json", "{\"alarm\": " + String(alarmIndex) + ", \"hour\": " + String(hour) + ", \"minute\": " + String(minute) + ", \"enabled\": " + enabledStr + "}");
			}
		}
	});

	server.on("/alarms", []() {
		if (server.method() == HTTP_GET) {
			String message = "{\"alarms\": [";
			for (int i = 0; i < NUM_ALARMS; i++) {
				Alarm* alarm = &alarms[i];
				String hour = String(alarm->getHour());
				String min = String(alarm->getMinute());
				String enabled = (alarm->isEnabled() ? "true" : "false");
				String obj = "{\"alarm\":" + String(i) + ", \"hour\": " + hour + ", \"minute\": " + min + ", \"enabled\": " + enabled + "}";
				if (i > 0) message += ", ";
				message += obj;
			}
			message += "]}";
			server.send(200, "application/json", message);
		} else {
			server.send(400, "text/plain", "Bad Request");
		}
	});

	server.on("/alarmcount", []() {
		if (server.method() == HTTP_GET) {
			server.send(200, "application/json", "{\"count\": " + String(NUM_ALARMS) + "}");
		} else {
			server.send(400, "text/plain", "Bad Request");
		}
	});

	server.onNotFound([]() {
		server.send(404, "text/plain", "Resource Not Found");
	});

	Serial.println("REST server settings set.");
}

void setBuzzer(int value) {
	analogWrite(buzzerPin, value);
}

void syncTime() {
	Serial.println("Syncing time...");

	configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
	Serial.println("Waiting for time...");
	while (!time(nullptr)) {
		delay(10);
	}

	updateTime(true);

	Serial.println("Time synced.");
}

void clearScreen() {
	matrix.writeDigitRaw(0, 0x00);
	matrix.writeDigitRaw(1, 0x00);
	matrix.writeDigitRaw(3, 0x00);
	matrix.writeDigitRaw(4, 0x00);
	matrix.drawColon(false);
	matrix.writeDisplay();
}

struct tm* getTimeInfo() {
	time_t now;
	struct tm *timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return timeinfo;
}

void updateTime(bool forceWrite) {
	uint16_t lastHour = hour;
	uint16_t lastMin = minute;
	struct tm *timeinfo = getTimeInfo();
	hour = timeinfo->tm_hour;
	minute = timeinfo->tm_min;
	if (forceWrite || (lastHour != hour || lastMin != minute)) {
		if (isDisplayOn) {
			writeTime();
		} else {
			clearScreen();
		}
	}
}

bool updateAlarms(bool buttonDown) {
	struct tm* timeinfo = getTimeInfo();
	bool buzzing = false;
	for (int i = 0; i < NUM_ALARMS; i++) {
		Alarm *alarm = &alarms[i];
		bool b = alarm->update(timeinfo, buttonDown);
		if (b) buzzing = true;
	}
	return buzzing;
}

void writeTime() {
	uint16_t _hour = hour;
	if (_hour > 12) {
		_hour -= 12;
	}
	if (_hour == 0) {
		_hour = 12;
	}
	if (_hour < 10) {
		matrix.writeDigitRaw(0, 0x00);
		matrix.writeDigitNum(1, _hour);
	} else {
		matrix.writeDigitNum(0, _hour / 10);
		matrix.writeDigitNum(1, _hour % 10);
	}
	if (minute < 10) {
		matrix.writeDigitNum(3, 0);
		matrix.writeDigitNum(4, minute);
	} else {
		matrix.writeDigitNum(3, minute / 10);
		matrix.writeDigitNum(4, minute % 10);
	}
	matrix.drawColon(true);
	matrix.writeDisplay();
}

void loopServer() {
	server.handleClient();
}

void loop() {
	bool wasDown = btnDown;
	btnDown = (digitalRead(buttonPin) == HIGH);
	bool buzzing = updateAlarms(btnDown);
	if (!buzzing && btnDown && !wasDown) {
		isDisplayOn = !isDisplayOn;
		updateTime(true);
	} else {
		if (buzzing && !isDisplayOn) {
			isDisplayOn = true;
			updateTime(true);
		} else {
			updateTime(false);
		}
	}
	loopServer();
	delay(buzzing ? 10 : 100);
}
