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
#include <ctype.h>
#include <limits.h>

#include "ymirdb.h"

int total_snapshots = 0;
int snapshot_counter = 0;
snapshot current_state;

entry* get_entry(char* key) {
	for (int entry_num = 0; entry_num < current_state.num_entries; entry_num++) {
		if (strcmp(current_state.entries[entry_num].key, key) == 0) {
			return &current_state.entries[entry_num];
		}
	}
	return NULL;
}

snapshot* get_snapshot(snapshot* snapshots, int id) {
	if (id > snapshot_counter || id < 0) {
		return NULL;
	}
	for (int snap = 0; snap < total_snapshots; snap++) {
		if (snapshots[snap].id == id) {
			return &snapshots[snap];
		}
	}
	return NULL;
}

void command_bye(snapshot* snapshots) {
	for (int current_entry = 0; current_entry < current_state.num_entries; current_entry++) {
		free(current_state.entries[current_entry].backward);
		free(current_state.entries[current_entry].forward);
		free(current_state.entries[current_entry].values);
	}
	free(current_state.entries);
	for (int current_snapshot = 0; current_snapshot < total_snapshots; current_snapshot++) {
		current_state = snapshots[current_snapshot];
		for (int current_entry = 0; current_entry < snapshots[current_snapshot].num_entries; current_entry++) {
			entry* free_entry = get_entry(snapshots[current_snapshot].entries[current_entry].key);
			free(free_entry->backward);
			free(free_entry->forward);
			free(free_entry->values);
		}
		free(snapshots[current_snapshot].entries);
	}
	free(snapshots);
	printf("bye\n");
	exit(0);
}

void command_help() {
	printf("%s\n", HELP);
}

void command_list_keys() {
	if (current_state.num_entries == 0) {
		printf("no keys\n\n");
		return;
	}
	for (int entry = current_state.num_entries-1; entry >= 0; entry--) {
		printf("%s\n", current_state.entries[entry].key);
	}
	printf("\n");
}

void command_list_entries() {
	if (current_state.num_entries == 0) {
		printf("no entries\n\n");
		return;
	}
	for (int current_entry = current_state.num_entries - 1; current_entry >= 0; current_entry--) {
		entry* read_entry = get_entry(current_state.entries[current_entry].key);
		printf("%s [", read_entry->key);
		for (int element = 0; element < read_entry->length; element++) {
			if (read_entry->values[element].type == INTEGER) {
				int value = read_entry->values[element].value;
				printf("%d", value);
			} else {
				printf("%s", read_entry->values[element].entry->key);
			}
			if (element != read_entry->length - 1) {
				printf(" ");
			}
		}
		printf("]\n");
	}
	printf("\n");
	fflush(stdout);
}

void command_list_snapshots(snapshot *snapshots) {
	if (total_snapshots == 0) {
		printf("no snapshots\n\n");
		return;
	}
	for (int snap_index = snapshot_counter; snap_index >= 0; snap_index--) {
		snapshot* current_snapshot = get_snapshot(snapshots, snap_index);
		if (current_snapshot != NULL) {
			printf("%d\n", current_snapshot->id);
		}
	}
	printf("\n");
}

void command_get(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		printf("[");
		for (int element_num = 0; element_num < current_entry->length; element_num++) {
			element current_element = current_entry->values[element_num];
			if (current_element.type == INTEGER) {
				printf("%d", current_element.value);
			} else {
				printf("%s", current_element.entry->key);
			}
			if (element_num != current_entry->length-1) {
				printf(" ");
			}
		}
		printf("]\n\n");
		return;
	}
	printf("no such key\n\n");
}

