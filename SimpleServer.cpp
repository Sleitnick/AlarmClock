#include "SimpleServer.h"


SimpleServer::SimpleServer(uint16_t port) : server(WiFiServer(port)) {
	
}

void SimpleServer::begin() {
	server.begin();
}

void SimpleServer::handle() {
	WiFiClient client = server.available();
	if (!client.available()) return;
	HttpRequest req(client);
	HttpResponse res();
}
