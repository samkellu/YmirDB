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
#include <limits.h>

#include "ymirdb.h"

// global variables to store snapshot information easily, aswell as the current state of the program
int total_snapshots = 0;
int snapshot_counter = 0;
snapshot current_state;

//Returns the entry in the current state with the given key, or NULL if the key doesnt exist in the current state
entry* get_entry(char* key) {
	for (int entry_num = 0; entry_num < current_state.num_entries; entry_num++) {
		if (strcmp(current_state.entries[entry_num].key, key) == 0) {
			return &current_state.entries[entry_num];
		}
	}
	return NULL;
}

//Returns the snapshot with the given id, or NULL if the id doesnt exist in the array of snapshots
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

//Ends the program, freeing all memory from each snapshot, its entries, forward/backward refs, values etc. aswell
//as the current state.
void command_bye(snapshot* snapshots) {
	for (int current_entry = 0; current_entry < current_state.num_entries; current_entry++) {
		free(current_state.entries[current_entry].backward);
		free(current_state.entries[current_entry].forward);
		free(current_state.entries[current_entry].values);
	}
	free(current_state.entries);
	//Traverses each snapshot and its entries, freeing each allocated memory space
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

//Prints a list of all commands and their syntax
void command_help() {
	printf("%s\n", HELP);
}

//Lists all keys present in the current state in order of creation
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

//Lists all entries in the current state in order of creation
void command_list_entries() {
	if (current_state.num_entries == 0) {
		printf("no entries\n\n");
		return;
	}
	//Traverses all entries in the current state, then their values, printing each element's value/key as it goes
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
}

//Lists all valid snapshots that have been recorded
void command_list_snapshots(snapshot *snapshots) {
	if (total_snapshots == 0) {
		printf("no snapshots\n\n");
		return;
	}
	//Traverses the array of snapshots and prints their id
	for (int snap_index = snapshot_counter; snap_index >= 0; snap_index--) {
		snapshot* current_snapshot = get_snapshot(snapshots, snap_index);
		if (current_snapshot != NULL) {
			printf("%d\n", current_snapshot->id);
		}
	}
	printf("\n");
}

//Prints the values stored in the entry with the given key in the current program state
void command_get(char* key) {
	//Gets the entry with the given key if it exists, if not returns
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		printf("[");
		//Traverses all elements of the entry's values array, printing their value/key
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

//Deletes an entry with a given key from the current program state
void command_del(char* key, int quiet) {
	//Gets the entry with the given key if it exists, if not returns
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		//Checks if the deletion will invalidate any backward/forward references
		if (current_entry->backward_size > 0) {
			if (!quiet) {
				printf("not permitted\n\n");
			}
			return;
		}
		//Traverses all entries in the current_state, then their backward references, deleting the entry from their
		//backward array if it is present.
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
		//Traverses all entries in the current_state to find the entry to be deleted, then removes it
		for  (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
			entry* test_entry = get_entry(current_state.entries[entry_index].key);
			if (strcmp(test_entry->key, key) == 0) {
				del_found = 1;
				//Frees all memory related to the given entry in the current state
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
		//reallocates the array of entries to reflect the change in size
		current_state.num_entries--;
		current_state.entries = realloc(current_state.entries, current_state.num_entries * sizeof(entry));
		//Reassigns pointers lost by the reallocation
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

//Deletes a given entry from all snapshots if valid
void command_purge(char* key, snapshot* snapshots) {
	//Gets the entry to be deleted and checks it exists
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		if (current_entry->backward_size > 0) {
			printf("not permitted\n\n");
			return;
		}
	}
	//saves a copy of the current state for referencing later
	snapshot original_snapshot;
	memcpy(&original_snapshot, &current_state, sizeof(snapshot));
	//Checks if the deletion is valid in all snapshots
	for (int snapshot_index = 0; snapshot_index < total_snapshots; snapshot_index++) {
		//changes the current state to perform the checks
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
	//reverts the current state
	current_state = original_snapshot;
	//deletes the entry from the current state and saves the state again
	command_del(key, 1);
	memcpy(&original_snapshot, &current_state, sizeof(snapshot));
	//deletes the entry from all snapshots
	for (int snapshot_index = 0; snapshot_index < total_snapshots; snapshot_index++) {
		//changes the current state to delete the entry in the given snapshot
		current_state = snapshots[snapshot_index];
		current_state.entries = snapshots[snapshot_index].entries;
		command_del(key, 1);
		snapshots[snapshot_index].entries = current_state.entries;
		snapshots[snapshot_index].num_entries = current_state.num_entries;
	}
	//reverts to the original state
	current_state = original_snapshot;
	printf("ok\n\n");
}

//sets a new or existing entry in the database, with a given set of values
void command_set(char** array, int array_length) {
	//ensures for all values of the new entry that the new entry is not a forward entry of itself
	for (int check_arg = 2; check_arg < array_length; check_arg++) {
		if (!(((char)array[check_arg][0] >= '0' && (char)array[check_arg][0] <= '9') || array[check_arg][0] == '-')) {
			if (strcmp(array[check_arg], array[1]) == 0) {
				printf("not permitted\n\n");
				return;
			}
			//ensures that the given entry exists before it is added to the array
			entry* test_entry = get_entry(array[check_arg]);
			if (test_entry == NULL) {
				printf("no such key\n\n");
				return;
			}
		}
	}
	//Checks if the entry is new or exists already
	entry* current_entry = get_entry(array[1]);
	if (current_entry == NULL) {
		//if the entry is new, allocates space for its entries and reassigns all pointers lost in this operation
		current_state.entries = realloc(current_state.entries, sizeof(entry) * (++current_state.num_entries));
		for (int entry_index = 0; entry_index < current_state.num_entries - 1; entry_index++) {
			for (int element_index = 0; element_index < current_state.entries[entry_index].length; element_index++) {
				if (current_state.entries[entry_index].values[element_index].type == ENTRY) {
					current_state.entries[entry_index].values[element_index].entry = get_entry(current_state.entries[entry_index].values[element_index].key);
				}
			}
		}
		//initialises the entry's parameters
		current_entry = &current_state.entries[current_state.num_entries-1];
		current_entry->forward_size = 0;
		current_entry->backward_size = 0;
		current_entry->forward = NULL;
		current_entry->backward = NULL;
		current_entry->values = NULL;
	}
	//deletes the entry from every other entry's backward ref array
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
	//frees the entry's forward ref array to make way for new references
	current_entry->forward_size = 0;
	free(current_entry->forward);
	//adds the entry to the forward ref array of each element in the new entry's backward ref array
	for (int backward_index = 0; backward_index < current_entry->backward_size; backward_index++) {
		entry* test_entry = get_entry(current_entry->backward[backward_index].key);
		for (int forward_index = 0; forward_index < test_entry->forward_size; forward_index++) {
			if (strcmp(test_entry->forward[forward_index].key, current_entry->key) == 0) {
				test_entry->forward[forward_index] = *current_entry;
			}
		}
	}
	//initialises parameters and memory for value storage in the new entry
	current_entry->values = realloc(current_entry->values, sizeof(element) * (array_length - 2));
	current_entry->length = array_length-2;
	current_entry->forward = NULL;
	memcpy(current_entry->key, array[1], MAX_KEY);
	//creates an element for each argument given and adds it to the new entry's value array
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[arg-2];
		//Case where the given argument is an integer
		if ((array[arg][0] >= '0' && array[arg][0] <= '9') || array[arg][0] == '-') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		//Case for when the given argument is an entry
		} else {
			//reallocates and reassigns ref arrays for the new entry, and the referenced entry
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

//Checks given arguments to the push and append functions
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

//Pushes the given array of elements to the start of the given entry
void command_push(char** array, int array_length) {
	for (int arg = 2; arg < array_length; arg++) {
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			continue;
		}
		//Checks that entries in the argument array exist
		entry* current_entry = get_entry(array[arg]);
		if (current_entry == NULL) {
			printf("no such key\n\n");
			return;
		}
	}
	entry* current_entry = get_entry(array[1]);
	//Checks given arguments for validity
	if (value_checks(array, array_length, current_entry)) {
		return;
	}
	//reallocates the values array for the given entry
	int old_len = current_entry->length;
	current_entry->length += array_length - 2;
	current_entry->values = realloc(current_entry->values, current_entry->length * sizeof(element));
	//Moves all old elements to their new locations
	for (int original_elem = 0; original_elem < old_len; original_elem++) {
		current_entry->values[current_entry->length - 1 - original_elem] = current_entry->values[old_len - original_elem - 1];
	}
	if (old_len == 0) {
		old_len = current_entry->length;
	}
	//Places the new elements at the correct locations
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[(array_length - 2) - (arg - 2) - 1];
		//Case where the new element is an integer
		if (array[arg][0] >= '0' && array[arg][0] <= '9') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		//Case where the new element is an entry
		} else {
			//reallocates and reassigns ref arrays for the given entry and the entry being referenced
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

//Appends the given array of elements to the end of a given entry's values
void command_append(char** array, int array_length) {
	for (int arg = 2; arg < array_length; arg++) {
		if ((array[arg][0] >= '0' && array[arg][0] <= '9') || array[arg][0] == '-') {
			continue;
		}
		//Checks that entries in the argument array exist
		entry* current_entry = get_entry(array[arg]);
		if (current_entry == NULL) {
			printf("no such key\n\n");
			return;
		}
	}
	//Checks given arguments for validity
	entry* current_entry = get_entry(array[1]);
	if (value_checks(array, array_length, current_entry)) {
		return;
	}
	//reallocates the values array for the given entry
	int old_len = current_entry->length;
	current_entry->length += array_length - 2;
	current_entry->values = realloc(current_entry->values, current_entry->length * sizeof(element));
	//Places the new elements at the correct locations
	for (int arg = 2; arg < array_length; arg++) {
		element* new_element = &current_entry->values[old_len + (arg-2)];
		//Case where the new element is an integer
		if ((array[arg][0] >= '0' && array[arg][0] <= '9') || array[arg][0] == '-') {
			new_element->type = INTEGER;
			new_element->value = (int)strtol(array[arg], NULL, 10);
		//Case where the new element is an entry
		} else {
			//reallocates and reassigns ref arrays for the given entry and the entry being referenced
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

//Prints the value at a given index in a given entry's value array
void command_pick(char* key, int index) {
	index--;
	//Gets the required entry by key and checks it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	//Checks if the given index is within bounds
	if (index >= current_entry->length || index < 0) {
		printf("index out of range\n\n");
		return;
	}
	//returns the value
	if (current_entry->values[index].type == INTEGER) {
		printf("%d\n\n", current_entry->values[index].value);
		return;
	}
	printf("%s\n\n", current_entry->values[index].entry->key);
}

//Removes a value at a given index in a given entry's values array
void command_pluck(char* key, int index) {
	//gets the entry to be processed by key and checks it exists
	entry* current_entry = get_entry(key);
	index--;
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	//ensures the index is within correct bounds
	if (current_entry->length <= index) {
		printf("index out of range\n\n");
		return;
	}
	//prints the value at the given index
	if (current_entry->values[index].type == INTEGER) {
		printf("%d\n\n", current_entry->values[index].value);
	} else {
		printf("%s\n\n", current_entry->values[index].entry->key);
		int valid = 1;
		//ensures that the entry both exists in the array, but also that only one instance of the entry is removed if multiple exist
		for (int element_index = 0; element_index < current_entry->length; element_index++) {
			if (strcmp(current_entry->values[element_index].key, current_entry->values[index].entry->key) == 0) {
				valid--;
			}
		}
		//Deletes the entry from the current entry's forward ref array
		if (valid == 0) {
			int del_found = 0;
			for (int forward_entry = 0; forward_entry < current_entry->forward_size; forward_entry++) {
				if (strcmp(current_entry->forward[forward_entry].key, current_entry->values[index].entry->key) == 0) {
					del_found = 1;
				}
				if (del_found && forward_entry != current_entry->forward_size - 1) {
					current_entry->forward[forward_entry] = current_entry->forward[forward_entry+1];
				}
			}
			current_entry->forward = realloc(current_entry->forward, sizeof(entry*) * --current_entry->forward_size);
			//Deletes the entry from the backward ref arrays of the entries it referenced
			entry* test_entry = get_entry(current_entry->values[index].entry->key);
			del_found = 0;
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
	//Deletes the element from the current entry's values array
	for (int element_index = index; element_index < current_entry->length; element_index++) {
		if (element_index != current_entry->length - 1) {
			current_entry->values[element_index] = current_entry->values[element_index + 1];
		}
	}
	current_entry->values = realloc(current_entry->values, sizeof(element) * --current_entry->length);
}

//removes the first element from a given entry
void command_pop(char* key) {
	//checks that the entry exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	if (current_entry->length <= 0) {
		printf("nil\n\n");
		return;
	}
	//plucks the first element of the entry
	command_pluck(key, 1);
}

//Deletes a snapshot from the current array of snapshots
snapshot* command_drop(int id, snapshot* snapshots, int quiet) {
	//gets the given snapshot by id and checks that it exists
	snapshot* current_snapshot = get_snapshot(snapshots, id);
	if (id == 0 || current_snapshot == NULL) {
		if (!quiet) {
			printf("no such snapshot\n\n");
		}
		return snapshots;
	}
	//deletes the snapshot from the array of snapshots
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
		//saves the current state for later referencing
		snapshot original_snapshot;
		memcpy(&original_snapshot, &current_state, sizeof(snapshot));
		current_state = snapshots[--total_snapshots];
		//frees all memory relating to the given snapshot
		for (int current_entry = 0; current_entry < snapshots[total_snapshots].num_entries; current_entry++) {
			entry* free_entry = get_entry(snapshots[total_snapshots].entries[current_entry].key);
			free(free_entry->backward);
			free(free_entry->forward);
			free(free_entry->values);
		}
		free(snapshots[total_snapshots].entries);
		//reverts to the current state
		current_state = original_snapshot;
		snapshots = realloc(snapshots, sizeof(snapshot) * total_snapshots);
	}
	if (!quiet) {
		printf("ok\n\n");
	}
	return snapshots;
}

//Replaces the current state with a copy of a given snapshot
void command_checkout(int id, snapshot* snapshots, int quiet) {
	//gets the required snapshot and checks that it exists
	snapshot* current_snapshot = get_snapshot(snapshots, id);
	if (!quiet && (id == 0 || current_snapshot == NULL)) {
		printf("no such snapshot\n\n");
		return;
	}
	//frees all memory related to the current state
	for (int entry_index = 0; entry_index < current_state.num_entries; entry_index++) {
		free(current_state.entries[entry_index].forward);
		free(current_state.entries[entry_index].backward);
		free(current_state.entries[entry_index].values);
	}
	free(current_state.entries);
	//Copies all allocated memory related to the given snapshot to the new current state
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

//Reverts the current state to a previous snapshot, deleting all newer snapshots
snapshot* command_rollback(int id, snapshot* snapshots) {
	//gets the required snapshot and ensures it exists
	snapshot* current_snapshot = get_snapshot(snapshots, id);
	if (id == 0 || current_snapshot == NULL) {
		printf("no such snapshot\n\n");
		return snapshots;
	}
	//Sets the current state to be a copy of the target snapshot
	command_checkout(id, snapshots, 1);
	//drops all newer snapshots
	for (int snapshot_index = id + 1; snapshot_index <= snapshot_counter; snapshot_index++) {
		snapshots = command_drop(snapshot_index, snapshots, 1);
	}
	printf("ok\n\n");
	return snapshots;
}

//Creates a copy of the current state and stores it in memory to be referenced later
snapshot* command_snapshot(snapshot* snapshots) {
	//reallocates the current array of snapshots and creates a new snapshot
	snapshots = realloc(snapshots, sizeof(snapshot)*(total_snapshots + 1));
	snapshot* new_snapshot = &snapshots[total_snapshots];
	//copies all memory related to the current state to the new snapshot
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
				new_snapshot->entries[entry_index].values[element_index].entry = get_entry(current_state.entries[entry_index].values[element_index].entry->key);
				memcpy(&new_snapshot->entries[entry_index].values[element_index].key, &current_state.entries[entry_index].values[element_index].key, MAX_KEY);
			}
		}
	}
	//sets the new id of the snapshot and updates global variables relating to snapshots
	new_snapshot->id = ++snapshot_counter;
	++total_snapshots;
	printf("saved as snapshot %d\n\n", new_snapshot->id);
	return snapshots;
}

//Recursively finds the minimum value in an entry and all of its forward refernces
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

//Prints the minimum value in an entry and its forward references
void command_min(char* key) {
	//gets the required entry and checks it exists
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		//finds the minimum value
		int min = recursive_min(current_entry, INT_MAX);
		//checks if the returned value has changed (indicating that at least one integer has been found)
		if (min == INT_MAX) {
			printf("no integer values\n\n");
		} else {
			printf("%d\n\n", min);
		}
		return;
	}
	printf("no such entry\n\n");
}

//recursivly finds the maximum element in a given entry, and all of its forward references
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

//Prints the maximum value contained in a given entry and its forward references
void command_max(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		//finds the maximum value
		int max = recursive_max(current_entry, INT_MIN);
		//checks if the returned value has changed (indicating that at least one integer has been found)
		if (max == INT_MIN) {
			printf("no integer values\n\n");
		} else {
			printf("%d\n\n", max);
		}
		return;
	}
	printf("no such entry\n\n");
}

//recursively calculates the sum of an entry (and its forward references) values
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
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		//prints the sum of all values
		printf("%d\n\n", recursive_sum(current_entry, 0));
		return;
	}
	printf("no such entry\n\n");
}

//Recursively finds the total length of a given entry and its forward references
int recursive_len(char* key) {
	//gets the required entry and checks if it exists
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

//prints the total length of an entry and its forward references
void command_len(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry != NULL) {
		//prints the total length
		printf("%d\n\n", recursive_len(key));
		return;
	}
	printf("no such entry\n\n");
}

//reverses a given array
void command_rev(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	//swaps elements at each end of the array until the whole array has reversed
	element* new_val = malloc(sizeof(element)*current_entry->length);
	for (int element = 0; element < current_entry->length; element++) {
		new_val[element] = current_entry->values[current_entry->length - element - 1];
	}
	free(current_entry->values);
	current_entry->values = new_val;
	printf("ok\n\n");
}

//removes adjacend duplicate integer values from an entry's value array
void command_uniq(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	//Checks that all elements in the given entry are integers
	for (int check_entry = 0; check_entry < current_entry->length; check_entry++) {
		if (current_entry->values[check_entry].type != INTEGER) {
			printf("simple entry only\n\n");
			return;
		}
	}
	//removes duplicate adjacent entries
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

//sorts a given entry's value array (simple entry only)
void command_sort(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	//insertion sort on all values in the array
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

//Sorts all elements of a given array lexicographically
void lex_sort(char** array) {
	//gets the length of the array
	int length = 0;
	while (array[length++] != NULL) {}
	length--;
	//finds the minimum key in the current portion of the array, prints it, then deletes it from the array
	while (length > 0) {
		char* min_key = malloc(MAX_KEY);
		memcpy(min_key, array[0], MAX_KEY);
		//finds the minimum value in the current array
		for (int key_index = 0; key_index < length; key_index++) {
			if (array[key_index][0] < min_key[0]) {
				memcpy(min_key, array[key_index], MAX_KEY);
			}
		}
		//deletes the smallest key from the array and prints it
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

//recursively adds the keys of forward entries of a given entry to an array
char** recurse_forward(entry* current_entry, char** array, int length) {
	//checks that the current entry has forward refs
	if (current_entry->forward_size == 0) {
		return array;
	}
	//checks that the current entry's key isnt already present in the array
	for (int forw_index = current_entry->forward_size - 1; forw_index >= 0; forw_index--) {
		int valid = 1;
		for (int index = 0; index < length; index++) {
			if (strcmp(current_entry->forward[forw_index].key, array[index]) == 0) {
				valid = 0;
			}
		}
		if (valid) {
			//adds the current entry's key to the array, with an extra NULL key for sorting
			array = (char**)realloc(array, sizeof(char*) * (length + 2));
			array[length] = (char*)malloc(MAX_KEY);
			memcpy(array[length++], current_entry->forward[forw_index].key, MAX_KEY);
			array[length] = NULL;
		}
		//gets the key's corresponding entry and continues recursion
		entry* for_entry = get_entry(current_entry->forward[forw_index].key);
		array = recurse_forward(for_entry, array, length);
	}
	return array;
}

//Prints all the forward entries of a given entry
void command_forward(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	//checks that the given entry has forward refereces
	if (current_entry->forward_size == 0) {
		printf("nil\n\n");
		return;
	}
	//gets an array of all forward entries, sorts it lexicographically and prints it
	char** array = (char**)malloc(0);
	array = recurse_forward(current_entry, array, 0);
	lex_sort(array);
	printf("\n\n");
}

//recursively adds the keys of backward entries of an entry to an array
char** recurse_backward(entry* current_entry, char** array, int length) {
	//checks that the current entry has backward entries
	if (current_entry->backward_size == 0) {
		return array;
	}
	//checks that the entry is not already present in the array
	for (int back_index = current_entry->backward_size - 1; back_index >= 0; back_index--) {
		int valid = 1;
		for (int index = 0; index < length; index++) {
			if (strcmp(current_entry->backward[back_index].key, array[index]) == 0) {
				valid = 0;
			}
		}
		if (valid) {
			//adds the current entry's key to the array, with an extra NULL key for sorting
			array = (char**)realloc(array, sizeof(char*) * (length + 2));
			array[length] = (char*)malloc(MAX_KEY);
			memcpy(array[length++], current_entry->backward[back_index].key, MAX_KEY);
			array[length] = NULL;
		}
		//gets the key's corresponding entry and continues recursion on it
		entry* back_entry = get_entry(current_entry->backward[back_index].key);
		array = recurse_backward(back_entry, array, length);
	}
	return array;
}

void command_backward(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such entry\n\n");
		return;
	}
	//checks if there are no backward refs
	if (current_entry->backward_size == 0) {
		printf("nil\n\n");
		return;
	}
	//creates an array of the keys of all backward entries, then sorts them lexicographically and prints them
	char** array = (char**)malloc(0);
	array = recurse_backward(current_entry, array, 0);
	lex_sort(array);
	printf("\n\n");
}

//Prints the type of a given entry
void command_type(char* key) {
	//gets the required entry and checks if it exists
	entry* current_entry = get_entry(key);
	if (current_entry == NULL) {
		printf("no such key\n\n");
		return;
	}
	//checks all values, prints general if there are any entries, simple if not
	for (int current_element = 0; current_element < current_entry->length; current_element++) {
		if (current_entry->values[current_element].type == ENTRY) {
			printf("general\n\n");
			return;
		}
	}
	printf("simple\n\n");
}

int main(void) {
	//initialising input variables
	char line[MAX_LINE];
	char *token,*input;
	//initialises the current state and snapshots array
	snapshot* snapshots = (snapshot*) malloc(0);
	current_state.entries = NULL;
	current_state.num_entries = 0;
	//main loop
	while (true) {
		printf("> ");
		//exits if fgets returns NULL
		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			command_bye(snapshots);
			return 0;
		}

		input = line;
		//allocates an array to store the entered arguments
		char **arg_array = (char**) malloc(sizeof(char**));
		int array_length = 0;
		//separates and parses all values given as arguments, adds them to the array of arguments
		while ((token = strsep(&input, " ")) != NULL) {
			token = strsep(&token, "\n");
			arg_array = (char**)realloc(arg_array, (array_length+1)*sizeof(char**));
			arg_array[array_length] = token;
			array_length++;
		}
		//handles the inputs and outputs of each function, given their specific input conditions are met
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
		} else if (strcasecmp("GET", arg) == 0 && array_length == 2) {
			command_get(arg_array[1]);
		} else if (strcasecmp("DEL", arg) == 0 && array_length == 2) {
			command_del(arg_array[1], 0);
		} else if (strcasecmp("PURGE", arg) == 0 && array_length == 2) {
			command_purge(arg_array[1], snapshots);
		} else if (strcasecmp("SET", arg) == 0 && array_length >= 1) {
			command_set(arg_array, array_length);
		} else if (strcasecmp("PUSH", arg) == 0 && array_length >= 1) {
			command_push(arg_array, array_length);
		} else if (strcasecmp("APPEND", arg) == 0 && array_length >= 1) {
			command_append(arg_array, array_length);
		} else if (strcasecmp("PICK", arg) == 0 && array_length == 3) {
			command_pick(arg_array[1],strtol(arg_array[2], NULL, 10));
		} else if (strcasecmp("PLUCK", arg) == 0 && array_length == 3) {
			command_pluck(arg_array[1],strtol(arg_array[2], NULL, 10));
		} else if (strcasecmp("POP", arg) == 0 && array_length == 2) {
			command_pop(arg_array[1]);
		} else if (strcasecmp("DROP", arg) == 0 && array_length == 2) {
			snapshots = command_drop(strtol(arg_array[1], NULL, 10), snapshots, 0);
		} else if (strcasecmp("ROLLBACK", arg) == 0 && array_length == 2) {
			snapshots = command_rollback(strtol(arg_array[1], NULL, 10), snapshots);
		} else if (strcasecmp("CHECKOUT", arg) == 0 && array_length == 2) {
			command_checkout(strtol(arg_array[1], NULL, 10), snapshots, 0);
		} else if (strcasecmp("SNAPSHOT", arg) == 0 && array_length == 1) {
			snapshots = command_snapshot(snapshots);
		} else if (strcasecmp("MIN", arg) == 0 && array_length == 2) {
			command_min(arg_array[1]);
		} else if (strcasecmp("MAX", arg) == 0 && array_length == 2) {
			command_max(arg_array[1]);
		} else if (strcasecmp("SUM", arg) == 0&& array_length == 2) {
			command_sum(arg_array[1]);
		} else if (strcasecmp("LEN", arg) == 0 && array_length == 2) {
			command_len(arg_array[1]);
		} else if (strcasecmp("REV", arg) == 0 && array_length == 2) {
			command_rev(arg_array[1]);
		} else if (strcasecmp("UNIQ", arg) == 0 && array_length == 2) {
			command_uniq(arg_array[1]);
		} else if (strcasecmp("SORT", arg) == 0 && array_length == 2) {
			command_sort(arg_array[1]);
		} else if (strcasecmp("FORWARD", arg) == 0 && array_length == 2) {
			command_forward(arg_array[1]);
		} else if (strcasecmp("BACKWARD", arg) == 0 && array_length == 2) {
			command_backward(arg_array[1]);
		} else if (strcasecmp("TYPE", arg) == 0 && array_length == 2) {
			command_type(arg_array[1]);
		}
		free(arg_array);
	}
	return 0;
}
