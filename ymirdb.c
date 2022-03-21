/**
 * comp2017 - assignment 2
 * Sam Kelly
 * SKEL4720
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

#include "ymirdb.h"

//
// We recommend that you design your program to be
// modular where each function performs a small task
//
// e.g.
//
// command_bye
// command_help
// ...
// entry_add
// entry_delete
// ...
//

void command_bye() {
	printf("bye\n");
}

void command_help() {
	printf("%s\n", HELP);
}

void command_list_keys(entry *entries, int num_entries) {
	if (num_entries == 0) {
		printf("No keys\n");
		return;
	}
	for (int entry = 0; entry < num_entries; entry++) {
		printf("%s\n", entries[entry].key);
	}
}

void command_list_entries(entry *entries, int num_entries) {
	if (num_entries == 0) {
		printf("No entries\n");
		return;
	}
	for (int current_entry = 0; current_entry < num_entries; current_entry++) {
		int element_limit = entries[current_entry].length;
		printf("%s [", entries[current_entry].key);
		fflush(stdout);
		for (int element = 0; element < element_limit; element++) {
			int type = entries[current_entry].values[element].type;
			if (type == 0) {
				int value = entries[current_entry].values[element].value;
				printf("%d", value);
			} else {
				printf("%s", entries[current_entry].values[element].entry->key);
			}
			if (element != element_limit-1) {
				printf(", ");
			}
		}
		printf("]\n");
	}
	fflush(stdout);
}

void command_list_snapshots(snapshot *snapshots, int num_snapshots) {
	if (num_snapshots == 0) {
		printf("No snapshots\n");
		return;
	}
}

void command_get(char* key, snapshot* current_snapshot) {// +++ rework to return the entry, maybe a helper func?
	char new_key[MAX_KEY];
	for (int count = 0; count < MAX_KEY; count++) {
		if (count < strlen(key)) {
			new_key[count] = key[count];
			continue;
		}
		new_key[count] = '\0';
	}
	for (int entry_num = 0; entry_num < current_snapshot->num_entries; entry_num++) {
		entry current_entry = current_snapshot->entries[entry_num];
		if (strcmp(current_entry.key, new_key) == 0) {
			printf("[");
			for (int element_num = 0; element_num < current_entry.length; element_num++) {
				element current_element = current_entry.values[element_num];
				if (current_element.type == INTEGER) {
					printf("%d", current_element.value);
				} else {
					printf("%s", current_element.entry->key);
				}
				if (element_num != current_entry.length-1) {
					printf(", ");
				}
			}
			printf("]\n");
			return;
		}
	}
	printf("No such key");
}

void command_del(char* key, snapshot* current_snapshot) {
	//
}

void command_purge(char* key) {
	//
}

void command_set(char** array, int array_length, snapshot* current_snapshot) {
	printf("setting: \n");
	current_snapshot->num_entries++;
	current_snapshot->entries = (entry*) realloc(current_snapshot->entries, current_snapshot->num_entries*sizeof(entry));
	entry current_entry;
	current_entry.length = array_length-2;

	char key[MAX_KEY];
	for (int count = 0; count < MAX_KEY; count++) {
		if (count < strlen(array[1])) {
			key[count] = array[1][count];
			continue;
		}
		key[count] = '\0';
	}
	memcpy(current_entry.key, key, MAX_KEY);
	element *values = malloc(sizeof(element) * (array_length - 2));
	for (int arg = 2; arg < array_length; arg++) {
		element new_element;
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			printf("	Integer %s\n", array[arg]);
			new_element.type = INTEGER;
			new_element.value = (int)strtol(array[arg], NULL, 10);
		} else {
			printf("	entry %s\n", array[arg]);
			new_element.type = ENTRY;
		//	new_element.backward = current_entry; +++
		//	current_entry.forward = realloc(current_entry.forward) +++
			entry *new_entry = malloc(sizeof(entry));
			new_entry->length = 0;
			char new_key[MAX_KEY];
			for (int count = 0; count < MAX_KEY; count++) {
				if (count < strlen(array[arg])) {
					new_key[count] = array[arg][count];
					continue;
				}
				new_key[count] = '\0';
			}
			memcpy(new_entry->key, new_key, MAX_KEY);
			new_element.entry = new_entry;
		}
		values[arg-2] = new_element;
	}
	current_entry.values = values;
	current_entry.length = array_length-2;
	current_snapshot->entries[current_snapshot->num_entries-1] = current_entry;
}

void command_push(char** array) {
	//
}

void command_append(char** array) {
	//
}

void command_pick(char* key, char* index) {
	//
}

void command_pluck(char* key, char* index) {
	//
}

void command_pop(char* key) {
	//
}

void command_drop(char* id) {
	//
}

void command_rollback(char* id) {
	//
}

void command_checkout(char* id) {
	//
}

void command_snapshot() {
	//
}

void command_min(char* key) {
	//
}

void command_max(char* key) {
	//
}

void command_sum(char* key) {
	//
}

void command_len(char* key) {
	//
}

void command_rev(char* key) {
	//
}

void command_uniq(char* key) {
	//
}

void command_sort(char* key) {
	//
}

void command_forward(char* key) {
	//
}

void command_backward(char* key) {
	//
}

int command_type(char* key) {// +++ check all elements in entry...
	if (key[0] >= '0' && key[0] <= '9') {
		return 0;
	}
	return 1;
}

int main(void) {

	char line[MAX_LINE];
	char *token,*input;

	int snapshot_number = 0;
	snapshot *snapshots = (snapshot*) malloc(sizeof(snapshot));
	snapshot current_snapshot = {snapshot_number, NULL, 0, NULL, NULL};
	snapshots[0] = current_snapshot;
	current_snapshot.num_entries = 0;
	while (true) {
		printf("> ");

		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			command_bye();
			return 0;
		}

		input = line;
		char **arg_array = (char**) malloc(sizeof(char**));
		int array_length = 0;
		while ((token = strsep(&input, " ")) != NULL) {
			token = strsep(&token, "\n");
			arg_array = (char**)realloc(arg_array, (array_length+1)*sizeof(char**));
			arg_array[array_length] = (char*)malloc(sizeof(char*));
			arg_array[array_length] = token;
			array_length++;
		}

		char *arg = arg_array[0];
		if (strcmp("BYE", arg) == 0) {
			command_bye();
			// return 0;
		} else if (strcmp("HELP", arg) == 0) {
			command_help();
		} else if (strcmp("LIST", arg) == 0) {
				char *arg = arg_array[1];//+++
				if (strcmp("KEYS", arg) == 0) {
					command_list_keys(current_snapshot.entries, current_snapshot.num_entries);
				} else if (strcmp("ENTRIES", arg) == 0) {
					command_list_entries(current_snapshot.entries, current_snapshot.num_entries);
				} else if (strcmp("SNAPSHOTS", arg) == 0) {
					command_list_snapshots(snapshots, current_snapshot.num_entries);
				} else {
					continue;
				}
		} else if (strcmp("GET", arg) == 0) {
			command_get(arg_array[1], &current_snapshot);
		} else if (strcmp("DEL", arg) == 0) {
			command_del(arg_array[1], &current_snapshot);
		} else if (strcmp("PURGE", arg) == 0) {
			command_purge(arg_array[1]);
		} else if (strcmp("SET", arg) == 0) {
			command_set(arg_array, array_length, &current_snapshot);
		} else if (strcmp("PUSH", arg) == 0) {
			command_push(arg_array);
		} else if (strcmp("APPEND", arg) == 0) {
			command_append(arg_array);
		} else if (strcmp("PICK", arg) == 0) {
			command_pick(arg_array[1],arg_array[2]);
		} else if (strcmp("PLUCK", arg) == 0) {
			command_pluck(arg_array[1],arg_array[2]);
		} else if (strcmp("POP", arg) == 0) {
			command_pop(arg_array[1]);
		} else if (strcmp("DROP", arg) == 0) {
			command_drop(arg_array[1]);
		} else if (strcmp("ROLLBACK", arg) == 0) {
			command_rollback(arg_array[1]);
		} else if (strcmp("CHECKOUT", arg) == 0) {
			command_checkout(arg_array[1]);
		} else if (strcmp("SNAPSHOT", arg) == 0) {
			command_snapshot();
		} else if (strcmp("MIN", arg) == 0) {
			command_min(arg_array[1]);
		} else if (strcmp("MAX", arg) == 0) {
			command_max(arg_array[1]);
		} else if (strcmp("SUM", arg) == 0) {
			command_sum(arg_array[1]);
		} else if (strcmp("LEN", arg) == 0) {
			command_len(arg_array[1]);
		} else if (strcmp("REV", arg) == 0) {
			command_rev(arg_array[1]);
		} else if (strcmp("UNIQ", arg) == 0) {
			command_uniq(arg_array[1]);
		} else if (strcmp("SORT", arg) == 0) {
			command_sort(arg_array[1]);
		} else if (strcmp("FORWARD", arg) == 0) {
			command_forward(arg_array[1]);
		} else if (strcmp("BACKWARD", arg) == 0) {
			command_backward(arg_array[1]);
		} else if (strcmp("TYPE", arg) == 0) {
			if (command_type(arg_array[1])) {
				printf("GENERAL\n");
			} else {
				printf("SIMPLE\n");
			}
		}
		free(arg_array);
	}
	return 0;
}
