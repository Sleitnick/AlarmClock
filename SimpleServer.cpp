#include "SimpleServer.h"


SimpleServer::SimpleServer(uint16_t port) : server(WiFiServer(port)) {

}


void SimpleServer::begin() {
	server.begin();
}


void SimpleServer::handle() {

	WiFiClient client = server.available();
	if (!client.available()) return;

	HttpRequest *req = new HttpRequest(client);
	HttpResponse *res = new HttpResponse();

	path_callbacks_t* pathCallbacks = NULL;

	if (req->method.equals(METHOD_GET)) {
		pathCallbacks = &gets;
	} else if (req->method.equals(METHOD_POST)) {
		pathCallbacks = &posts;
	}

	if (pathCallbacks != NULL) {
		callback_t callback = (*pathCallbacks)[req->path];
		if (callback != NULL) {
			callback(req, res);
		}
	}

	// TODO: Send response

	client.flush();

}


void SimpleServer::get(String path, callback_t callback) {
	gets[path] = callback;
}


void SimpleServer::post(String path, callback_t callback) {
	posts[path] = callback;
}
