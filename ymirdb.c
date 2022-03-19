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

void command_list_keys() {
	//
}

void command_list_entries() {
	//
}

void command_list_snapshots() {
	//
}

void command_get(char* key) {
	//
}

void command_del(char* key) {
	//
}

void command_purge(char* key) {
	//
}

void command_set(char** array, int array_length) {
	printf("setting: \n");
	entry current_entry;
	memcpy(current_entry.key, array[1], MAX_KEY);
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
			entry new_entry;
			memcpy(new_entry.key, array[arg], MAX_KEY);
			new_element.entry = &new_entry;
		}
		values[arg-2] = new_element;
	}
	current_entry.values = values;
	current_entry.length = array_length-2;
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

void command_type(char* key) {
	//
}

int main(void) {

	char line[MAX_LINE];
	char *token,*input;

	while (true) {
		printf("> ");

		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			command_bye();
			return 0;
		}

		input = line;
		char **arg_array = (char**)malloc(sizeof(char**));
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
			printf("registered list");
				char *arg = arg_array[1];//+++
				if (strcmp("KEYS", arg) == 0) {
					command_list_keys();
				} else if (strcmp("ENTRIES", arg) == 0) {
					command_list_entries();
				} else if (strcmp("SNAPSHOTS", arg) == 0) {
					command_list_snapshots();
				} else {
					continue;
				}
		} else if (strcmp("GET", arg) == 0) {
			command_get(arg_array[1]);
		} else if (strcmp("DEL", arg) == 0) {
			command_del(arg_array[1]);
		} else if (strcmp("PURGE", arg) == 0) {
			command_purge(arg_array[1]);
		} else if (strcmp("SET", arg) == 0) {
			command_set(arg_array, array_length);
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
			command_type(arg_array[1]);
		}
		free(arg_array);
	}
	return 0;
}
