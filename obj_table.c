#include "obj_table.h"

// Simple HashTable implementation
// Using LinearProbing for simplicity / performance

Obj_Table * init_obj_table(Obj_Table_Init_Config * obj_table_init_config) {

	Obj_Table * tab = (Obj_Table *) malloc(sizeof(Obj_Table));

	// set number of elements to size passed in
	uint64_t min_size = obj_table_init_config -> min_size;
	tab -> size = min_size;
	tab -> min_size = min_size;
	tab -> max_size = obj_table_init_config -> max_size;
	tab -> load_factor = obj_table_init_config -> load_factor;
	tab -> shrink_factor = obj_table_init_config -> shrink_factor;
	// initially 0 objects in table
	tab -> cnt = 0;

	// init table with no references to objects (all Null)
	Obj ** table = (Obj **) malloc(min_size * sizeof(Obj *));
	for (uint64_t i = 0; i < min_size; i++){
		table[i] = NULL;
	}
	tab -> table = table;

	// TODO:
	//	- look into mutex attributes (not sure if NULL is what we want)
	pthread_mutex_init(&(tab -> table_lock), NULL);

	return tab;
}

void destroy_table(Obj_Table * obj_table){
	fprintf(stderr, "Destroy Table: Unimplemented Error\n");
}

// Mixing hash function
// Taken from "https://github.com/shenwei356/uint64-hash-bench?tab=readme-ov-file"
// Credit: Thomas Wang
uint64_t hash_func(uint64_t key){
	key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8);
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4);
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

int resize_table(Obj_Table * obj_table, uint64_t new_size) {

	if (obj_table == NULL){
		fprintf(stderr, "Error in resize_table, object table is null\n");
		return -1;
	}

	// ensure that new size can fit all objects
	uint64_t cnt = obj_table -> cnt;
	if (cnt > new_size){
		return -1;
	}

	Obj ** old_table = obj_table -> table;
	uint64_t old_size = obj_table -> size;

	// create new table that will replace old one
	Obj ** new_table = (Obj **) malloc(new_size * sizeof(Obj *));

	// re-hash all the objects into new table
	uint64_t hash_ind, table_ind;
	for (uint64_t i = 0; i < old_size; i++){
		hash_ind = hash_func(old_table[i] -> obj_id) % new_size;
		// do open addressing insert
		for (uint64_t j = hash_ind; j < hash_ind + new_size; j++){
			table_ind = j % new_size;
			// found empty slot to insert
			if (new_table[table_ind] == NULL) {
				new_table[table_ind] = old_table[i];
				// inserted new object so break from inner loop
				break;
			}
		}
	}

	// free the old table and reset to new one
	free(old_table);
	obj_table -> table = new_table;
	return 0;
}


// return -1 upon error
int insert_obj(Obj_Table * obj_table, Obj * obj) {

	if (obj_table == NULL){
		fprintf(stderr, "Error in insert_obj, object table is null\n");
		return -1;
	}

	if (obj == NULL){
		fprintf(stderr, "Error in insert_obj, object is null\n");
		return -1;
	}

	int ret;
	uint64_t size = obj_table -> size;

	
	uint64_t cnt = obj_table -> cnt;
	Obj ** table = obj_table -> table;

	// should only happen when cnt = max_size
	if (cnt == size){
		return -1;
	}

	uint64_t hash_ind = hash_func(obj -> obj_id) % size;
	uint64_t table_ind;
	// doing the Linear Probing
	// worst case O(size) insert time
	for (uint64_t i = hash_ind; i < hash_ind + size; i++){
		table_ind = i % size;
		// there is a free slot to insert
		if (table[table_ind] == NULL){
			table[table_ind] = obj;
			break;
		}
	}

	cnt += 1;
	obj_table -> cnt = cnt;

	// check if we exceed load and are below max cap
	// if so, grow
	float load_factor = obj_table -> load_factor;
	uint64_t max_size = obj_table -> max_size;
	// make sure types are correct when multiplying uint64_t by float
	uint64_t load_cap = (uint64_t) (size * load_factor);
	if ((size < max_size) && (cnt > load_cap)){
		size = (uint64_t) (size * round(1 / load_factor));
		if (size > max_size){
			size = max_size;
		}
		ret = resize_table(obj_table, size);
		// check if there was an error growing table
		if (ret == -1){
			return -1;
		}
	}
	return 0;
}


Obj * find_obj(Obj_Table * obj_table, uint64_t obj_id_key){

	if (obj_table == NULL){
		fprintf(stderr, "Error in find_obj, object table is null\n");
		return NULL;
	}

	uint64_t size = obj_table -> size;
	uint64_t hash_ind = hash_func(obj_id_key) % size;
	Obj ** table = obj_table -> table;
	uint64_t table_ind;
	// do linear scan
	for (uint64_t i = hash_ind; i < hash_ind + size; i++){
		table_ind = i % size;
		// check if we found object
		if ((table[table_ind] != NULL) && (table[table_ind] -> obj_id == obj_id_key)){
			return table[table_ind];
		}
	}
	// didn't find object
	return NULL;
}

// remove from table and return pointer to obj (can be used later for destroying)
Obj * remove_obj(Obj_Table * obj_table, uint64_t obj_id_key) {
	
	if (obj_table == NULL){
		fprintf(stderr, "Error in remove_obj, object table is null\n");
		return NULL;
	}

	int ret;

	uint64_t size = obj_table -> size;
	uint64_t cnt = obj_table -> cnt;
	Obj ** table = obj_table -> table;

	uint64_t hash_ind = hash_func(obj_id_key) % size;

	// orig set to null in case obj not in table 
	// in which case we want to return NULL
	Obj * obj = NULL;
	uint64_t table_ind;
	// do linear scan
	for (uint64_t i = hash_ind; i < hash_ind + size; i++){
		table_ind = i % size;
		// check if we found object, remember its contents and make room in table
		if ((table[table_ind] != NULL) && (table[table_ind] -> obj_id == obj_id_key)){
			// set obj to be the object removed so we can return it
			obj = table[table_ind];
			// remove reference from table
			table[table_ind] = NULL;
			cnt -= 1;
			obj_table -> cnt = cnt;
			// found obj so break
			break;
		}
	}

	// check if we should shrink
	float shrink_factor = obj_table -> shrink_factor;
	uint64_t min_size = obj_table -> min_size;
	// make sure types are correct when multiplying uint64_t by float
	uint64_t shrink_cap = (uint64_t) (size * shrink_factor);
	if ((size > min_size) && (cnt < shrink_cap)) {
		size = (uint64_t) (size * (1 - shrink_factor));
		if (size < min_size){
			size = min_size;
		}
		ret = resize_table(obj_table, size);
		// check if there was an error growing table
		if (ret == -1){
			return NULL;
		}
	}

	// if found is pointer to obj, otherwise null
	return obj;
}