#ifndef SOCKET_CHAT_UTILS
#define SOCKET_CHAT_UTILS

#include "json-c/json.h"
#include "json-c/arraylist.h"
#include <stdarg.h>
#include <stdbool.h>

typedef long t_id;


typedef bool (*t_comparator)(void *element, void *data);

bool ptr_comparator(void* el, void* data);
 
typedef struct s_array_of_ids
{
    t_id* array;
    size_t capacity;
    size_t length;
} t_array_of_ids;

typedef t_array_of_ids array_of_ids;

array_of_ids* array_of_ids_new2(size_t capacity);
void array_of_ids_free(array_of_ids* list);
void array_of_ids_add(array_of_ids* list, t_id id);
bool array_of_ids_remove(array_of_ids *ids, t_id id);
t_id array_of_ids_get(array_of_ids *list, size_t index);
bool array_of_ids_contains(array_of_ids *ids, t_id id);
size_t array_of_ids_length(array_of_ids* ids);
bool array_of_ids_remove_at(array_of_ids *ids, size_t index);
array_of_ids* json_to_array_of_ids(json_object* json);

array_list *array_list_copy(array_list *arr_list);
void array_list_del_idx_no_free(array_list* list, size_t index, size_t count);
array_list_free_fn *array_list_disable_free(array_list *arr_list);
void array_list_add_all(array_list *dest, array_list *source);
void array_list_clear(array_list *list);
array_list *array_list_find_all(array_list *list, t_comparator comparator, void *data);
void* array_list_find_first(array_list* list, t_comparator comparator, void* data);
#endif
