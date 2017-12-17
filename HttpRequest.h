#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <map>

typedef std::map<String, String> headers_t;

class HttpRequest {
public:
	String method;
	String path;
	String body;
	HttpRequest(WiFiClient client);
	String getHeaderValue(String header);
private:
	headers_t headers;
};
