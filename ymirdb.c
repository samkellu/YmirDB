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

entry* get_entry(char* key, snapshot* snapshots, int snapshot_number) {
	for (int entry_num = 0; entry_num < snapshots[snapshot_number].num_entries; entry_num++) {
		if (strcmp(snapshots[snapshot_number].entries[entry_num].key, key) == 0) {
			return &snapshots[snapshot_number].entries[entry_num];
		}
	}
	return NULL;
}

snapshot* get_snapshot(snapshot* snapshots, int snapshot_number) {
	return &snapshots[snapshot_number];
}

void command_bye(snapshot* snapshots) {
	for (int current_snapshot = 0; current_snapshot < sizeof(snapshots)/sizeof(snapshot*); current_snapshot++) {
		free(snapshots[current_snapshot].prev);
		free(snapshots[current_snapshot].next);
		for (int current_entry = 0; current_entry < snapshots[current_snapshot].num_entries; current_entry++) {
			entry* free_entry = get_entry(snapshots[current_snapshot].entries[current_entry].key, snapshots, current_snapshot);
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

void command_list_entries(snapshot* snapshots, int snapshot_number) {
	if (snapshots[snapshot_number].num_entries == 0) {
		printf("no entries\n\n");
		return;
	}
	for (int current_entry = snapshots[snapshot_number].num_entries - 1; current_entry >= 0; current_entry--) {
		entry read_entry = snapshots[snapshot_number].entries[current_entry];
		printf("%s [", read_entry.key);
		fflush(stdout);
		for (int element = 0; element < read_entry.length; element++) {
			if (read_entry.values[element].type == INTEGER) {
				int value = read_entry.values[element].value;
				printf("%d", value);
			} else {
				printf("%s", read_entry.values[element].entry->key);
			}
			if (element != read_entry.length - 1) {
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
	for (int snap_index = 0; snap_index < num_snapshots; snap_index++) {
		printf("%d", snapshots[snap_index].id);
	}
	printf("\n\n");
}

void command_get(char* key, snapshot* snapshots, int snapshot_number) {// +++ rework to return the entry, maybe a helper func?
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

void command_del(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry != NULL) {
		for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
			entry* test_entry = get_entry(snapshots[snapshot_number].entries[entry_index].key, snapshots, snapshot_number);
			int del_found = 0;
			for (int forward_index = 0; forward_index < test_entry->forward_size; forward_index++) {
				if (&test_entry->forward[forward_index] == current_entry) {
					del_found = 1;
				}
				if (del_found) {
					if (forward_index != test_entry->forward_size - 1) {
						test_entry->forward[forward_index] = test_entry->forward[forward_index+1];
					}
				}
			}
			if (del_found) {
				test_entry->forward_size--;
				test_entry->forward = realloc(test_entry->forward, sizeof(entry) * test_entry->forward_size);
			}
			del_found = 0;
			for (int backward_index = 0; backward_index < test_entry->backward_size; backward_index++) {
				if (&test_entry->backward[backward_index] == current_entry) {
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
		int del_found = 0;
		for  (int entry_index = 0; entry_index < snapshots[snapshot_number].num_entries; entry_index++) { //Case where the element is the last in the array is covered as default
			entry* test_entry = get_entry(snapshots[snapshot_number].entries[entry_index].key, snapshots, snapshot_number);
			int element_del_found = 0;
			for (int element_index = 0; element_index < test_entry->length; element_index++) {
				element el = test_entry->values[element_index];
				if (el.type == ENTRY && strcmp(el.entry->key, current_entry->key) == 0) {

					element_del_found = 1;
				}
				if (element_del_found) {
					if (entry_index != snapshots[snapshot_number].num_entries - 1) {
						test_entry->values[element_index] = test_entry->values[element_index+1];
					}
				}
			}
			if (element_del_found) {
				test_entry->length--;
				test_entry->values = realloc(test_entry->values, test_entry->length*sizeof(element));
			}
			if (strcmp(test_entry->key, current_entry->key) == 0) {
				del_found = 1;
			}
			if (del_found) {
				if (entry_index != snapshots[snapshot_number].num_entries - 1) {
					snapshots[snapshot_number].entries[entry_index] = snapshots[snapshot_number].entries[entry_index+1];
				}
			}
		}
		snapshots[snapshot_number].num_entries--;
		snapshots[snapshot_number].entries = realloc(snapshots[snapshot_number].entries, snapshots[snapshot_number].num_entries * sizeof(entry));
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
			entry* test_entry = get_entry(array[check_arg], snapshots, snapshot_number);
			if (test_entry == NULL) {
				printf("no such key\n\n");
				return;
			}
		}
	}
	entry* current_entry = get_entry(array[1], snapshots, snapshot_number);
	snapshot* current_snapshot = get_snapshot(snapshots, snapshot_number);
	if (current_entry == NULL) {
		//update all forward and backwards to point to new entry locations
		current_snapshot->num_entries++;
		current_snapshot->entries = realloc(current_snapshot->entries, sizeof(entry) * (current_snapshot->num_entries));
		current_entry = &current_snapshot->entries[current_snapshot->num_entries-1];
		current_entry->forward_size = 0;
		current_entry->backward_size = 0;
		current_entry->forward = NULL;
		current_entry->backward = NULL;
		current_entry->values = NULL;
	}
	current_entry->values = realloc(current_entry->values, sizeof(element) * (array_length - 2));
	current_entry->length = array_length-2;

	memcpy(current_entry->key, array[1], MAX_KEY);
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[arg-2];
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		} else {
			entry* test_entry = get_entry(array[arg], snapshots, snapshot_number);
			test_entry->backward_size++;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry)*test_entry->backward_size);
			test_entry->backward[test_entry->backward_size-1] = *current_entry;

			current_entry->forward_size++;
			current_entry->forward = realloc(current_entry->forward, current_entry->forward_size * sizeof(entry));
			new_element->type = ENTRY;
			new_element->entry = test_entry;
			current_entry->forward[current_entry->forward_size-1] = *test_entry;
		}
	}
	printf("ok\n\n");
}

int value_checks(char** array, int array_length, entry* current_entry) { //+++ not necessary??
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

void command_push(char** array, int array_length, snapshot* snapshots, int snapshot_number) {
	for (int arg = 2; arg < array_length; arg++) {
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			continue;
		}
		entry* current_entry = get_entry(array[arg], snapshots, snapshot_number);
		if (current_entry == NULL) {
			printf("no such key\n\n");
			return;
		}
	}
	entry* current_entry = get_entry(array[1], snapshots, snapshot_number);
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
			entry* test_entry = get_entry(array[arg], snapshots, snapshot_number);
			test_entry->backward_size++;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry)*test_entry->backward_size);
			test_entry->backward[test_entry->backward_size-1] = *current_entry;

			current_entry->forward_size++;
			current_entry->forward = realloc(current_entry->forward, current_entry->forward_size * sizeof(entry));
			new_element->type = ENTRY;
			new_element->entry = test_entry;
			current_entry->forward[current_entry->forward_size-1] = *test_entry;
		}
	}
	printf("ok\n\n");
}

