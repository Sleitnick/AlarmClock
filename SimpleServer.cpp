#include "SimpleServer.h"


SimpleServer::SimpleServer(int port) {
	server = WiFiServer(port);
}

void SimpleServer::handle() {
	WiFiClient = server.available();
	if (!client.available()) return;
	HttpRequest req(client);
	HttpResponse res();
}