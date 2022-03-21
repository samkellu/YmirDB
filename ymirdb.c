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

void command_get_key(char* key) {
	//
}

void command_del(char* key) {
	//
}

void command_purge(char* key) {
	//
}

void command_set(char* key, int* values) {
	//
}

void command_push(char* key, int* values) {
	//
}

void command_append(char* key, int* values) {
	//
}

void command_pick(char* key, int index) {
	//
}

void command_pluck(char* key, int index) {
	//
}

void command_pop(char* key) {
	//
}

void command_drop(int id) {
	//
}

void command_rollback(int id) {
	//
}

void command_checkout(int id) {
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
	int token_number = 0;

	while (true) {
		printf("> ");

		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			command_bye();
			return 0;
		}
		input = line;
		while((token = strsep(&input, " ")) != NULL) {
			if (token_number++ == 0) {
				if (strcmp("HELP", token)) {
					command_help();
				} else if (strcmp("LIST", token)) {
					if ((token = strsep(&input, " ")) != NULL) {
						if (strcmp("KEYS", token)) {
							
						} else if (strcmp("ENTRIES", token)) {

						} else if (strcmp("SNAPSHOTS", token)) {

						}
					}
				} else if (strcmp("GET", token)) {

				} else if (strcmp("DEL", token)) {

				} else if (strcmp("PURGE", token)) {

				} else if (strcmp("SET", token)) {

				} else if (strcmp("PUSH", token)) {

				} else if (strcmp("APPEND", token)) {

				} else if (strcmp("PICK", token)) {

				} else if (strcmp("PLUCK", token)) {

				} else if (strcmp("POP", token)) {

				} else if (strcmp("DROP", token)) {

				} else if (strcmp("ROLLBACK", token)) {

				} else if (strcmp("CHECKOUT", token)) {

				} else if (strcmp("SNAPSHOT", token)) {

				} else if (strcmp("MIN", token)) {

				} else if (strcmp("MAX", token)) {

				} else if (strcmp("SUM", token)) {

				} else if (strcmp("LEN", token)) {

				} else if (strcmp("REV", token)) {

				} else if (strcmp("UNIQ", token)) {

				} else if (strcmp("SORT", token)) {

				} else if (strcmp("FORWARD", token)) {

				} else if (strcmp("BACKWARD", token)) {

				} else if (strcmp("TYPE", token)) {

				}
			}
			printf("%s ", token);
		}

	}

	return 0;
}
