#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// List node structure
typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

// List structure
typedef struct List {
    ListNode* head;
    ListNode* tail;
    size_t size;
} List;

// Function declarations
List* list_create(void);
void list_destroy(List* list);
void list_destroy_with_data(List* list, void (*free_func)(void*));

int list_add(List* list, void* data);
int list_remove(List* list, void* data);
int list_remove_at_index(List* list, size_t index);
void* list_get(List* list, size_t index);
int list_set(List* list, size_t index, void* data);

size_t list_size(List* list);
int list_is_empty(List* list);
void list_clear(List* list);
void list_clear_with_data(List* list, void (*free_func)(void*));

int list_contains(List* list, void* data, int (*compare_func)(const void* a, const void* b));
int list_index_of(List* list, void* data, int (*compare_func)(const void* a, const void* b));

// Iterator functions
ListNode* list_iterator_begin(List* list);
ListNode* list_iterator_next(ListNode* current);
void* list_iterator_data(ListNode* node);

#ifdef __cplusplus
}
#endif