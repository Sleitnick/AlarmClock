/*
 * Author: Stephen Leitnick
 * Date: November 2017
 */
 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <aREST.h>
#include "Alarm.h"
#include "Network.h"
// Network.h simply defines NETWORK_SSID and NETWORK_PSK

#define SERVER_PORT 80

Adafruit_7segment matrix = Adafruit_7segment();
aREST rest = aREST();

WiFiServer server(SERVER_PORT);

const char* timeHost = "time.nist.gov";

uint16_t hour = 0;
uint16_t minute = 0;
int dst = 0;
int timezone = -5;

int matrixBrightness = 15;
String matrixBrightnessStr = String(matrixBrightness);

const unsigned long COLON_BLINK_INTERVAL_MILLIS = 500L;

const int buzzerPin = 15;
const int buttonPin = 13;

Alarm alarms[] = {
//Alarm(memoryLocationStart)
  Alarm(0),
  Alarm(4),
  Alarm(8)
};

String alarmsStr;
const int NUM_ALARMS = sizeof(alarms) / sizeof(alarms[0]);

typedef struct HttpAlarmParams {
  Alarm *alarm;
  int value;
  bool failed;
};

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  setupMatrixDisplay();
  setupRest();
  setupWiFi();
  setupServer();
  syncTime();
}

void setupMatrixDisplay() {
  matrix.begin(0x70);
  matrix.setBrightness(matrixBrightness);
  clearScreen();
}