void command_del(char* key, int quiet) {
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		if (current_entry->backward_size > 0) {
			if (!quiet) {
				printf("not permitted\n\n");
			}
			return;
		}
		for  (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
			entry* test_entry = get_entry(current_state.entries[entry_index].key);
			int del_found = 0;
			for (int backward_index = 0; backward_index < test_entry->backward_size; backward_index++) {
				if (strcmp(test_entry->backward[backward_index].key, current_entry->key) == 0) {
					del_found = 1;
				}
				if (del_found) {
					if (backward_index != test_entry->backward_size - 1) {
						test_entry->backward[backward_index] = test_entry->backward[backward_index+1];
					}
				}
			}
			if (del_found) {
				test_entry->backward = realloc(test_entry->backward, sizeof(entry) * --test_entry->backward_size);
			}
		}
		int del_found = 0;
		for  (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
			entry* test_entry = get_entry(current_state.entries[entry_index].key);
			if (strcmp(test_entry->key, key) == 0) {
				del_found = 1;
				free(current_entry->values);
				free(current_entry->forward);
				free(current_entry->backward);
			}
			if (del_found) {
				if (entry_index != current_state.num_entries - 1) {
					current_state.entries[entry_index] = current_state.entries[entry_index+1];
				}
			}
		}
		current_state.num_entries--;
		current_state.entries = realloc(current_state.entries, current_state.num_entries * sizeof(entry));
		for (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
			entry* current_entry = get_entry(current_state.entries[entry_index].key);
			for (int element_index = 0; element_index < current_entry->length; element_index++) {
				if (current_entry->values[element_index].type == ENTRY) {
					current_entry->values[element_index].entry = get_entry(current_entry->values[element_index].key);
				}
			}
		}
		if (!quiet) {
			printf("ok\n\n");
		}
		return;
	}
	if (!quiet) {
		printf("no such key\n\n");
	}
}

void command_purge(char* key, snapshot* snapshots) {
	snapshot original_snapshot;
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		if (current_entry->backward_size > 0) {
			printf("not permitted\n\n");
			return;
		}
	}
	memcpy(&original_snapshot, &current_state, sizeof(snapshot));
	for (int snapshot_index = 0; snapshot_index < total_snapshots; snapshot_index++) {
		current_state = snapshots[snapshot_index];
		entry* current_entry = get_entry(key);
		if (current_entry != NULL) {
			if (current_entry->backward_size > 0) {
				printf("not permitted\n\n");
				current_state = original_snapshot;
				return;
			}
		}
	}
	current_state = original_snapshot;
	command_del(key, 1);
	memcpy(&original_snapshot, &current_state, sizeof(snapshot));
	for (int snapshot_index = 0; snapshot_index < total_snapshots; snapshot_index++) {
		current_state = snapshots[snapshot_index];
		current_state.entries = snapshots[snapshot_index].entries;
		command_del(key, 1);
		snapshots[snapshot_index].entries = current_state.entries;
		snapshots[snapshot_index].num_entries = current_state.num_entries;
	}
	current_state = original_snapshot;
	printf("ok\n\n");
}

void command_set(char** array, int array_length) {
	for (int check_arg = 2; check_arg < array_length; check_arg++) {
		if (!(((char)array[check_arg][0] >= '0' && (char)array[check_arg][0] <= '9') || array[check_arg][0] == '-')) {
			if (strcmp(array[check_arg], array[1]) == 0) {
				printf("not permitted\n\n");
				return;
			}
			entry* test_entry = get_entry(array[check_arg]);
			if (test_entry == NULL) {
				printf("no such key\n\n");
				return;
			}
		}
	}
	entry* current_entry = get_entry(array[1]);
	if (current_entry == NULL) {
		current_state.entries = realloc(current_state.entries, sizeof(entry) * (++current_state.num_entries));
		for (int entry_index = 0; entry_index < current_state.num_entries - 1; entry_index++) {
			for (int element_index = 0; element_index < current_state.entries[entry_index].length; element_index++) {
				if (current_state.entries[entry_index].values[element_index].type == ENTRY) {
					current_state.entries[entry_index].values[element_index].entry = get_entry(current_state.entries[entry_index].values[element_index].key);
				}
			}
		}
		current_entry = &current_state.entries[current_state.num_entries-1];
		current_entry->forward_size = 0;
		current_entry->backward_size = 0;
		current_entry->forward = NULL;
		current_entry->backward = NULL;
		current_entry->values = NULL;
	}

	for (int forward_index = 0; forward_index < current_entry->forward_size; forward_index++) {
		entry* test_entry = get_entry(current_entry->forward[forward_index].key);
		int del_found = 0;
		for (int backward_index = 0; backward_index < test_entry->backward_size; backward_index++) {
			if (strcmp(test_entry->backward[backward_index].key, current_entry->key) == 0) {
				del_found = 1;
			}
			if (del_found) {
				if (backward_index != test_entry->backward_size - 1) {
					test_entry->backward[backward_index] = test_entry->backward[backward_index+1];
				}
			}
		}
		if (del_found) {
			test_entry->backward_size--;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry) * test_entry->backward_size);
		}
	}

	current_entry->forward_size = 0;
	free(current_entry->forward);
	for (int backward_index = 0; backward_index < current_entry->backward_size; backward_index++) {
		entry* test_entry = get_entry(current_entry->backward[backward_index].key);
		for (int forward_index = 0; forward_index < test_entry->forward_size; forward_index++) {
			if (strcmp(test_entry->forward[forward_index].key, current_entry->key) == 0) {
				test_entry->forward[forward_index] = *current_entry;
			}
		}
	}
	current_entry->values = realloc(current_entry->values, sizeof(element) * (array_length - 2));
	current_entry->length = array_length-2;
	current_entry->forward = NULL;

	memcpy(current_entry->key, array[1], MAX_KEY);
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[arg-2];
		if ((array[arg][0] >= '0' && array[arg][0] <= '9') || array[arg][0] == '-') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		} else {
			entry* test_entry = get_entry(array[arg]);
			test_entry->backward_size++;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry)*test_entry->backward_size);
			test_entry->backward[test_entry->backward_size-1] = *current_entry;

			current_entry->forward_size++;
			current_entry->forward = realloc(current_entry->forward, current_entry->forward_size * sizeof(entry));
			new_element->type = ENTRY;
			new_element->entry = test_entry;
			memcpy(new_element->key, test_entry->key, MAX_KEY);
			current_entry->forward[current_entry->forward_size-1] = *test_entry;
		}
	}
	printf("ok\n\n");
}

