#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <limits.h>
#include <sys/epoll.h>

#define PORT 8000
#define BUFF_SIZE 4096
#define QUERY_SIZE 256
#define HEADER_SIZE 1024

void handle_sigint(int sig);
char *read_html_file(const char *fn, long *out_fs);
char *makeHTTPHeader(char* status, char* type, long fs);
void readPostBody(int conn_socket, int len);
char **parseQuery(char* query);
void findHTMLFile(char **content, long *out_fs);