void command_append(char** array, int array_length, snapshot* snapshots, int snapshot_number) {
	for (int arg = 2; arg < array_length; arg++) {
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			continue;
		}
		entry* current_entry = get_entry(array[arg], snapshots, snapshot_number);
		if (current_entry == NULL) {
			printf("no such key\n\n");
			return;
		}
	}
	entry* current_entry = get_entry(array[1], snapshots, snapshot_number);
	if (value_checks(array, array_length, current_entry)) {
		return;
	}

	int old_len = current_entry->length;
	current_entry->length += array_length - 2;
	current_entry->values = realloc(current_entry->values, current_entry->length * sizeof(element));
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[old_len + (arg-2)];
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		} else {
			entry* test_entry = get_entry(array[arg], snapshots, snapshot_number);
			test_entry->backward_size++;
			test_entry->backward = realloc(test_entry->backward, sizeof(entry)*test_entry->backward_size);
			test_entry->backward[test_entry->backward_size-1] = *current_entry;

			current_entry->forward_size++;
			current_entry->forward = realloc(current_entry->forward, current_entry->forward_size * sizeof(entry));
			new_element->type = ENTRY;
			new_element->entry = test_entry;
			current_entry->forward[current_entry->forward_size-1] = *test_entry;
		}
	}
	printf("ok\n\n");
}

void command_pick(char* key, int index, snapshot* snapshots, int snapshot_number) {
	index--;
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

void command_pluck(char* key, int index, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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
	}
	for (int element_index = index; element_index < current_entry->length; element_index++) {
		if (element_index != current_entry->length - 1) {
			current_entry->values[element_index] = current_entry->values[element_index + 1];
		}
	}
	current_entry->length--;
	current_entry->values = realloc(current_entry->values, sizeof(element) * current_entry->length);
}

