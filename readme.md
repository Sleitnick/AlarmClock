# Alarm Clock
This is the code to an alarm clock that I built with the ESP8266 microcontroller.

--------------

## Network

Does not include the necessary `Network.h` header file. That file should look like the following:

```c
#define NETWORK_SSID "ssid"
#define NETWORK_PSK "psk"
```

--------------

## Endpoints

### `GET /`
Returns a simple 'success' message for testing purposes. Expected response:
```json
{
   "success": true
}
```

### `GET /brightness`
Gets the current display brightness. Response example:
```json
{
   "brightness": 0
}
```


### `POST /brightness`
Sets the current display brightness. Request example:
```json
{
   "amount": 0
}
```
Amount can be between 0 and 15.

### `GET /alarms`
Gets a list of all current alarms. Response example:
```json
{
   "alarms": [
      {
         "index": 0,
         "hour": 0,
         "minute": 0,
         "enabled": false
      }
   ]
}
```

### `GET /alarmcount`
Gets the number of available alarms. Response example:
```json
{
   "count": 3
}
```

### `GET /alarm`
Gets the current properties of an alarm.
Parameters:

| Parameter | Value |
| --------- | ----- |
| alarm     | 0     |

Example request:
`/alarm?alarm=1`

Example response:
```json
{
   "alarm": 1,
   "hour": 0,
   "minute": 0,
   "enabled": false
}
```

### `POST /alarm`
Sets the current properties of an alarm.
URL-Encoded parameters:

| Parameter | Value |
| --------- | ----- |
| alarm     | 0     |
| hour      | 0     |
| minute    | 0     |
| enabled   | false |
