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
#include <ctype.h> //Check +++
#include <limits.h>

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

entry get_entry(char* key, snapshot* snapshots, int snapshot_number) {
	for (int entry_num = 0; entry_num < snapshots[snapshot_number].num_entries; entry_num++) {
		entry current_entry = snapshots[snapshot_number].entries[entry_num];
		if (strcmp(current_entry.key, key) == 0) {
			return current_entry;
		}
	}
	entry null_entry;
	null_entry.length = -1;
	return null_entry;
}

void command_bye(snapshot* snapshots) {
	for (int current_snapshot = 0; current_snapshot < sizeof(snapshots)/sizeof(snapshot*); current_snapshot++) {
		free(snapshots[current_snapshot].prev);
		free(snapshots[current_snapshot].next);
		for (int current_entry = 0; current_entry < snapshots[current_snapshot].num_entries; current_entry++) {
			for (int current_element = 0; current_element < snapshots[current_snapshot].entries[current_entry].length; current_element++) {
				if (snapshots[current_snapshot].entries[current_entry].values[current_element].type == ENTRY) {
							free(snapshots[current_snapshot].entries[current_entry].values[current_element].entry);
							snapshots[current_snapshot].entries[current_entry].values[current_element].entry = NULL;
				}
			}
			free(snapshots[current_snapshot].entries[current_entry].values);
			for (int back_index = 0; back_index < snapshots[current_snapshot].entries[current_entry].backward_size; back_index++) {
				if (snapshots[current_snapshot].entries[current_entry].backward[back_index] != NULL) {
					free(snapshots[current_snapshot].entries[current_entry].backward[back_index]);
					snapshots[current_snapshot].entries[current_entry].backward[back_index] = NULL;
				}
			}
			free(snapshots[current_snapshot].entries[current_entry].backward);
			free(snapshots[current_snapshot].entries[current_entry].forward);
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

void command_list_keys(entry *entries, int num_entries) {
	if (num_entries == 0) {
		printf("no keys\n\n");
		return;
	}
	for (int entry = num_entries-1; entry >= 0; entry--) {
		printf("%s\n", entries[entry].key);
	}
	printf("\n");
}

void command_list_entries(entry *entries, int num_entries) {
	if (num_entries == 0) {
		printf("no entries\n\n");
		return;
	}
	for (int current_entry = num_entries - 1; current_entry >= 0; current_entry--) {
		int element_limit = entries[current_entry].length;
		printf("%s [", entries[current_entry].key);
		fflush(stdout);
		for (int element = 0; element < element_limit; element++) {
			if (entries[current_entry].values[element].type == INTEGER) {
				int value = entries[current_entry].values[element].value;
				printf("%d", value);
			} else {
				printf("%s", entries[current_entry].values[element].entry->key);
			}
			if (element != element_limit-1) {
				printf(" ");
			}
		}
		printf("]\n");
	}
	printf("\n");
	fflush(stdout);
}

void command_list_snapshots(snapshot *snapshots, int num_snapshots) {
	if (num_snapshots == 0) {
		printf("no snapshots\n\n");
		return;
	}
	snapshot current_snapshot = snapshots[0];
	while (current_snapshot.next != NULL) {
		printf("%d", current_snapshot.id);
		current_snapshot = *current_snapshot.next;
	}
	printf("\n\n");
}

void command_get(char* key, snapshot* snapshots, int snapshot_number) {// +++ rework to return the entry, maybe a helper func?
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length != -1) {
		printf("[");
		for (int element_num = 0; element_num < current_entry.length; element_num++) {
			element current_element = current_entry.values[element_num];
			if (current_element.type == INTEGER) {
				printf("%d", current_element.value);
			} else {
				printf("%s", current_element.entry->key);
			}
			if (element_num != current_entry.length-1) {
				printf(" ");
			}
		}
		printf("]\n\n");
		return;
	}
	printf("no such key\n\n");
}

void command_del(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length != -1) {
		int del_found = 0;
		for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries - 1; entry_index++) { //Case where the element is the last in the array is covered as default
			// int element_del_found = 0;
			// for (int element_index = 0; element_index < snapshots[snapshot_number].entries[entry_index].length; element_index++) {
			// 	element el = snapshots[snapshot_number].entries[entry_index].values[element_index];
			// 	if (el.type == ENTRY && strcmp(el.entry->key, current_entry.key) == 0) {
			// 		element_del_found = 1;
			// 	}
			// 	if (element_del_found) {
			// 		snapshots[snapshot_number].entries[entry_index].values[element_index] = snapshots[snapshot_number].entries[entry_index].values[element_index+1];
			// 	}
			// }
			// if (element_del_found) {
			// 	snapshots[snapshot_number].entries[entry_index].length--;
			// 	snapshots[snapshot_number].entries[entry_index].values = realloc(snapshots[snapshot_number].entries[entry_index].values, snapshots[snapshot_number].entries[entry_index].length*sizeof(element));
			// }
			if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
				del_found = 1;
			}
			if (del_found) {
				snapshots[snapshot_number].entries[entry_index] = snapshots[snapshot_number].entries[entry_index+1];
			}
		}
		snapshots[snapshot_number].num_entries--;
		entry* new_entries = realloc(snapshots[snapshot_number].entries, snapshots[snapshot_number].num_entries * sizeof(entry));
		snapshots[snapshot_number].entries = new_entries;
		for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries - 1; entry_index++) { //Case where the element is the last in the array is covered as default
			for (int element_index = 0; element_index < snapshots[snapshot_number].entries[entry_index].length; element_index++) {
				int del_found = 0;
				for (int forward_index = 0; forward_index < snapshots[snapshot_number].entries[entry_index].forward_size; forward_index++) {
					del_found = 0;
					if (snapshots[snapshot_number].entries[entry_index].forward[forward_index] == &current_entry) {
						del_found = 1;
					}
					if (del_found) {
						snapshots[snapshot_number].entries[entry_index].forward[forward_index] = snapshots[snapshot_number].entries[entry_index].forward[forward_index+1];
					}
				}
				if (del_found) {
					snapshots[snapshot_number].entries[entry_index].forward_size--;
					printf("%ld",snapshots[snapshot_number].entries[entry_index].forward_size);
					fflush(stdout);
					snapshots[snapshot_number].entries[entry_index].forward = realloc(snapshots[snapshot_number].entries[entry_index].forward, sizeof(entry*) * snapshots[snapshot_number].entries[entry_index].forward_size);
				}
			}
		}
		for (int current_element = 0; current_element < current_entry.length; current_element++) {
			if (current_entry.values[current_element].type == ENTRY) {
				free(current_entry.values[current_element].entry->backward);
				free(current_entry.values[current_element].entry->forward);
				free(current_entry.values[current_element].entry);
			}
		}
		free(current_entry.values);
		if (snapshots[snapshot_number].entries == NULL && snapshots[snapshot_number].num_entries != 0) {
			perror("Realloc failed");
			command_bye(snapshots);
		}
		printf("ok\n\n");
		return;
	}
	printf("no such key\n\n");
}