int value_checks(char** array, int array_length, entry* current_entry) {
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return 1;
	}
	for (int current_element = 2; current_element < array_length; current_element++) {
		if (strcmp(array[current_element], current_entry->key) == 0) {
			printf("not permitted\n\n");
			return 1;
		}
	}
	return 0;
}

void command_push(char** array, int array_length) {
	for (int arg = 2; arg < array_length; arg++) {
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			continue;
		}
		entry* current_entry = get_entry(array[arg]);
		if (current_entry == NULL) {
			printf("no such key\n\n");
			return;
		}
	}
	entry* current_entry = get_entry(array[1]);
	if (value_checks(array, array_length, current_entry)) {
		return;
	}
	int old_len = current_entry->length;
	current_entry->length += array_length - 2;
	current_entry->values = realloc(current_entry->values, current_entry->length * sizeof(element));
	for (int original_elem = 0; original_elem < old_len; original_elem++) {
		current_entry->values[current_entry->length - 1 - original_elem] = current_entry->values[old_len - original_elem - 1];
	}
	if (old_len == 0) {
		old_len = current_entry->length;
	}
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[(array_length - 2) - (arg - 2) - 1];
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		} else {
			entry* test_entry = get_entry(array[arg]);
			test_entry->backward_size++;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry)*test_entry->backward_size);
			test_entry->backward[test_entry->backward_size-1] = *current_entry;

			current_entry->forward_size++;
			current_entry->forward = realloc(current_entry->forward, current_entry->forward_size * sizeof(entry));
			new_element->type = ENTRY;
			new_element->entry = test_entry;
			memcpy(new_element->key, test_entry->key, MAX_KEY);
			current_entry->forward[current_entry->forward_size-1] = *test_entry;
		}
	}
	printf("ok\n\n");
}

void command_append(char** array, int array_length) {
	for (int arg = 2; arg < array_length; arg++) {
		if ((array[arg][0] >= '0' && array[arg][0] <= '9') || array[arg][0] == '-') {
			continue;
		}
		entry* current_entry = get_entry(array[arg]);
		if (current_entry == NULL) {
			printf("no such key\n\n");
			return;
		}
	}
	entry* current_entry = get_entry(array[1]);
	if (value_checks(array, array_length, current_entry)) {
		return;
	}

	int old_len = current_entry->length;
	current_entry->length += array_length - 2;
	current_entry->values = realloc(current_entry->values, current_entry->length * sizeof(element));
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[old_len + (arg-2)];
		if ((array[arg][0] >= '0' && array[arg][0] <= '9') || array[arg][0] == '-') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		} else {
			entry* test_entry = get_entry(array[arg]);
			test_entry->backward_size++;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry)*test_entry->backward_size);
			test_entry->backward[test_entry->backward_size-1] = *current_entry;

			current_entry->forward_size++;
			current_entry->forward = realloc(current_entry->forward, current_entry->forward_size * sizeof(entry));
			new_element->type = ENTRY;
			new_element->entry = test_entry;
			memcpy(new_element->key, test_entry->key, MAX_KEY);
			current_entry->forward[current_entry->forward_size-1] = *test_entry;
		}
	}
	printf("ok\n\n");
}

