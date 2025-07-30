#include "list.h"
#include "memory/heap.h"

// Create a new list
List* list_create(void) {
    List* list = (List*)heap_alloc(&kernel_heap, sizeof(List));
    if (!list) {
        return NULL;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    
    return list;
}

// Destroy list (doesn't free data)
void list_destroy(List* list) {
    if (!list) return;
    
    list_clear(list);
    heap_free(&kernel_heap, list);
}

// Destroy list and free data using provided function
void list_destroy_with_data(List* list, void (*free_func)(void*)) {
    if (!list) return;
    
    list_clear_with_data(list, free_func);
    heap_free(&kernel_heap, list);
}

// Add element to the end of the list
int list_add(List* list, void* data) {
    if (!list) return -1;
    
    ListNode* new_node = (ListNode*)heap_alloc(&kernel_heap, sizeof(ListNode));
    if (!new_node) {
        return -1;
    }
    
    new_node->data = data;
    new_node->next = NULL;
    
    if (list->size == 0) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    
    list->size++;
    return 0;
}

// Remove first occurrence of data
int list_remove(List* list, void* data) {
    if (!list || list->size == 0) return -1;
    
    ListNode* current = list->head;
    ListNode* previous = NULL;
    
    while (current) {
        if (current->data == data) {
            if (previous) {
                previous->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if (current == list->tail) {
                list->tail = previous;
            }
            
            heap_free(&kernel_heap, current);
            list->size--;
            return 0;
        }
        
        previous = current;
        current = current->next;
    }
    
    return -1; // Not found
}

// Remove element at specific index
int list_remove_at_index(List* list, size_t index) {
    if (!list || index >= list->size) return -1;
    
    if (index == 0) {
        ListNode* to_remove = list->head;
        list->head = list->head->next;
        
        if (list->size == 1) {
            list->tail = NULL;
        }
        
        heap_free(&kernel_heap, to_remove);
        list->size--;
        return 0;
    }
    
    ListNode* current = list->head;
    for (size_t i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    ListNode* to_remove = current->next;
    current->next = to_remove->next;
    
    if (to_remove == list->tail) {
        list->tail = current;
    }
    
    heap_free(&kernel_heap, to_remove);
    list->size--;
    return 0;
}

// Get element at specific index
void* list_get(List* list, size_t index) {
    if (!list || index >= list->size) return NULL;
    
    ListNode* current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current->data;
}

// Set element at specific index
int list_set(List* list, size_t index, void* data) {
    if (!list || index >= list->size) return -1;
    
    ListNode* current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    current->data = data;
    return 0;
}

// Get list size
size_t list_size(List* list) {
    return list ? list->size : 0;
}

// Check if list is empty
int list_is_empty(List* list) {
    return list ? (list->size == 0) : 1;
}

// Clear all elements (doesn't free data)
void list_clear(List* list) {
    if (!list) return;
    
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        heap_free(&kernel_heap, current);
        current = next;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Clear all elements and free data using provided function
void list_clear_with_data(List* list, void (*free_func)(void*)) {
    if (!list || !free_func) return;
    
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        free_func(current->data);
        heap_free(&kernel_heap, current);
        current = next;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Check if list contains data
int list_contains(List* list, void* data, int (*compare_func)(const void* a, const void* b)) {
    return list_index_of(list, data, compare_func) != -1;
}

// Get index of data in list
int list_index_of(List* list, void* data, int (*compare_func)(const void* a, const void* b)) {
    if (!list || !compare_func) return -1;
    
    ListNode* current = list->head;
    int index = 0;
    
    while (current) {
        if (compare_func(current->data, data) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }
    
    return -1; // Not found
}

// Iterator functions
ListNode* list_iterator_begin(List* list) {
    return list ? list->head : NULL;
}

ListNode* list_iterator_next(ListNode* current) {
    return current ? current->next : NULL;
}

void* list_iterator_data(ListNode* node) {
    return node ? node->data : NULL;
}