void command_purge(char* key) {
	//
}

void command_set(char** array, int array_length, snapshot* snapshots, int snapshot_number) {
	for (int check_arg = 2; check_arg < array_length; check_arg++) {
		if (!((char)array[check_arg][0] >= '0' && (char)array[check_arg][0] <= '9')) {
			if (strcmp(array[check_arg], array[1]) == 0) {
				printf("not permitted\n\n");
				return;
			}
			entry test_entry = get_entry(array[check_arg], snapshots, snapshot_number);
			if (test_entry.length == -1) {
				printf("no such key\n\n");
				return;
			}
		}
	}
	entry current_entry = get_entry(array[1], snapshots, snapshot_number);
	int mem_index;
	if (current_entry.length == -1) {
		snapshots[snapshot_number].num_entries++;
		current_entry.forward_size = 0;
		current_entry.backward_size = 0;
		current_entry.forward = NULL;
		current_entry.backward = NULL;
		current_entry.values = NULL;
		snapshots[snapshot_number].entries = realloc(snapshots[snapshot_number].entries, sizeof(entry)*(snapshots[snapshot_number].num_entries));
		memcpy(&snapshots[snapshot_number].entries[snapshots[snapshot_number].num_entries-1], &current_entry, sizeof(entry));
		mem_index = snapshots[snapshot_number].num_entries-1;
	} else {
		for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
			if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
				mem_index = entry_index;
				break;
			}
		}
		free(current_entry.values);
	}
	element *values = malloc(sizeof(element) * (array_length - 2));
	current_entry.length = array_length-2;

	memcpy(current_entry.key, array[1], MAX_KEY);
	for (int arg = 2; arg < array_length; arg++) {
		element new_element;
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			new_element.type = INTEGER;
			new_element.value = (int)strtol(array[arg], NULL, 10);
			memcpy(&values[arg-2], &new_element, sizeof(element));
		} else {
			entry test_entry = get_entry(array[arg], snapshots, snapshot_number);
			int mem_test_index = -1;
			for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
				if (strcmp(snapshots[snapshot_number].entries[entry_index].key, test_entry.key) == 0) {
					mem_test_index = entry_index;
					break;
				}
			}
			test_entry.backward_size++;
			test_entry.backward = realloc(test_entry.backward, sizeof(entry*)*test_entry.backward_size);
			entry* heaped_test_entry = malloc(sizeof(entry));
			memcpy(heaped_test_entry, &current_entry, sizeof(entry));
			test_entry.backward[test_entry.backward_size-1] = heaped_test_entry;
			snapshots[snapshot_number].entries[mem_test_index] = test_entry;

			current_entry.forward_size++;
			current_entry.forward = realloc(current_entry.forward, current_entry.forward_size * sizeof(entry*));
			entry* heaped_entry = malloc(sizeof(entry));
			memcpy(heaped_entry, &test_entry, sizeof(entry));
			new_element.type = ENTRY;
			new_element.entry = heaped_entry;
			current_entry.forward[current_entry.forward_size-1] = heaped_entry;
			memcpy(&values[arg-2], &new_element, sizeof(element));
		}
	}
	current_entry.values = values;
	current_entry.length = array_length-2;
	snapshots[snapshot_number].entries[mem_index] = current_entry;
	printf("ok\n\n");
}

