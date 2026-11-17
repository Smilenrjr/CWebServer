#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXDIRSIZE 256
#define NUMEXTS 3
#define SOTYPE 32

char *findFile(const char *fn);
char *readFile(const char *fn, long *out_fs, char **type);