void command_pop(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	if (current_entry->length <= 0) {
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

snapshot* command_snapshot(snapshot* snapshots, int snapshot_number, int total_snapshots) {
	snapshots = realloc(snapshots, sizeof(snapshot)*(total_snapshots + 1));
	snapshot new_snapshot = snapshots[total_snapshots];
	memcpy(&new_snapshot, &snapshots[snapshot_number], sizeof(snapshot));
	new_snapshot.id = snapshot_number+1;
	new_snapshot.next = NULL;
	new_snapshot.prev = &snapshots[snapshot_number];
	new_snapshot.num_entries = snapshots[snapshot_number].num_entries;
	new_snapshot.entries = malloc(sizeof(entry) * new_snapshot.num_entries);
	for (int entry_index = 0; entry_index < new_snapshot.num_entries; entry_index++) {
		memcpy(&new_snapshot.entries[entry_index], &snapshots[snapshot_number].entries[entry_index], sizeof(entry));
		for (int element_index = 0; element_index < new_snapshot.entries[entry_index].length; element_index++) {
			memcpy(&new_snapshot.entries[entry_index].values[element_index], &snapshots[snapshot_number].entries[entry_index].values[element_index], sizeof(element));
			if (new_snapshot.entries[entry_index].values[element_index].type == ENTRY) {
				memcpy(new_snapshot.entries[entry_index].values[element_index].entry, snapshots[snapshot_number].entries[entry_index].values[element_index].entry, sizeof(entry));
			}
		}
		memcpy(new_snapshot.entries[entry_index].values, snapshots[snapshot_number].entries[entry_index].values, sizeof(entry) * new_snapshot.entries[entry_index].length);

	}
	printf("saved as snapshot %d\n\n", ++total_snapshots);
	snapshot_number = total_snapshots;
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

void command_min(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry != NULL) {
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

void command_max(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

void command_sum(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry != NULL) {
		printf("%d\n\n", recursive_sum(current_entry, 0));
		return;
	}
	printf("No such entry\n\n");
}

void command_len(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry != NULL) {
		int size = 0;
		for (int forw_entry = 0; forw_entry < current_entry->forward_size; forw_entry++) {
			size += current_entry->forward[forw_entry].length;
		}
		size += current_entry->length - current_entry->forward_size;
		printf("%d\n\n", size);
		return;
	}
	printf("No such entry\n\n");
}

void command_rev(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

void command_uniq(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

void command_sort(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

void recurse_forward(entry* current_entry, entry* end) {
	// printf("%ld",current_entry->forward_size);
	// fflush(stdout);
	if (current_entry->forward_size == 0) {
		return;
	}
	for (int forw_index = current_entry->forward_size - 1; forw_index >= 0; forw_index--) {
		recurse_forward(&current_entry->forward[forw_index], end);
		printf("%s", current_entry->forward[forw_index].key);
		if (!(forw_index == 0 && strcmp(current_entry->key, end->key) == 0)) {
			printf(", ");
		}
	}
}

void command_forward(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	if (current_entry->forward_size == 0) {
		printf("nil\n\n");
		return;
	}
	recurse_forward(current_entry, current_entry);
	printf("\n\n");
}

void recurse_backward(entry* current_entry, entry* end, snapshot* snapshots, int snapshot_number) {
	if (current_entry->backward_size == 0) {
		return;
	}
	for (int back_index = current_entry->backward_size - 1; back_index >= 0; back_index--) {
		entry* back_entry = get_entry(current_entry->backward[back_index].key, snapshots, snapshot_number);
		printf("%s", current_entry->backward[back_index].key);
		if (!(back_index == 0 && strcmp(current_entry->key, end->key) == 0)) {
			printf(", ");
		}
		recurse_backward(back_entry, end, snapshots, snapshot_number);
	}
}

void command_backward(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	if (current_entry->backward_size == 0) {
		printf("nil\n\n");
		return;
	}

	recurse_backward(current_entry, current_entry, snapshots, snapshot_number);
	printf("\n\n");
}

void command_type(char* key, snapshot* snapshots, int snapshot_number) {
	entry* current_entry = get_entry(key, snapshots, snapshot_number);
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

	int snapshot_number = 0;
	int total_snapshots = 0;
	snapshot* snapshots = (snapshot*) malloc(sizeof(snapshot));
	snapshot current_snapshot = snapshots[0];
	memset(&snapshots[0], 0, sizeof(snapshot));
	current_snapshot.id = snapshot_number + 1;
	current_snapshot.entries = NULL;
	current_snapshot.num_entries = 0;
	current_snapshot.next = NULL;
	current_snapshot.prev = NULL;
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
					command_list_entries(snapshots, snapshot_number);
				} else if (strcasecmp("SNAPSHOTS", arg) == 0) {
					command_list_snapshots(snapshots, total_snapshots);
					total_snapshots++;
					snapshot_number = total_snapshots - 1;
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
			snapshots = command_snapshot(snapshots, snapshot_number, total_snapshots);
			total_snapshots++;
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