void command_pick(char* key, int index) {
	index--;
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	if (index >= current_entry->length || index < 0) {
		printf("index out of range\n\n");
		return;
	}
	if (current_entry->values[index].type == INTEGER) {
		printf("%d\n\n", current_entry->values[index].value);
		return;
	}
	printf("%s\n\n", current_entry->values[index].entry->key);
}

//update forward and backward +++
//validity +++
//del quiet?? +++
void command_pluck(char* key, int index) {
	entry* current_entry = get_entry(key);
	index--;
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	if (current_entry->length <= index) {
		printf("index out of range\n\n");
		return;
	}

	if (current_entry->values[index].type == INTEGER) {
		printf("%d\n\n", current_entry->values[index].value);
	} else {
		printf("%s\n\n", current_entry->values[index].entry->key);
		int valid = 1;
		for (int element_index = 0; element_index < current_entry->length; element_index++) {
			if (strcmp(current_entry->values[element_index].key, current_entry->values[index].entry->key) == 0) {
				valid--;
			}
		}
		if (valid == 0) {
			for (int forward_entry = 0; forward_entry < current_entry->forward_size; forward_entry++) {
				if (strcmp(current_entry->forward[forward_entry].key, current_entry->values[index].entry->key) == 0) {
					del_found = 1;
				}
				if (del_found && forward_entry != current_entry->forward_size - 1) {
					current_entry->forward[forward_entry] = current_entry->forward[forward_entry+1];
				}
			}
			current_entry->forward = realloc(current_entry->forward, sizeof(entry*) * --current_entry->forward_size);

			entry* test_entry = get_entry(current_entry->values[index].entry->key);
			int del_found = 0;
			for (int backward_entry = 0; backward_entry < test_entry->backward_size; backward_entry++) {
				if (strcmp(test_entry->backward[backward_entry].key, current_entry->values[index].entry->key) == 0) {
					del_found = 1;
				}
				if (del_found && backward_entry != test_entry->backward_size - 1) {
					test_entry->backward[backward_entry] = test_entry->backward[backward_entry+1];
				}
			}
			test_entry->backward = realloc(test_entry->backward, sizeof(entry*) * --test_entry->backward_size);
		}
	}
	for (int element_index = index; element_index < current_entry->length; element_index++) {
		if (element_index != current_entry->length - 1) {
			current_entry->values[element_index] = current_entry->values[element_index + 1];
		}
	}
	current_entry->values = realloc(current_entry->values, sizeof(element) * --current_entry->length);
}

void command_pop(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	if (current_entry->length <= 0) {
		printf("nil\n\n");
		return;
	}
	command_pluck(key, 1);
}

snapshot* command_drop(int id, snapshot* snapshots, int quiet) {
	snapshot* current_snapshot = get_snapshot(snapshots, id);
	if (id == 0 ||current_snapshot == NULL) {
		if (!quiet) {
			printf("no such snapshot\n\n");
		}
		return snapshots;
	}
	int del_found = 0;
	for (int snapshot_index = 0; snapshot_index < total_snapshots; snapshot_index++) {
		if (snapshots[snapshot_index].id == id) {
			del_found = 1;
		}
		if (del_found && snapshot_index != total_snapshots - 1) {
			snapshots[snapshot_index] = snapshots[snapshot_index + 1];
		}
	}
	if (del_found) {
		snapshot original_snapshot;
		memcpy(&original_snapshot, &current_state, sizeof(snapshot));
		current_state = snapshots[total_snapshots - 1];
		for (int current_entry = 0; current_entry < snapshots[total_snapshots - 1].num_entries; current_entry++) {
			entry* free_entry = get_entry(snapshots[total_snapshots - 1].entries[current_entry].key);
			free(free_entry->backward);
			free(free_entry->forward);
			free(free_entry->values);
		}
		free(snapshots[total_snapshots - 1].entries);
		current_state = original_snapshot;
		total_snapshots--;
		snapshots = realloc(snapshots, sizeof(snapshot) * total_snapshots);
	}
	if (!quiet) {
		printf("ok\n\n");
	}
	return snapshots;
}