int value_checks(char** array, int array_length, entry current_entry) { //+++ not necessary??
	if (current_entry.length == -1) {
		printf("no such key\n\n");
		return 1;
	}
	for (int current_element = 2; current_element < array_length; current_element++) {
		if (strcmp(array[current_element], current_entry.key) == 0) {
			printf("not permitted\n\n");
			return 1;
		}
	}
	return 0;
}

void command_push(char** array, int array_length, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(array[1], snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such key\n\n");
		return;
	}
	if (value_checks(array, array_length, current_entry)) {
		return;
	}
	int mem_index;
	for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
		if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
			mem_index = entry_index;
			break;
		}
	}
	current_entry.values = realloc(current_entry.values, (current_entry.length + array_length - 2) * sizeof(element));
	for (int original_elem = 0; original_elem < current_entry.length; original_elem++) {
		current_entry.values[array_length - 2 + original_elem] = current_entry.values[original_elem];
	}
	for (int arg = 0; arg < array_length - 2; arg++) {
		element new_element;
		if (array[array_length - arg - 2][0] >= '0' && array[array_length - arg - 1][0] <= '9') {
			new_element.type = INTEGER;
			new_element.value = (int)strtol(array[array_length - arg - 1], NULL, 10);
			memcpy(&current_entry.values[arg], &new_element, sizeof(element));
		} else {
			entry test_entry = get_entry(array[array_length - arg - 2], snapshots, snapshot_number);
			// test_entry.backward_size++;
			// test_entry.backward = realloc(test_entry.backward, sizeof(entry*)*test_entry.backward_size);
			// entry* test_entry_ptr = &current_entry;
			// memcpy(test_entry.backward[0], test_entry_ptr, sizeof(entry)); +++ fix backwards

			current_entry.forward_size++;
			current_entry.forward = realloc(current_entry.forward, current_entry.forward_size * sizeof(entry*));
			new_element.type = ENTRY;
			entry* heaped_entry = malloc(sizeof(entry));
			memcpy(heaped_entry, &test_entry, sizeof(entry));
			new_element.entry = heaped_entry;
			current_entry.forward[current_entry.forward_size-1] = heaped_entry;
			memcpy(&current_entry.values[arg], &new_element, sizeof(element));
		}
	}
	current_entry.length += array_length - 2;
	memcpy(&snapshots[snapshot_number].entries[mem_index], &current_entry, sizeof(entry));
	printf("ok\n\n");
}


