#include "HttpRequest.h"

HttpRequest::HttpRequest(WiFiClient client) {

	int lineNum = 0;

	headers = headers_t();
	bool isBody = false;

	body = "";

	while (client.available()) {

		String line = client.readStringUntil('\r');
		line.trim();

		if (lineNum == 0) {

			// Parse method & path:
			int indexOfSpace = line.indexOf(' ');
			String remainingLine = line.substring(indexOfSpace + 1);
			method = line.substring(0, indexOfSpace);
			path = remainingLine.substring(0, remainingLine.indexOf(' '));
			Serial.println("METHOD: " + method);
			Serial.println("PATH: " + path);

		} else {

			if (isBody) {
				// Concatenate body:
				body += line + "\n";
			} else {
				if (line.equals("")) {
					isBody = true;
				} else {
					// Parse headers:
					int indexOfColon = line.indexOf(':');
					String headerKey = line.substring(0, indexOfColon);
					String headerVal = line.substring(indexOfColon + 1);
					headerKey.toLowerCase();
					headerVal.trim();
					headers[headerKey] = headerVal;
					Serial.println("HEADER " + headerKey + " = " + headerVal);
				}
			}

		}

		lineNum++;
	}

	Serial.println("BODY:\n" + body);

	client.flush();

}


String HttpRequest::getHeaderValue(String header) {
	String h = header;
	h.toLowerCase();
	return headers[h];
}