#include "chat-socket/array_list_utils.h"
#include "json-c/arraylist.h"
#include <string.h>

#ifdef __MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

static void empty_free_fn(void *ptr)
{
}

bool ptr_comparator(void *el, void *data)
{
    return el == data;
}

array_of_ids *array_of_ids_new2(size_t capacity)
{
    array_of_ids *ids = (array_of_ids *)malloc(sizeof(array_of_ids));
    ids->capacity = capacity;
    ids->length = 0;
    ids->array = malloc(sizeof(t_id) * capacity);

    return ids;
}

void array_of_ids_free(array_of_ids *list)
{
    free(list->array);
    free(list);
}

t_id array_of_ids_get(array_of_ids *list, size_t index)
{
    return list->array[index];
}

void array_of_ids_add(array_of_ids *list, t_id id)
{
    if (list->length < list->capacity)
    {
        list->array[list->length] = id;
        ++list->length;
        return;
    }

    size_t new_capacity = list->capacity += 8;

    t_id *new_array = malloc(sizeof(t_id) * new_capacity);
    memcpy(new_array, list->array, sizeof(t_id) * list->length);

    free(list->array);
    new_array[list->length] = id;
    list->array = new_array;
    list->capacity = new_capacity;

    ++list->length;
}

array_of_ids *json_to_array_of_ids(json_object *json)
{
    size_t array_length = json_object_array_length(json);
    array_of_ids *ids = array_of_ids_new2(array_length);

    for (size_t i = 0; i < array_length; i++)
    {
        t_id id = json_object_get_uint64(json_object_array_get_idx(json, i));
        array_of_ids_add(ids, id);
    }

    return ids;
}

bool array_of_ids_contains(array_of_ids *ids, t_id id)
{
    for (size_t i = 0; i < array_of_ids_length(ids); i++)
        if (id == array_of_ids_get(ids, i))
            return true;

    return false;
}

bool array_of_ids_remove(array_of_ids *ids, t_id id)
{
    for (size_t i = 0; i < array_of_ids_length(ids); i++)
    {
        if (id == array_of_ids_get(ids, i))
        {
            array_of_ids_remove_at(ids, i);
            return true;
        }
    }
    return false;
}

bool array_of_ids_remove_at(array_of_ids *ids, size_t index)
{
    if (index < 0 || index >= ids->length)
        return false;

    --ids->length;
    for (size_t i = index; i < array_of_ids_length(ids); i++)
        ids->array[i] = ids->array[i + 1];

    return true;
}

size_t array_of_ids_length(array_of_ids *ids)
{
    return ids->length;
}

array_list *array_list_copy(array_list *array)
{
    array_list *copy = array_list_new2(array->free_fn, array->length);

    for (size_t i = 0; i < array_list_length(array); i++)
        array_list_add(copy, array_list_get_idx(array, i));

    return copy;
}

array_list_free_fn *array_list_disable_free(array_list *arr_list)
{
    array_list_free_fn *prev_free = arr_list->free_fn;
    arr_list->free_fn = (array_list_free_fn *)empty_free_fn;
    return prev_free;
}

void array_list_add_all(array_list *dest, array_list *source)
{
    for (size_t i = 0; i < array_list_length(source); i++)
    {
        array_list_add(dest, array_list_get_idx(source, i));
    }
}

void array_list_clear(array_list *list)
{
    array_list_del_idx(list, 0, array_list_length(list));
}

array_list *array_list_find_all(array_list *list, t_comparator comparator, void *data)
{
    array_list *filtered = array_list_new2(list->free_fn, list->length);

    for (size_t i = 0; i < array_list_length(list); i++)
    {
        void *element = array_list_get_idx(list, i);

        if (comparator(element, data))
            array_list_add(filtered, element);
    }
    return filtered;
}

void *array_list_find_first(array_list *list, t_comparator comparator, void *data)
{
    for (size_t i = 0; i < array_list_length(list); i++)
    {
        void *element = array_list_get_idx(list, i);
        if (comparator(element, data))
            return element;
    }
    return NULL;
}

void array_list_del_idx_no_free(array_list *list, size_t index, size_t count)
{
    array_list_free_fn *prev_free = array_list_disable_free(list);

    array_list_del_idx(list, index, count);

    list->free_fn = prev_free;
}

/*

array_of_ids *array_of_ids_new2(size_t capacity)
{
    return array_list_new2(empty_free_fn, capacity);
}

void array_of_ids_free(array_of_ids *list)
{
    array_list_free(list);
}

t_id array_of_ids_get(array_of_ids *list, size_t index)
{
    return (t_id)array_list_get_idx(list, index);
}

void array_of_ids_add(array_of_ids *list, t_id id)
{
    array_list_add(list, (void *)id);
}

array_of_ids *json_to_array_of_ids(json_object *json)
{
    size_t array_length = json_object_array_length(json);
    array_of_ids *ids = array_of_ids_new2(array_length);

    for (size_t i = 0; i < array_length; i++)
    {
        t_id id = json_object_get_uint64(json_object_array_get_idx(json, i));
        array_of_ids_add(ids, id);
    }

    return ids;
}

bool array_of_ids_contains(array_of_ids *ids, t_id id)
{
    for (size_t i = 0; i < array_list_length(ids); i++)
        if (array_of_ids_get(ids, i) == id)

            return true;
    return false;
}

bool array_of_ids_remove(array_of_ids *ids, t_id id)
{
    for (size_t i = 0; i < array_list_length(ids); i++)
    {
        if (array_of_ids_get(ids, i) == id)
        {
            array_list_del_idx(ids, i, 1);
            return true;
        }
    }
    return false;
}

*/