void command_append(char** array, int array_length, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(array[1], snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such entry\n\n");
		return;
	}
	if (value_checks(array, array_length, current_entry)) {
		return;
	}
	int mem_index;
	for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
		if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
			mem_index = entry_index;
			break;
		}
	}

	////// +++ FIX THIS SHIT YOU SPOON
	current_entry.values = realloc(current_entry.values, (current_entry.length + array_length - 2) * sizeof(element));
	for (int arg = current_entry.length; arg < current_entry.length + array_length - 2; arg++) {
		element new_element;
		printf("%d", arg);
		if (array[arg-current_entry.length][0] >= '0' && array[arg-current_entry.length][0] <= '9') {
			new_element.type = INTEGER;
			new_element.value = (int)strtol(array[arg-current_entry.length], NULL, 10);
			memcpy(&current_entry.values[arg - 1], &new_element, sizeof(element));
		} else {
			entry test_entry = get_entry(array[arg], snapshots, snapshot_number);
			int mem_test_index = -1;
			for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
				if (strcmp(snapshots[snapshot_number].entries[entry_index].key, test_entry.key) == 0) {
					mem_test_index = entry_index;
					break;
				}
			}
			test_entry.backward_size++;
			test_entry.backward = realloc(test_entry.backward, sizeof(entry*)*test_entry.backward_size);
			entry* heaped_test_entry = malloc(sizeof(entry));
			memcpy(heaped_test_entry, &current_entry, sizeof(entry));
			test_entry.backward[test_entry.backward_size-1] = heaped_test_entry;
			snapshots[snapshot_number].entries[mem_test_index] = test_entry;

			current_entry.forward_size++;
			current_entry.forward = realloc(current_entry.forward, current_entry.forward_size * sizeof(entry*));
			new_element.type = ENTRY;
			entry* heaped_entry = malloc(sizeof(entry));
			memcpy(heaped_entry, &test_entry, sizeof(entry));
			new_element.entry = heaped_entry;
			current_entry.forward[current_entry.forward_size-1] = heaped_entry;
			memcpy(&current_entry.values[arg - 1], &new_element, sizeof(element));
		}
	}
	current_entry.length += array_length;
	memcpy(&snapshots[snapshot_number].entries[mem_index], &current_entry, sizeof(entry));
	printf("ok\n\n");
}

void command_pick(char* key, int index, snapshot* snapshots, int snapshot_number) {
	index--;
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such key\n\n");
		return;
	}
	if (index >= current_entry.length || index < 0) {
		printf("index out of range\n\n");
		return;
	}
	if (current_entry.values[index].type == INTEGER) {
		printf("%d\n\n", current_entry.values[index].value);
		return;
	}
	printf("%s\n\n", current_entry.values[index].entry->key);
}

