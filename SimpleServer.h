#ifndef SIMPLE_SERVER_H
#define SIMPLE_SERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HttpRequest.h"
#include "HttpResponse.h"


class SimpleServer {
public:
	WiFiServer server;
	SimpleServer(uint16_t port);
	void begin();
	void handle();
	void get(String path, void (&f)(HttpRequest req, HttpResponse res));
};


#endif
