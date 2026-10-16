#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#define PORT 8000
volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
	(void)sig;
	printf("\nKeyboard Interrupt. Exiting Program.\n");
	keep_running = 0;
}

char *read_html_file(const char *fn, long *out_fs) {
	
	FILE *fp = fopen(fn, "rb");
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


int main(int argc, char const* argv[])
{
	if (argc < 2) {
		perror("need file arg");
		exit(EXIT_FAILURE);
	}
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
			exit(EXIT_FAILURE);
	    	}
	  
		read(conn_socket, buf, 1023);
		char *ptr = buf;
		while (*ptr) {
			printf("%c", *ptr);
			ptr++;
	    	}

	    	long file_size = 0;
	    	char *html_content = read_html_file(argv[1], &file_size);
	    	if (html_content == NULL) {
			const char *not_found =
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 9\r\n"
				"Connection: close\r\n"
				"\r\n"
				"Not Found";
			send(conn_socket, not_found, strlen(not_found), 0);
			perror("cannot read file");
			exit(EXIT_FAILURE);
	    	}

		char header_buffer[512];

		int header_len = snprintf(header_buffer, sizeof(header_buffer),
				"HTTP/1.1 200 OK\r\n"
				"Server: WebServer/1.0\r\n"
				"Content-Type: text/html; charset=UTF-8\r\n"
				"Content-Length: %ld\r\n"
				"Connection: close\r\n"
				"\r\n",
				file_size
		);
	    	
		// Send HTTP headers
		send(conn_socket, header_buffer, header_len, 0);

		// Send HTML file content
		send(conn_socket, html_content, file_size, 0);

		free(html_content);

	    	// closing the connected socket
	    	close(conn_socket);
	}

	// closing the listening socket
	close(server_fd);

	printf("\nDone.\n");
	return 0;
}