void command_pluck(char* key, int index, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	index--;
	if (current_entry.length <= index) {
		printf("index out of range\n\n");
		return;
	}
	if (current_entry.length == -1) {
		printf("no such key\n\n");
		return;
	}
	int mem_index = -1;
	for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
		if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
			mem_index = entry_index;
			break;
		}
	}

	if (current_entry.values[index].type == INTEGER) {
		printf("%d\n\n", current_entry.values[index].value);
	} else {
		printf("%s\n\n", current_entry.values[index].entry->key);
		free(current_entry.values[index].entry->values);
		free(current_entry.values[index].entry->forward);
		free(current_entry.values[index].entry->backward);
	}
	for (int element_index = index; element_index < current_entry.length-1; element_index++) {
		current_entry.values[element_index] = current_entry.values[element_index + 1];
	}
	current_entry.length--;
	current_entry.values = realloc(current_entry.values, sizeof(element) * current_entry.length);
	memcpy(&snapshots[snapshot_number].entries[mem_index], &current_entry, sizeof(entry));
}

void command_pop(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length <= 0) {
		printf("nil\n\n");
		return;
	}
	command_pluck(key, 1, snapshots, snapshot_number);
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

int command_snapshot(snapshot* snapshots, int snapshot_number) {
	snapshot new_snapshot;
	new_snapshot.id = snapshot_number+1;
	new_snapshot.next = NULL;
	new_snapshot.prev = malloc(sizeof(snapshot));
	new_snapshot.entries = malloc(sizeof(entry) * snapshots[snapshot_number].num_entries);
	new_snapshot.prev = &snapshots[snapshot_number];
	memcpy(&new_snapshot, &snapshots[snapshot_number], sizeof(snapshot));
	memcpy(new_snapshot.entries, snapshots[snapshot_number].entries, sizeof(entry) * new_snapshot.num_entries);
	for (int entry_index = 0; entry_index < new_snapshot.num_entries; entry_index++) {
		memcpy(&new_snapshot.entries[entry_index], &new_snapshot.entries[entry_index], sizeof(entry));
		for (int element_index = 0; element_index < new_snapshot.entries[entry_index].length; element_index++) {
			memcpy(&new_snapshot.entries[entry_index].values[element_index], &new_snapshot.entries[entry_index].values[element_index], sizeof(element));
			if (new_snapshot.entries[entry_index].values[element_index].type == ENTRY) {
				memcpy(new_snapshot.entries[entry_index].values[element_index].entry, new_snapshot.entries[entry_index].values[element_index].entry, sizeof(entry));
			}
		}
	}
	snapshots = realloc(snapshots, sizeof(snapshot)*(snapshot_number + 1));
	memcpy(&snapshots[snapshot_number], &new_snapshot, sizeof(snapshot));
	return snapshot_number + 1;
}

int recursive_min(entry current_entry, int min) {
	for (int element = 0; element < current_entry.length; element++) {
		if (current_entry.values[element].type == INTEGER) {
			if (current_entry.values[element].value < min) {
				min = current_entry.values[element].value;
			}
		} else {
			min = recursive_min(*current_entry.values[element].entry, min);
		}
	}
	return min;
}

void command_min(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length != -1) {
		int min = recursive_min(current_entry, INT_MAX);
		if (min == INT_MAX) {
			printf("no integer values\n\n");
		} else {
			printf("%d\n\n", min);
		}
		return;
	}
	printf("No such entry\n\n");
}

int recursive_max(entry current_entry, int max) {
	for (int element = 0; element < current_entry.length; element++) {
		if (current_entry.values[element].type == INTEGER) {
			if (current_entry.values[element].value > max) {
				max = current_entry.values[element].value;
			}
		} else {
			max = recursive_max(*current_entry.values[element].entry, max);
		}
	}
	return max;
}

void command_max(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length != -1) {
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

int recursive_sum(entry current_entry, int sum) {
	for (int element = 0; element < current_entry.length; element++) {
		if (current_entry.values[element].type == INTEGER) {
			sum += current_entry.values[element].value;
		} else {
			sum = recursive_sum(*current_entry.values[element].entry, sum);
		}
	}
	return sum;
}

void command_sum(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length != -1) {
		printf("%d\n\n", recursive_sum(current_entry, 0));
		return;
	}
	printf("No such entry\n\n");
}

