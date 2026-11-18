#include <server.h>
#include <map.h>
#include <file.h>
#include <parse.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
	(void)sig;
	printf("\nKeyboard Interrupt. Exiting Program.\n");
	keep_running = 0;
}



char *makeHTTPHeader(char* status, char* type, long fs) {
	char *header = malloc(HEADER_SIZE);

	if (header == NULL) {
		perror("memory allocation failed for header");
		return NULL;
	}

	int header_len = snprintf(header, HEADER_SIZE,
			"HTTP/1.1 %s\r\n"
			"Server: WebServer/1.0\r\n"
			"Content-Type: %s; charset=UTF-8\r\n"
			"Content-Length: %ld\r\n"
			"Connection: close\r\n"
			"\r\n",
			status, type, fs
	);
	return header;
}



char** parseQuery(char* query) {
	char **_attributes = (char **)malloc(QUERY_SIZE);
	// parse =
	char *tok;
	tok = strtok(query, "?");
	printf("dir: %s\n", tok);
	_attributes[0] = tok;
	
	// parse &
	// tok = strtok(NULL, "=");
	// printf("query: %s\n", tok);
	int idx = 1;
	for (tok = strtok(NULL, "="); tok != NULL; tok = strtok(NULL, "=")) {
		printf("query: %s\n", tok);
		if (tok != NULL) {
			_attributes[idx] = tok;
			idx++;
			if ((tok = strtok(NULL, "&")) != NULL) {
				_attributes[idx] = tok;
				idx++;
			} else if ((tok = strtok(NULL, "\r\n")) != NULL) {
				_attributes[idx] = tok;
				idx++;
			}
		}
	}

	return _attributes;
}




int main(int argc, char const* argv[])
{
	// if (argc < 2) {
	// 	perror("need file arg");
	// 	exit(EXIT_FAILURE);
	// }
	struct sigaction sa;
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);


	int server_fd, conn_socket;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	char buf[1024] = { 0 };

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Attaching socket to port 8000
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (keep_running) {
		if ((conn_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
			if (errno == EINTR) {
				break;
			}
			perror("accept");
	    	}
	  
		read(conn_socket, buf, 1023);

		char *ptr = buf;
		while (*ptr) {
			printf("%c", *ptr);
			ptr++;
		}

		struct RequestHeader *reqHeader;
		char tempbuf[1024] = { 0 };
		memcpy(tempbuf, buf, 1024);
		*reqHeader = parseHeader(tempbuf);

		if (reqHeader->method == NULL || reqHeader->dest == NULL) {
			fprintf(stderr, "expected method and destination\n");
			close(conn_socket);
		}


		if(reqHeader->method != NULL && strncmp(reqHeader->method, "GET", strlen("GET")) == 0) {
			long file_size = 0;
			char *html_content = NULL;
			char* type = (char *)malloc(sizeof(char) * SOTYPE);
			char* header;
			if (strlen(reqHeader->dest) == 1) {
				html_content = readFile(reqHeader->dest, &file_size, &type);
			}

			int isCont = 1;
			// TODO: get any file type specified (i.e. JS, PHP, PDF, etc)
			if (strlen(reqHeader->dest) > 1){
				if (strncmp(reqHeader->dest + 1, "favicon.ico", strlen(reqHeader->dest+1)) == 0) {	// TODO: parse favicon request.
					isCont = 0;
				} else if (strchr(reqHeader->dest + 1, '?') != NULL) {	// TODO: parse query with attributes
					//isCont = 0;
					char **_attributes;
					_attributes = parseQuery(reqHeader->dest);
					if (_attributes != NULL) {	// If query contains attributes
						if (*_attributes != NULL) {	// are there attributes?
							if (strlen(*_attributes) == 1) {	// if no dest given
								html_content = readFile(*_attributes, &file_size, &type);
							} else html_content = readFile(*_attributes+1, &file_size, &type);	// else try dest
							printf("destination: %s\n", *_attributes + 1);
						} else isCont = 0;	// no contents specified
						// while (*_attributes != NULL) {
						// 	printf("attributes: %s\n", *_attributes++);
						// }
						free(_attributes);
					} else {	// no attributes
						printf("attributes null\n");
						isCont = 0;
					}
				} else {	// try header dest field
					html_content = readFile(reqHeader->dest + 1, &file_size, &type);	
				}
			}

			if (html_content == NULL && isCont) {
				char *not_found = makeHTTPHeader("404 Not Found", "text/html", 9);
				send(conn_socket, not_found, strlen(not_found), 0);
				free(not_found);
				perror("cannot read file");
				exit(EXIT_FAILURE);
			}


			// Send HTML file content
			if (isCont) {
				header = makeHTTPHeader("200 OK", type, file_size);

				// Send HTTP headers
				send(conn_socket, header, strlen(header), 0);
				free(header);
				send(conn_socket, html_content, file_size, 0);
			} else {
				printf("Favicon Req\n");
				char *not_found = makeHTTPHeader("404 Not Found", "text/html", 9);
				send(conn_socket, not_found, strlen(not_found), 0);
				free(not_found);
				//send(conn_socket, html_content, file_size, 0);
			}

			free(html_content);
			free(type);
		} else if(strncmp(reqHeader->method, "POST", strlen(reqHeader->method)) == 0) {

			// readPostBody(conn_socket, content_len);
			// printf("Done Reading Body\r\n\r\n");

			char *resp = (char *)malloc(sizeof(char) * 1024);
			snprintf(resp, sizeof(char)*1024, "<!DOCTYPE html><html><h1>Welcome %s!</h1></html>", reqHeader->reqPL->value);
			char *header = makeHTTPHeader("200 OK", "text/html", strlen(resp));

			while (strlen(reqHeader->reqPL->field) > 0) {
				printf("\nfield: %s -- value: %s\n", reqHeader->reqPL->field, reqHeader->reqPL->value);
				reqHeader->reqPL++;
			}

			//TODO: Need a way to update server resources from
			// 	valid POST requests.
			//
			// if (strlen(reqHeader->dest[1]) > 1) {
			// 	struct dirent *de;
			// 	DIR *dr = opendir(".");
			// 	if (dr == NULL) {
			// 		perror("could not open dir");
			// 	} else {
			// 		while ((de = readdir(dr)) != NULL) {
			// 			if (strcmp(de->d_name, reqHeader->dest[1] + 1) == 0) {
			// 				system(strcat("./", de->d_name));
			// 			}
			// 		}
			// 		closedir(dr);
			// 	}
			// }

			// Send HTTP headers
			send(conn_socket, header, strlen(header), 0);
			send(conn_socket, resp, strlen(resp), 0);
			free(header);
			free(resp);
		} else {
			printf("Request Error. Try Again.\n");
		}

	    	// closing the connected socket
	    	close(conn_socket);
	}

	// closing the listening socket
	close(server_fd);

	printf("\nDone.\n");
	return 0;
}

