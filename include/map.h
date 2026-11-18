#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSIZE 100
#define MAXSTRSIZE 32


//
// Map: {key: value}
// { {key: {values}}, {key: {values}}, {key: {values}} }
//
// Keys:   {        key1,           key2,           key3,        ... }
//   |               |               |               |            |
// Values: { { {val1}, {val2} }, { {val3} }, { {val1}, {val3} }, ... }
//

struct _Values {
	char *value; // then {1, {"val"}, {"val"}}
};


struct _Keys {
	char* key;
};


struct Values {
	int size;	// size of values array
	struct _Values* _val;
};


struct KeyMap {
	int size;	// size of map
	struct _Keys* keys;
	struct Values* values;
};

int keymap_getIndex(struct KeyMap map, char* key);
void keymap_insert(struct KeyMap* map, char* key, char* value);
void keymap_init(struct KeyMap* map, char** keys, struct Values* values, int size);
struct Values keymap_initValues(char* values[], int size);
void keymap_free(struct KeyMap* map);
struct Values keymap_get(struct KeyMap map, char* key);
void keymap_printMap(struct KeyMap map);