void command_len(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length != -1) {
		printf("%ld\n", current_entry.length);
		return;
	}
	printf("No such entry\n\n");
}

void command_rev(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	int mem_index = -1;
	for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
		if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
			mem_index = entry_index;
			break;
		}
	}
	if (mem_index == -1) {
		printf("no such entry\n\n");
		return;
	}
	element* new_val = malloc(sizeof(element)*current_entry.length);
	for (int element = 0; element < current_entry.length; element++) {
		new_val[element] = current_entry.values[current_entry.length - element - 1];
	}
	free(snapshots[snapshot_number].entries[mem_index].values);
	snapshots[snapshot_number].entries[mem_index].values = new_val;
	printf("ok\n\n");
}

void command_uniq(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such key\n\n");
		return;
	}
	int mem_index = -1;
	for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
		if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
			mem_index = entry_index;
			break;
		}
	}

	element* new_val = malloc(sizeof(element) * current_entry.length);
	int counter = 0;
	int valid = 1;
	for (int element = 0; element < current_entry.length; element++) {
		if (current_entry.values[element].type == INTEGER) {
			if (element != 0 && current_entry.values[element].value == current_entry.values[element - 1].value) {
				valid = 0;
			} else {
				valid = 1;
			}
		} else {
			printf("simple entry only\n\n");
			free(new_val);
			return;
		}
		if (valid) {
			new_val[counter++] = current_entry.values[element];
		}
	}
	new_val = realloc(new_val, sizeof(element) * (counter));
	current_entry.values = realloc(current_entry.values, sizeof(element) * (counter));
	memcpy(current_entry.values, new_val, sizeof(element) * (counter));
	free(new_val);
	current_entry.length = counter;
	memcpy(&snapshots[snapshot_number].entries[mem_index], &current_entry, sizeof(entry));
	printf("ok\n\n");
}

void command_sort(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such key\n\n");
		return;
	}
	int mem_index = -1;
	for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
		if (strcmp(snapshots[snapshot_number].entries[entry_index].key, current_entry.key) == 0) {
			mem_index = entry_index;
			break;
		}
	}

	int element, cmp_value, prev_element;
	for (element = 1; element < current_entry.length; element++)
	{
		cmp_value = current_entry.values[element].value;
		prev_element = element - 1;

		while (prev_element >= 0 && current_entry.values[prev_element].value > cmp_value)
		{
			current_entry.values[prev_element + 1].value = current_entry.values[prev_element].value;
			prev_element = prev_element - 1;
		}
	current_entry.values[prev_element + 1].value = cmp_value;
	}
	memcpy(&snapshots[snapshot_number].entries[mem_index], &current_entry, sizeof(entry));
	printf("ok\n\n");
}

void recurse_forward(entry current_entry) {
	if (current_entry.forward_size == 0) {
		return;
	}
	for (int forw_index = 0; forw_index < current_entry.forward_size; forw_index++) {
		printf("%s", current_entry.forward[forw_index]->key);
		if (!(forw_index == current_entry.forward_size - 1 && current_entry.forward[forw_index]->forward_size == 0)) {
			printf(", ");
		}
		recurse_forward(*current_entry.forward[forw_index]);
	}
}

void command_forward(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such entry\n\n");
		return;
	}
	if (current_entry.forward_size == 0) {
		printf("nil\n\n");
		return;
	}
	recurse_forward(current_entry);
	printf("\n\n");
}
//+++ fix these
void recurse_backward(entry current_entry) {
	if (current_entry.backward_size == 0) {
		return;
	}
	for (int back_index = 0; back_index < current_entry.backward_size; back_index++) {
		printf("%s", current_entry.backward[back_index]->key);
		if (!(back_index == current_entry.backward_size - 1 && current_entry.backward[back_index]->backward_size == 0)) {
			printf(", ");
		}
		recurse_backward(*current_entry.backward[back_index]);
	}
}

