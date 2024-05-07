#ifndef OBJ_TABLE_H
#define OBJ_TABLE_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "system_config.h"

// DEFINING INTERFACE FUNCTIONS FOR BUILD PROCESS

typedef struct obj_table_init_config Obj_Table_Init_Config;

// Functions to export:

Obj_Table * init_obj_table(Obj_Table_Init_Config * obj_table_init_config);
void destroy_table(Obj_Table * obj_table);
int insert_obj(Obj_Table * obj_table, Obj * obj);
Obj * find_obj(Obj_Table * obj_table, uint64_t obj_id_key);
Obj * remove_obj(Obj_Table * obj_table, uint64_t obj_id_key);

#endif