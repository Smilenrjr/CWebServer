#include <map.h>


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