void command_backward(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry.length == -1) {
		printf("no such entry\n\n");
		return;
	}
	if (current_entry.backward_size == 0) {
		printf("nil\n\n");
		return;
	}
	recurse_backward(current_entry);
	printf("\n\n");
}

void command_type(char* key, snapshot* snapshots, int snapshot_number) {
	entry current_entry = get_entry(key, snapshots, snapshot_number);
	for (int current_element = 0; current_element < current_entry.length; current_element++) {
		if (current_entry.values[current_element].type == ENTRY) {
			printf("general\n\n");
			return;
		}
	}
	printf("simple\n\n");
}

int main(void) {

	char line[MAX_LINE];
	char *token,*input;

	int snapshot_number = 0;
	snapshot *snapshots = (snapshot*) malloc(sizeof(snapshot));
	snapshot current_snapshot = {snapshot_number, NULL, 0, NULL, NULL};
	snapshots[0] = current_snapshot;
	snapshots[0].num_entries = 0;
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
				char *arg = arg_array[1];//+++
				if (strcasecmp("KEYS", arg) == 0) {
					command_list_keys(snapshots[snapshot_number].entries, snapshots[snapshot_number].num_entries);
				} else if (strcasecmp("ENTRIES", arg) == 0) {
					command_list_entries(snapshots[snapshot_number].entries, snapshots[snapshot_number].num_entries);
				} else if (strcasecmp("SNAPSHOTS", arg) == 0) {
					command_list_snapshots(snapshots, snapshots[snapshot_number].num_entries);
				} else {
					continue;
				}
		} else if (strcasecmp("GET", arg) == 0) {
			command_get(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("DEL", arg) == 0) {
			command_del(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("PURGE", arg) == 0) {
			command_purge(arg_array[1]);
		} else if (strcasecmp("SET", arg) == 0) {
			command_set(arg_array, array_length, snapshots, snapshot_number);
		} else if (strcasecmp("PUSH", arg) == 0) {
			command_push(arg_array, array_length, snapshots, snapshot_number);
		} else if (strcasecmp("APPEND", arg) == 0) {
			command_append(arg_array, array_length, snapshots, snapshot_number);
		} else if (strcasecmp("PICK", arg) == 0) {
			command_pick(arg_array[1],strtol(arg_array[2], NULL, 10), snapshots, snapshot_number);
		} else if (strcasecmp("PLUCK", arg) == 0 && array_length == 3) {
			command_pluck(arg_array[1],strtol(arg_array[2], NULL, 10), snapshots, snapshot_number);
		} else if (strcasecmp("POP", arg) == 0) {
			command_pop(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("DROP", arg) == 0) {
			command_drop(arg_array[1]);
		} else if (strcasecmp("ROLLBACK", arg) == 0) {
			command_rollback(arg_array[1]);
		} else if (strcasecmp("CHECKOUT", arg) == 0) {
			command_checkout(arg_array[1]);
		} else if (strcasecmp("SNAPSHOT", arg) == 0) {
			snapshot_number = command_snapshot(snapshots, snapshot_number);
		} else if (strcasecmp("MIN", arg) == 0) {
			command_min(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("MAX", arg) == 0) {
			command_max(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("SUM", arg) == 0) {
			command_sum(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("LEN", arg) == 0) {
			command_len(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("REV", arg) == 0) {
			command_rev(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("UNIQ", arg) == 0) {
			command_uniq(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("SORT", arg) == 0) {
			command_sort(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("FORWARD", arg) == 0) {
			command_forward(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("BACKWARD", arg) == 0) {
			command_backward(arg_array[1], snapshots, snapshot_number);
		} else if (strcasecmp("TYPE", arg) == 0 && array_length == 2) {
			command_type(arg_array[1], snapshots, snapshot_number);
		}
		free(arg_array);
	}
	return 0;
}
