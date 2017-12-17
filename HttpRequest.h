#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <map>

typedef std::map<String, String> headers_t;

class HttpRequest {
public:
	String method;
	String path;
	String body;
	headers_t headers;
	HttpRequest(WiFiClient client);
	String getHeaderValue(String header);
};

#endif