void command_checkout(int id, snapshot* snapshots, int quiet) {
	snapshot* current_snapshot = get_snapshot(snapshots, id);
	if (!quiet && (id == 0 || current_snapshot == NULL)) {
		printf("no such snapshot\n\n");
		return;
	}
	for (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
		free(current_state.entries[entry_index].forward);
		free(current_state.entries[entry_index].backward);
		free(current_state.entries[entry_index].values);
	}
	free(current_state.entries);
	memcpy(&current_state, current_snapshot, sizeof(snapshot));
	current_state.num_entries = current_snapshot->num_entries;
	current_state.entries = (entry*)malloc(sizeof(entry) * current_snapshot->num_entries);
	memcpy(current_state.entries, current_snapshot->entries, sizeof(entry) * current_snapshot->num_entries);
	for (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
		memcpy(&current_state.entries[entry_index], &current_snapshot->entries[entry_index], sizeof(entry));
		current_state.entries[entry_index].values = (element*)malloc(sizeof(element) * current_snapshot->entries[entry_index].length);
		memcpy(current_state.entries[entry_index].values, current_snapshot->entries[entry_index].values, sizeof(element) * current_state.entries[entry_index].length);
		current_state.entries[entry_index].forward = (entry*)malloc(sizeof(entry) * current_snapshot->entries[entry_index].forward_size);
		memcpy(current_state.entries[entry_index].forward, current_snapshot->entries[entry_index].forward, sizeof(entry) * current_snapshot->entries[entry_index].forward_size);
		current_state.entries[entry_index].backward = (entry*)malloc(sizeof(entry) * current_state.entries[entry_index].backward_size);
		memcpy(current_state.entries[entry_index].backward, current_snapshot->entries[entry_index].backward, sizeof(entry) * current_snapshot->entries[entry_index].backward_size);
		for (int element_index = 0; element_index < current_snapshot->entries[entry_index].length; element_index++) {
			memcpy(&current_state.entries[entry_index].values[element_index], &current_snapshot->entries[entry_index].values[element_index], sizeof(element));
			if (current_snapshot->entries[entry_index].values[element_index].type == ENTRY) {
				current_state.entries[entry_index].values[element_index].entry = get_entry(current_snapshot->entries[entry_index].values[element_index].key);
				memcpy(&current_state.entries[entry_index].values[element_index].key, &current_snapshot->entries[entry_index].values[element_index].key, MAX_KEY);
			}
		}
	}
	if (!quiet) {
		printf("ok\n\n");
	}
}

snapshot* command_rollback(int id, snapshot* snapshots) {
	snapshot* current_snapshot = get_snapshot(snapshots, id);
	if (id == 0 || current_snapshot == NULL) {
		printf("no such snapshot\n\n");
		return snapshots;
	}
	command_checkout(id, snapshots, 1);
	for (int snapshot_index = id + 1; snapshot_index <= snapshot_counter; snapshot_index++) {
		snapshots = command_drop(snapshot_index, snapshots, 1);
	}
	printf("ok\n\n");
	return snapshots;
}


