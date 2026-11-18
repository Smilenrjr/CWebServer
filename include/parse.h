#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define REQUEST_SIZE 1024

// TODO: list out ALL header content types
struct RequestHeader {
	char* method;
	char* dest;
	char* version;
	char* host;
	int conLen;
	char* conType;
	struct RequestPayload *reqPL;
};


// TODO: probably should use fixed size
struct RequestPayload {
	char field[REQUEST_SIZE];
	char value[REQUEST_SIZE];
	// char* field;
	// char* value;
};

extern struct RequestHeader parseHeader(char* buf);
extern void parseRequest(struct RequestPayload* reqPL, char* req);
