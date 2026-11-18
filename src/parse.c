#include <parse.h>

struct RequestHeader parseHeader(char* buf) {
	struct RequestHeader head = (struct RequestHeader){NULL, NULL, NULL, NULL, 0, NULL, (struct RequestPayload[REQUEST_SIZE]){}};
	struct RequestHeader* header = &head;
	int ind = 0;
	char *post_body = NULL;
	char *_ptr = NULL;

	for (char *tok = strtok(buf, " "); tok != NULL; tok = strtok(NULL, "\r\n")) {
		if (ind < 1) {
			header->method = tok;
			header->dest = strtok(NULL, " ");
			ind++;
		}

		// parse HTTP req Content-Length
		if ((_ptr = strstr(tok, "Content-Length:")) != NULL) {
			// tok = strtok(NULL, "\r\n");
			tok = _ptr + strlen("Content-Length: ");
			// convert char* to int only if valid
			char *endptr;
			errno = 0;
			long val = strtol(tok, &endptr, 10);
			if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (val > INT_MAX || val < INT_MIN)) {
				perror("Error Fetching Content-Length: Buffer Overflow");
			} else if (endptr == tok) {
				perror("Error: No Content-Length");
			} else {
				header->conLen = (int)val;
			}
		}

		// parse HTTP req Content-Type
		if ((_ptr = strstr(tok, "Content-Type:")) != NULL) {
			header->conType = _ptr + strlen("Content-Type: ");
		}

		// parse HTTP req POST body
		if ((_ptr = strstr(tok, "name"))!= NULL) {
			post_body = tok; 
			parseRequest(header->reqPL, tok);
		}

	}


	printf("\r\n\r\n--- method: %s --- dir: %s ---\r\n\r\n", header->method, header->dest);

	printf("--- Content-Length: %d ---\r\n\r\n", header->conLen);
	printf("--- Content-Type: %s ---\r\n\r\n", header->conType);
	printf("--- POST Body: %s ---\r\n\r\n", post_body);

	return head;
}


void parseRequest(struct RequestPayload* reqPL, char* req) {
	char *tok;
	for (tok = strtok(req, "="); tok != NULL; tok = strtok(NULL, "=")) {
		printf("query: %s\n", tok);
		if (tok != NULL) {
			// reqPL->field = tok;
			strncpy(reqPL->field, tok, REQUEST_SIZE);
			if ((tok = strtok(NULL, "&")) != NULL) {
				// reqPL->value = tok;
				strncpy(reqPL->value, tok, REQUEST_SIZE);
			} else if ((tok = strtok(NULL, "\r\n")) != NULL) {
				// reqPL->value = tok;
				strncpy(reqPL->value, tok, REQUEST_SIZE);
			}
		}
		reqPL++;
	}
}