snapshot* command_snapshot(snapshot* snapshots) {
	snapshots = realloc(snapshots, sizeof(snapshot)*(total_snapshots + 1));
	snapshot* new_snapshot = &snapshots[total_snapshots];
	memcpy(new_snapshot, &current_state, sizeof(snapshot));
	new_snapshot->num_entries = current_state.num_entries;
	new_snapshot->entries = (entry*)malloc(sizeof(entry) * current_state.num_entries);
	memcpy(new_snapshot->entries, current_state.entries, sizeof(entry) * current_state.num_entries);
	for (int entry_index = 0; entry_index < new_snapshot->num_entries; entry_index++) {
		memcpy(&new_snapshot->entries[entry_index], &current_state.entries[entry_index], sizeof(entry));
		new_snapshot->entries[entry_index].values = (element*)malloc(sizeof(element) * current_state.entries[entry_index].length);
		memcpy(new_snapshot->entries[entry_index].values, current_state.entries[entry_index].values, sizeof(element) * new_snapshot->entries[entry_index].length);
		new_snapshot->entries[entry_index].forward = (entry*)malloc(sizeof(entry) * current_state.entries[entry_index].forward_size);
		memcpy(new_snapshot->entries[entry_index].forward, current_state.entries[entry_index].forward, sizeof(entry) * current_state.entries[entry_index].forward_size);
		new_snapshot->entries[entry_index].backward = (entry*)malloc(sizeof(entry) * new_snapshot->entries[entry_index].backward_size);
		memcpy(new_snapshot->entries[entry_index].backward, current_state.entries[entry_index].backward, sizeof(entry) * current_state.entries[entry_index].backward_size);
		for (int element_index = 0; element_index < current_state.entries[entry_index].length; element_index++) {
			memcpy(&new_snapshot->entries[entry_index].values[element_index], &current_state.entries[entry_index].values[element_index], sizeof(element));
			if (current_state.entries[entry_index].values[element_index].type == ENTRY) {
				// new_snapshot->entries[entry_index].values[element_index].entry = malloc(sizeof(entry*));
				new_snapshot->entries[entry_index].values[element_index].entry = get_entry(current_state.entries[entry_index].values[element_index].entry->key);
				memcpy(&new_snapshot->entries[entry_index].values[element_index].key, &current_state.entries[entry_index].values[element_index].key, MAX_KEY);
			}
		}
	}
	new_snapshot->id = ++snapshot_counter;
	++total_snapshots;
	printf("saved as snapshot %d\n\n", new_snapshot->id);
	return snapshots;
}

int recursive_min(entry* current_entry, int min) {
	for (int element = 0; element < current_entry->length; element++) {
		if (current_entry->values[element].type == INTEGER) {
			if (current_entry->values[element].value < min) {
				min = current_entry->values[element].value;
			}
		} else {
			min = recursive_min(current_entry->values[element].entry, min);
		}
	}
	return min;
}

void command_min(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		int min = recursive_min(current_entry, INT_MAX);
		if (min == INT_MAX) {
			printf("no integer values\n\n");
		} else {
			printf("%d\n\n", min);
		}
		return;
	}
	printf("no such entry\n\n");
}

int recursive_max(entry* current_entry, int max) {
	for (int element = 0; element < current_entry->length; element++) {
		if (current_entry->values[element].type == INTEGER) {
			if (current_entry->values[element].value > max) {
				max = current_entry->values[element].value;
			}
		} else {
			max = recursive_max(current_entry->values[element].entry, max);
		}
	}
	return max;
}

void command_max(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		int max = recursive_max(current_entry, INT_MIN);
		if (max == INT_MIN) {
			printf("no integer values\n\n");
		} else {
			printf("%d\n\n", max);
		}
		return;
	}
	printf("No such entry\n\n");
}

int recursive_sum(entry* current_entry, int sum) {
	for (int element = 0; element < current_entry->length; element++) {
		if (current_entry->values[element].type == INTEGER) {
			sum += current_entry->values[element].value;
		} else {
			sum = recursive_sum(current_entry->values[element].entry, sum);
		}
	}
	return sum;
}

void command_sum(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		printf("%d\n\n", recursive_sum(current_entry, 0));
		return;
	}
	printf("No such entry\n\n");
}

int recursive_len(char* key) {
	entry* current_entry = get_entry(key);
	int size = 0;
	if (current_entry != NULL) {
		if (current_entry->forward_size == 0) {
			return current_entry->length;
		}
		for (int forw_entry = 0; forw_entry < current_entry->forward_size; forw_entry++) {
			size += recursive_len(current_entry->forward[forw_entry].key);
		}
		size += current_entry->length - current_entry->forward_size;
		return size;
	}
	return size;
}

void command_len(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		printf("%d\n\n", recursive_len(key));
		return;
	}
	printf("No such entry\n\n");
}

void command_rev(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	element* new_val = malloc(sizeof(element)*current_entry->length);
	for (int element = 0; element < current_entry->length; element++) {
		new_val[element] = current_entry->values[current_entry->length - element - 1];
	}
	free(current_entry->values);
	current_entry->values = new_val;
	printf("ok\n\n");
}

