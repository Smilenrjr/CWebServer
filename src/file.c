#include <file.h>
#include <map.h>

char *findFile(const char *fn) {
	struct dirent *de;
	DIR *dr = opendir(".");
	if (dr == NULL) {
		perror("could not open dir");
	} else {
		char *new_fn = (char *)malloc(sizeof(char) * MAXDIRSIZE);
		while ((de = readdir(dr)) != NULL) {
			if (strstr(de->d_name, fn) != NULL) {
				strncpy(new_fn, de->d_name, MAXDIRSIZE);
				closedir(dr);
				return new_fn;
			}
			if (strstr(de->d_name, "html") != NULL) {
				strncpy(new_fn, de->d_name, MAXDIRSIZE);
			}
		}
		closedir(dr);
		if (strlen(new_fn) > 0) return new_fn;
	}
	return NULL;
}


char *readFile(const char *fn, long *out_fs, char **type) {
	// see RFC-6838 and https://www.iana.org/assignments/media-types
	struct Values types[] = {{1, &(struct _Values){"text/html"}}, {1, &(struct _Values){"text/javascript"}}, {1, &(struct _Values){"text/plain"}}, {1, &(struct _Values){"application/pdf"}}, {1, &(struct _Values){"image/jpeg"}}};	// add content types as needed
	char *exts[5] = {"html", "js", "txt", "pdf", "jpg"};		// add file extensions as needed
	struct KeyMap map;
	keymap_init(&map, exts, types, 5);
	// keymap_insert(&map, "md", "text/plain");
	keymap_printMap(map);

	FILE *fp;
	char *ptr = NULL;
	if ((ptr = strstr(fn, ".")) == NULL) {
		char *file = findFile(fn);
		fp = fopen(file, "rb");
	     	strncpy(*type, keymap_get(map, strstr(file, ".")+1)._val->value, SOTYPE);
		free(file);
	} else {
		fp = fopen(fn, "rb");
		strncpy(*type, keymap_get(map, ptr+1)._val->value, SOTYPE);
	}


	if (type != NULL) {
		printf("\ntype: %s\n", *type);
	}

	if (fp == NULL) {
		perror("unable to open file");
		return NULL;
	}

	keymap_free(&map);
	

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


