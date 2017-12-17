#ifndef SIMPLE_SERVER_H
#define SIMPLE_SERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HttpRequest.h"
#include "HttpResponse.h"


class SimpleServer {
public:
	WiFiServer server;
	SimpleServer::SimpleServer(int port);
	void handle();
	void SimpleServer::get(String path, void (&f)(HttpRequest req, HttpResponse res));
};


#endif



app.on((req, res, next) -> {

});