void command_uniq(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	for (int check_entry = 0; check_entry < current_entry->length; check_entry++) {
		if (current_entry->values[check_entry].type != INTEGER) {
			printf("simple entry only\n\n");
			return;
		}
	}
	int counter = 0;
	for (int element = 0; element < current_entry->length; element++) {
		if (!(element != 0 && current_entry->values[element].value == current_entry->values[element - 1].value)) {
			current_entry->values[counter++] = current_entry->values[element];
		}
	}
	current_entry->values = realloc(current_entry->values, sizeof(element) * (counter));
	current_entry->length = counter;
	printf("ok\n\n");
}

void command_sort(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	int element, cmp_value, prev_element;
	for (element = 1; element < current_entry->length; element++)
	{
		cmp_value = current_entry->values[element].value;
		prev_element = element - 1;
		while (prev_element >= 0 && current_entry->values[prev_element].value > cmp_value)
		{
			current_entry->values[prev_element + 1].value = current_entry->values[prev_element].value;
			prev_element = prev_element - 1;
		}
	current_entry->values[prev_element + 1].value = cmp_value;
	}
	printf("ok\n\n");
}

void lex_sort(char** array) {
	int length = 0;
	while (array[length++] != NULL) {}
	length--;

	while (length > 0) {
		char* min_key = malloc(MAX_KEY);
		memcpy(min_key, array[0], MAX_KEY);
		for (int key_index = 0; key_index < length; key_index++) {
			if (array[key_index][0] < min_key[0]) {
				memcpy(min_key, array[key_index], MAX_KEY);
			}
		}
		int del_found = 0;
		for (int key_index = 0; key_index < length; key_index++) {
			if (strcmp(array[key_index], min_key) == 0) {
				del_found = 1;
			}
			if (del_found && key_index != length - 1) {
				memcpy(array[key_index], array[key_index + 1], MAX_KEY);
			}
		}
		if (del_found) {
			free(array[length - 1]);
			array = realloc(array, sizeof(char*) * length - 1);
		}
		printf("%s", min_key);
		if (length != 1) {
			printf(", ");
		}
		length--;
		free(min_key);
	}
	free(array);
}

char** recurse_forward(entry* current_entry, char** array, int length) {
	if (current_entry->forward_size == 0) {
		return array;
	}
	for (int forw_index = current_entry->forward_size - 1; forw_index >= 0; forw_index--) {
		int valid = 1;
		for (int index = 0; index < length; index++) {
			if (strcmp(current_entry->forward[forw_index].key, array[index]) == 0) {
				valid = 0;
			}
		}
		if (valid) {
			array = (char**)realloc(array, sizeof(char*) * (length + 2));
			array[length] = (char*)malloc(MAX_KEY);
			memcpy(array[length++], current_entry->forward[forw_index].key, MAX_KEY);
			array[length] = NULL;
		}
		entry* for_entry = get_entry(current_entry->forward[forw_index].key);
		array = recurse_forward(for_entry, array, length);
	}
	return array;
}

void command_forward(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	if (current_entry->forward_size == 0) {
		printf("nil\n\n");
		return;
	}
	char** array = (char**)malloc(0);
	array = recurse_forward(current_entry, array, 0);
	lex_sort(array);
	printf("\n\n");
}

char** recurse_backward(entry* current_entry, char** array, int length) {
	if (current_entry->backward_size == 0) {
		return array;
	}
	for (int back_index = current_entry->backward_size - 1; back_index >= 0; back_index--) {
		int valid = 1;
		for (int index = 0; index < length; index++) {
			if (strcmp(current_entry->backward[back_index].key, array[index]) == 0) {
				valid = 0;
			}
		}
		if (valid) {
			array = (char**)realloc(array, sizeof(char*) * (length + 2));
			array[length] = (char*)malloc(MAX_KEY);
			memcpy(array[length++], current_entry->backward[back_index].key, MAX_KEY);
			array[length] = NULL;
		}
		entry* back_entry = get_entry(current_entry->backward[back_index].key);
		array = recurse_backward(back_entry, array, length);
	}
	return array;
}

void command_backward(char* key) {
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	if (current_entry->backward_size == 0) {
		printf("nil\n\n");
		return;
	}
	char** array = (char**)malloc(0);
	array = recurse_backward(current_entry, array, 0);
	lex_sort(array);
	printf("\n\n");
}


