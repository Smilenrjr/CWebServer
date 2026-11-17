#include <server.h>
#include <map.h>
#include <file.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
	(void)sig;
	printf("\nKeyboard Interrupt. Exiting Program.\n");
	keep_running = 0;
}


char *read_html_file(const char *fn, long *out_fs) {
	FILE *fp;
	if (strstr(fn, ".") == NULL) {
		size_t sz = sizeof(fn) + sizeof(".html");
		char *dst = malloc(sz);
		strncpy(dst, fn, sz);
		char *file = strcat(dst, ".html");
		printf("file: %s\n", dst);
		fp = fopen(file, "rb");
	} else {
		fp = fopen(fn, "rb");
	}
	// fp = fopen(fn, "rb");
	if (fp == NULL) {
		perror("unable to open file");
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long fs = ftell(fp);
	rewind(fp);

	char *buf = malloc(fs + 1);
	if (buf == NULL) {
		perror("Memory allocation failed");
		fclose(fp);
		return NULL;
	}

	size_t bytes_read = fread(buf, 1, fs, fp);
	buf[bytes_read] = '\0';

	fclose(fp);
	if(out_fs) {
		*out_fs = (long)bytes_read;
	}

	return buf;
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


void readPostBody(int conn_socket, int len) {
	if (len <= 0) return;

	char *body = (char *)malloc(len + 1);
	if (!body) {
		perror("failed to allocate memory");
		return;
	}

	int total_bytes_read = 0;
	while(total_bytes_read < len) {
		ssize_t bytes_read = recv(conn_socket, body + total_bytes_read, len - total_bytes_read, 0);

		if (bytes_read <= 0) {
			break;
		}

		total_bytes_read += bytes_read;
	}

	body[len] = '\0';
	printf("Received POST body: %s\n", body);
	free(body);
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


void findHTMLFile(char **content, long *out_fs) {
	struct dirent *de;
	DIR *dr = opendir(".");
	if (dr == NULL) {
		perror("could not open dir");
	} else {
		while ((de = readdir(dr)) != NULL) {
			if (strstr(de->d_name, ".html") != NULL) {
				*content = read_html_file(de->d_name, out_fs);
			}
		}
		closedir(dr);
	}
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

	// TESTING MAP
	struct KeyMap map;
	struct Values values;
	char **keys;
	keymap_init(&map, keys, &values, 0);
	keymap_insert(&map, "md", "text/plain");
	keymap_insert(&map, "txt", "text/plain");
	keymap_insert(&map, "html", "text/html");
	keymap_insert(&map, "js", "text/javascript");
	values = keymap_get(map, "js");
	printf("%s\n", values._val[0].value);
	keymap_printMap(map);
	keymap_free(&map);
	long out;
	char* test;
	char* fn = (char *)malloc(sizeof(char) * MAXDIRSIZE);
	strcpy(fn, "index");
	char* type = (char *)malloc(sizeof(char) * 128);
	test = readFile(fn, &out, &type);
	printf("\nfile: %s\n", test);
	free(test);
	free(type);
	free(fn);
	// END TESTING

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
			exit(EXIT_FAILURE);
	    	}
	  
		read(conn_socket, buf, 1023);

		char *ptr = buf;
		while (*ptr) {
			printf("%c", *ptr);
			ptr++;
		}

		// parse HTTP req method and HTTP req filename
		// TODO: rewrite this without strtok.
		//
		// buf[strcspn(buf, "\r\n")] = ' ';
		char *fields[2] = {NULL, NULL};
		int ind = 0;
		int content_len = 0;
		for (char *tok = strtok(buf, " "); tok != NULL; tok = strtok(NULL, " ")) {
			if (ind < 2) {
				fields[ind++] = tok;
			}

			// parse HTTP req Content-Length
			if (strstr(tok, "Content-Length:") != NULL) {
				tok = strtok(NULL, "\r\n");
				// convert char* to int only if valid
				char *endptr;
				errno = 0;
				long val = strtol(tok, &endptr, 10);
				if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (val > INT_MAX || val < INT_MIN)) {
					perror("Error Fetching Content-Length: Buffer Overflow");
				} else if (endptr == tok) {
					perror("Error: No Content-Length");
				} else {
					content_len = (int)val;
				}
			}
		}

		if (ind != 2) {
			fprintf(stderr, "expected method and destination\n");
			close(conn_socket);
			continue;
		}
		
		printf("--- method: %s --- dir: %s ---\r\n\r\n", fields[0], fields[1]);

		printf("--- Content-Length: %d ---\r\n\r\n", content_len);


		if(strncmp(fields[0], "GET", strlen("GET")) == 0) {
			long file_size = 0;
			char *html_content = NULL;
			char* type = (char *)malloc(sizeof(char) * SOTYPE);
			char* header;
			if (strlen(fields[1]) == 1) {
				// findHTMLFile(&html_content, &file_size);
				html_content = readFile(fields[1], &file_size, &type);
			}

			int isCont = 1;
			// TODO: get any file type specified (i.e. JS, PHP, PDF, etc)
			if (strlen(fields[1]) > 1){
				if (strncmp(fields[1] + 1, "favicon.ico", strlen(fields[1]+1)) == 0) {	// TODO: parse favicon request.
					isCont = 0;
				} else if (strchr(fields[1] + 1, '?') != NULL) {	// TODO: parse query with attributes
					//isCont = 0;
					char **_attributes;
					_attributes = parseQuery(fields[1]);
					if (_attributes != NULL) {	// If query contains attributes
						if (*_attributes != NULL) {	// are there attributes?
							if (strlen(*_attributes) == 1) {	// if no dest given
								findHTMLFile(&html_content, &file_size);
							} else html_content = read_html_file(*_attributes + 1, &file_size);	// else try dest
							printf("destination: %s\n", *_attributes + 1);
						} else isCont = 0;	// no contents specified
						while (*_attributes != NULL) {
							printf("attributes: %s\n", *_attributes++);
						}
						free(_attributes);
					} else {	// no attributes
						printf("attributes null\n");
						isCont = 0;
					}
				} else {	// try header dest field
					// html_content = read_html_file(fields[1] + 1, &file_size);	
					html_content = readFile(fields[1] + 1, &file_size, &type);	
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
		} else if(strncmp(fields[0], "POST", strlen(fields[0])) == 0) {
			char *header = makeHTTPHeader("200 OK", "text/html", 0);

			readPostBody(conn_socket, content_len);
			printf("Done Reading Body\r\n\r\n");
			//TODO: Need a way to update server resources from
			// 	valid POST requests.
			//
			// if (strlen(fields[1]) > 1) {
			// 	struct dirent *de;
			// 	DIR *dr = opendir(".");
			// 	if (dr == NULL) {
			// 		perror("could not open dir");
			// 	} else {
			// 		while ((de = readdir(dr)) != NULL) {
			// 			if (strcmp(de->d_name, fields[1] + 1) == 0) {
			// 				system(strcat("./", de->d_name));
			// 			}
			// 		}
			// 		closedir(dr);
			// 	}
			// }

			// Send HTTP headers
			send(conn_socket, header, strlen(header), 0);
			free(header);
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