void setupWiFi() {
  Serial.print("Connecting to LAN...");
  WiFi.begin(NETWORK_SSID, NETWORK_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
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
  rest.set_id("esp001");
  rest.set_name("EspAlarmClock");
  rest.function("setbrightness", httpSetBrightness);
  rest.function("alarmhour", httpSetAlarmHour);
  rest.function("alarmminute", httpSetAlarmMinute);
  rest.function("alarmenabled", httpSetAlarmEnabled);
  rest.function("alarmset", httpSetAlarm);
  rest.variable("brightness", &matrixBrightnessStr);
  setAlarmsRestVariable();
}

HttpAlarmParams getHttpAlarmParams(String params) {
  // alarmIndex-value
  // e.g.: 1-10
  HttpAlarmParams alarmParams;
  int dashIndex = params.indexOf("-");
  if (dashIndex == -1) {
    Serial.println("getHttpAlarmParams FAILED: Did not find dash in params string");
    alarmParams.failed = true;
    return alarmParams;
  }
  String alarmIndexStr = params.substring(0, dashIndex);
  Serial.print("alarmIndexStr = ");
  Serial.println(alarmIndexStr);
  int value = params.substring(dashIndex + 1).toInt();
  int alarmIndex = alarmIndexStr.toInt();
  if ((alarmIndex == 0 && alarmIndexStr != "0") || (alarmIndex < 0) || (alarmIndex >= NUM_ALARMS)) {
    Serial.print("getHttpAlarmParams FAILED: Invalid alarm index -> ");
    Serial.println(alarmIndex);
    alarmParams.failed = true;
    return alarmParams;
  }
  alarmParams.value = value;
  alarmParams.alarm = &alarms[alarmIndex];
  return alarmParams;
}

int httpSetBrightness(String amount) {
  int b = amount.toInt();
  if (b == 0 && amount != "0") return -1;
  b = (b < 0 ? 0 : b > 15 ? 15 : b);
  matrix.setBrightness(b);
  matrixBrightness = b;
  matrixBrightnessStr = String(b);
  rest.variable("brightness", &matrixBrightnessStr);
  return b;
}

//   /alarmhour?params=1-15  sets alarm at index '1' to 3pm
int httpSetAlarmHour(String params) {
  Serial.print("HTTP SetAlarmHour: ");
  Serial.println(params);
  HttpAlarmParams alarmParams = getHttpAlarmParams(params);
  if (alarmParams.failed) {
    Serial.println("HTTP SetAlarmHour failed");
    return -1;
  }
  int hour = alarmParams.value;
  hour = (hour < 0 ? 0 : hour > 23 ? 23 : hour);
  alarmParams.alarm->setHour(hour);
  alarmParams.alarm->save();
  setAlarmsRestVariable();
  return 0;
}

//   /alarmminute?params=0-35  sets alarm at index '0' to 35 minutes
int httpSetAlarmMinute(String params) {
  Serial.print("HTTP SetAlarmMinute: ");
  Serial.println(params);
  HttpAlarmParams alarmParams = getHttpAlarmParams(params);
  if (alarmParams.failed) {
    return -1;
  }
  int minute = alarmParams.value;
  minute = (minute < 0 ? 0 : minute > 59 ? 59 : minute);
  alarmParams.alarm->setMinute(minute);
  alarmParams.alarm->save();
  setAlarmsRestVariable();
  return 0;
}

//   /alarmenabled?params=0-1  sets alarm at index '0' to ENABLED (0=disabled, 1=enabled)
int httpSetAlarmEnabled(String params) {
  Serial.print("HTTP SetAlarmEnabled: ");
  Serial.println(params);
  HttpAlarmParams alarmParams = getHttpAlarmParams(params);
  if (alarmParams.failed) {
    return -1;
  }
  bool enabled = (alarmParams.value == 1);
  alarmParams.alarm->setEnabled(enabled);
  alarmParams.alarm->save();
  setAlarmsRestVariable();
  return 0;
}

byte* getSeparatedList(char sep, String str) {
  int numItems = 1;
  int lastIndex = 0;
  int index;
  while ((index = str.indexOf(sep, lastIndex + 1)) != -1) {
    numItems++;
    lastIndex = index;
  }
  byte items[numItems];
  lastIndex = -1;
  index = 0;
  int i = 0;
  while ((index = str.indexOf(sep, lastIndex + 1)) != -1) {
    Serial.println(" > " + str.substring(lastIndex + 1, index));
    items[i] = str.substring(lastIndex + 1, index).toInt();
    lastIndex = index;
    i++;
  }
  items[i] = str.substring(lastIndex + 1).toInt();
  return items;
}

void printValue(String label, int value) {
  Serial.print(label);
  Serial.println(value);
}

//   /alarmset?params=0-15-35-1   sets alarm 0 to 15 hours, 35 minutes, and enabled
int httpSetAlarm(String params) {
  // alarm-hour-minute-enabled
  // 0-15-35-1
  Serial.print("HTTP SetAlarm: ");
  Serial.println(params);
  byte *items = getSeparatedList('-', params);
  if (sizeof(items) != 4) {
    printValue("Incorrect number of items: ", sizeof(items));
    return -1;
  }
  Serial.println("Getting items");
  int alarmIndex = items[0];
  int hour = items[1];
  int minute = items[2];
  int enabled = items[3];
  printValue("alarmIndex: ", alarmIndex);
  printValue("hour: ", hour);
  printValue("minute: ", minute);
  printValue("enabled: ", enabled);
  if (alarmIndex < 0 || alarmIndex >= NUM_ALARMS) {
    printValue("Invalid alarmIndex: ", alarmIndex);
    return -1;
  }
  Serial.println("Transforming data");
  hour = (hour < 0 ? 0 : hour > 23 ? 23 : hour);
  minute = (minute < 0 ? 0 : minute > 59 ? 59 : minute);
  Alarm *alarm = &alarms[alarmIndex];
  alarm->setHour(hour);
  alarm->setMinute(minute);
  alarm->setEnabled(enabled == 1);
  alarm->save();
  setAlarmsRestVariable();
  return 0;
}

void setBuzzer(int value) {
  analogWrite(buzzerPin, value);
}

void setAlarmsRestVariable() {
  alarmsStr = "";
  for (int i = 0; i < NUM_ALARMS; i++) {
    Alarm *alarm = &alarms[i];
    alarmsStr += "alarm" + String(i) + ":" + String(alarm->getHour()) + "," + String(alarm->getMinute()) + "," + (alarm->isEnabled() ? "Y" : "N");
    if (i != (NUM_ALARMS - 1)) {
      alarmsStr += "|";
    }
  }
  Serial.println(alarmsStr);
  rest.variable("alarms", &alarmsStr);
}

void syncTime() {
  Serial.println("Syncing time...");

  configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for time...");
  while (!time(nullptr)) {
    delay(10);
  }

  updateTime();

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

void updateTime() {
  uint16_t lastHour = hour;
  uint16_t lastMin = minute;
  struct tm *timeinfo = getTimeInfo();
  hour = timeinfo->tm_hour;
  minute = timeinfo->tm_min;
  if (lastHour != hour || lastMin != minute) {
    writeTime();
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
  WiFiClient client = server.available();
  if (!client || !client.available()) return;
  rest.handle(client);
}

bool btnDown = false;
void loop() {
  btnDown = (digitalRead(buttonPin) == HIGH);
  updateTime();
  bool buzzing = updateAlarms(btnDown);
  loopServer();
  delay(buzzing ? 10 : 100);
}