void command_type(char* key) {
	entry* current_entry = get_entry(key);
	for (int current_element = 0; current_element < current_entry->length; current_element++) {
		if (current_entry->values[current_element].type == ENTRY) {
			printf("general\n\n");
			return;
		}
	}
	printf("simple\n\n");
}

int main(void) {

	char line[MAX_LINE];
	char *token,*input;

	snapshot* snapshots = (snapshot*) malloc(0);
	current_state.entries = NULL;
	current_state.num_entries = 0;
	while (true) {
		printf("> ");

		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			command_bye(snapshots);
			return 0;
		}

		input = line;
		char **arg_array = (char**) malloc(sizeof(char**));
		int array_length = 0;
		while ((token = strsep(&input, " ")) != NULL) {
			token = strsep(&token, "\n");
			arg_array = (char**)realloc(arg_array, (array_length+1)*sizeof(char**));
			arg_array[array_length] = token;
			array_length++;
		}

		char *arg = arg_array[0];
		if (strcasecmp("BYE", arg) == 0) {
			free(arg_array);
			command_bye(snapshots);
		} else if (strcasecmp("HELP", arg) == 0) {
			command_help();
		} else if (strcasecmp("LIST", arg) == 0) {
				char *arg = arg_array[1];
				if (strcasecmp("KEYS", arg) == 0) {
					command_list_keys();
				} else if (strcasecmp("ENTRIES", arg) == 0) {
					command_list_entries();
				} else if (strcasecmp("SNAPSHOTS", arg) == 0) {
					command_list_snapshots(snapshots);
				} else {
					continue;
				}
		} else if (strcasecmp("GET", arg) == 0) {
			command_get(arg_array[1]);
		} else if (strcasecmp("DEL", arg) == 0) {
			command_del(arg_array[1], 0);
		} else if (strcasecmp("PURGE", arg) == 0) {
			command_purge(arg_array[1], snapshots);
		} else if (strcasecmp("SET", arg) == 0) {
			command_set(arg_array, array_length);
		} else if (strcasecmp("PUSH", arg) == 0) {
			command_push(arg_array, array_length);
		} else if (strcasecmp("APPEND", arg) == 0) {
			command_append(arg_array, array_length);
		} else if (strcasecmp("PICK", arg) == 0) {
			command_pick(arg_array[1],strtol(arg_array[2], NULL, 10));
		} else if (strcasecmp("PLUCK", arg) == 0 && array_length == 3) {
			command_pluck(arg_array[1],strtol(arg_array[2], NULL, 10));
		} else if (strcasecmp("POP", arg) == 0) {
			command_pop(arg_array[1]);
		} else if (strcasecmp("DROP", arg) == 0) {
			snapshots = command_drop(strtol(arg_array[1], NULL, 10), snapshots, 0);
		} else if (strcasecmp("ROLLBACK", arg) == 0) {
			snapshots = command_rollback(strtol(arg_array[1], NULL, 10), snapshots);
		} else if (strcasecmp("CHECKOUT", arg) == 0) {
			command_checkout(strtol(arg_array[1], NULL, 10), snapshots, 0);
		} else if (strcasecmp("SNAPSHOT", arg) == 0) {
			snapshots = command_snapshot(snapshots);
		} else if (strcasecmp("MIN", arg) == 0) {
			command_min(arg_array[1]);
		} else if (strcasecmp("MAX", arg) == 0) {
			command_max(arg_array[1]);
		} else if (strcasecmp("SUM", arg) == 0) {
			command_sum(arg_array[1]);
		} else if (strcasecmp("LEN", arg) == 0) {
			command_len(arg_array[1]);
		} else if (strcasecmp("REV", arg) == 0) {
			command_rev(arg_array[1]);
		} else if (strcasecmp("UNIQ", arg) == 0) {
			command_uniq(arg_array[1]);
		} else if (strcasecmp("SORT", arg) == 0) {
			command_sort(arg_array[1]);
		} else if (strcasecmp("FORWARD", arg) == 0) {
			command_forward(arg_array[1]);
		} else if (strcasecmp("BACKWARD", arg) == 0) {
			command_backward(arg_array[1]);
		} else if (strcasecmp("TYPE", arg) == 0 && array_length == 2) {
			command_type(arg_array[1]);
		}
		free(arg_array);
	}
	return 0;
}
