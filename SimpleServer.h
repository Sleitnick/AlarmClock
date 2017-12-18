#ifndef SIMPLE_SERVER_H
#define SIMPLE_SERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <map>
#include "HttpRequest.h"
#include "HttpResponse.h"

#define METHOD_GET "GET"
#define METHOD_POST "POST"

typedef void (*callback_t)(HttpRequest *req, HttpResponse *res);
typedef std::map<String, callback_t> path_callbacks_t;

class SimpleServer {
public:
	WiFiServer server;
	path_callbacks_t gets;
	path_callbacks_t posts;
	SimpleServer(uint16_t port);
	void begin();
	void handle();
	void get(String path, callback_t callback);
	void post(String path, callback_t callback);
};


#endif
