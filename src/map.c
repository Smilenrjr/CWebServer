#include <map.h>

int map_getIndex(struct Map map, char* key) {
	for (int i = 0; i < map.size; i++) {
		if (strncmp(map.keys[i], key, MAXSTRSIZE) == 0) {
			return i;
		}
	}
	return -1;
}


void map_insert(struct Map* map, char* key, char* value) {
	int idx = map_getIndex(*map, key);
	if (idx == -1) {
		map->keys[map->size] = (char *)malloc(sizeof(char) * MAXSTRSIZE);
		strncpy(map->keys[map->size], key, MAXSTRSIZE);
		map->values[map->size] = (char *)malloc(sizeof(char) * MAXSTRSIZE);
		strncpy(map->values[map->size], value, MAXSTRSIZE);
		map->size++;
	} else {
		// for this implementation, if key already assigned to a value, reassign. 
		// TODO: Make each value an arrays so keys can store multiple values
		strncpy(map->values[idx], value, MAXSTRSIZE);
	}
}


void map_init(struct Map* map, char* keys[MAXSIZE], char* values[MAXSIZE], int newSize) {
	for (int i = 0; i < newSize; i++) {
		map_insert(map, keys[i], values[i]);
		// strncpy(map->keys[i], keys[i], MAXSTRSIZE);
		// strncpy(map->values[i], values[i], MAXSTRSIZE);
	}
}


char* map_get(struct Map map, char* key) {
	int idx = map_getIndex(map, key);
	if (idx == -1) return NULL;
	else return map.values[idx];
}


void map_printMap(struct Map map) {
	for (int i = 0; i < map.size; i++) {
		printf("%s: %s\n", map.keys[i], map.values[i]);
	}
}



// Alternate Implementation:

int keymap_getIndex(struct KeyMap map, char* key) {
	for (int i = 0; i < map.size; i++) {
		if (strncmp(map.keys[i].key, key, MAXSTRSIZE) == 0) {
			return i;
		} 
	}
	return -1;
}


void keymap_insert(struct KeyMap* map, char* key, char* value) {
	int idx = keymap_getIndex(*map, key);
	if (idx == -1) {
		int mSize = map->size;
		struct Values *val = &map->values[mSize];
		struct _Keys *_key = &map->keys[mSize];
		val->size = 0;
		_key->key = (char *)malloc(sizeof(char) * MAXSTRSIZE);
		strncpy(_key->key, key, MAXSTRSIZE);
		// val->_val = (char **)malloc(MAXSIZE);
		val->_val = (struct _Values *)malloc(sizeof(struct _Values) * MAXSIZE);
		struct _Values *_val = &val->_val[val->size];
		_val->value = (char *)malloc(sizeof(char) * MAXSTRSIZE);
		strncpy(_val->value, value, MAXSTRSIZE);
		val->size++;
		map->size++;
	} else {
		struct Values *val = &map->values[idx];
		struct _Keys *_key = &map->keys[idx];
		struct _Values *_val = &val->_val[val->size];
		_val->value = (char *)malloc(sizeof(char) * MAXSTRSIZE);
		strncpy(_val->value, value, MAXSTRSIZE);
		map->values[idx].size++;
	}

}


void keymap_init(struct KeyMap* map, char** keys, struct Values* values, int size) {
	map->size = 0;
	map->keys = (struct _Keys *)malloc(sizeof(struct _Keys) * MAXSIZE);
	map->values = (struct Values *)malloc(sizeof(struct Values) * MAXSIZE);

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < values->size; j++) {
			keymap_insert(map, keys[i], values[i]._val[j].value);
		}
	}
}


void keymap_free(struct KeyMap* map) {
	for (int i = 0; i < map->size; i++) {
		for (int j = 0; j < map->values[i].size; j++) {
			free(map->values[i]._val[j].value);
		}
		free(map->values[i]._val);
		free(map->keys[i].key);
	}
	free(map->values);
	free(map->keys);
}


struct Values keymap_get(struct KeyMap map, char* key) {
	struct Values vals;
	int idx = keymap_getIndex(map, key);
	if (idx == -1) return vals;
	else return map.values[idx];
}


void keymap_printMap(struct KeyMap map) {
	printf("\n--------------------------------------------------------------------\n");
	for (int i = 0; i < map.size; i++) {
		printf("| Key: %s\t| ", map.keys[i].key);
		for (int j = 0; j < map.values[i].size; j++) {
			printf("Value: %s\t| ", map.values[i]._val[j].value);
		}
		printf("\n");
	}
	printf("--------------------------------------------------------------------\n");
}
