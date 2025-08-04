#include "stack.h"
#include "memory/heap.h"
#include "memory/memory.h"

// Create a new stack with specified data size per element
Stack* stack_create(size_t data_size) {
    if (data_size == 0) {
        return NULL; // Invalid data size
    }
    
    Stack* stack = (Stack*)heap_alloc(&kernel_heap, sizeof(Stack));
    if (!stack) {
        return NULL;
    }
    
    stack->top = NULL;
    stack->count = 0;
    stack->data_size = data_size;
    stack->total_size = 0;
    
    return stack;
}

// Destroy stack and free all nodes
void stack_destroy(Stack* stack) {
    if (!stack) return;
    
    stack_clear(stack);
    heap_free(&kernel_heap, stack);
}

// Clear all nodes from stack
void stack_clear(Stack* stack) {
    if (!stack) return;
    
    StackNode* current = stack->top;
    while (current) {
        StackNode* next = current->next;
        heap_free(&kernel_heap, current);
        current = next;
    }
    
    stack->top = NULL;
    stack->count = 0;
    stack->total_size = 0;
}

// Push data onto stack (LIFO - top of stack)
int stack_push(Stack* stack, const void* data) {
    if (!stack || !data) return -1;
    
    // Node + veri için yer ayır (flexible array member)
    StackNode* new_node = (StackNode*)heap_alloc(&kernel_heap, 
                                                  sizeof(StackNode) + stack->data_size);
    if (!new_node) {
        return -1; // Allocation failed
    }
    
    // Veriyi node'un data alanına kopyala
    memcpy(new_node->data, data, stack->data_size);
    
    // Stack'e ekle (top'a ekle)
    new_node->next = stack->top;
    stack->top = new_node;
    
    stack->count++;
    stack->total_size += stack->data_size;
    
    return 0; // Success
}

// Pop data from stack (LIFO - top of stack)
void* stack_pop(Stack* stack) {
    if (!stack || stack->count == 0) {
        return NULL;
    }
    
    StackNode* to_remove = stack->top;
    void* data = to_remove->data;
    
    // Top'u güncelle
    stack->top = stack->top->next;
    
    stack->count--;
    stack->total_size -= stack->data_size;
    
    // Not: Veri pointer'ını döndürüyoruz ama node'u hemen silmiyoruz
    // Kullanıcı veriyi kopyaladıktan sonra stack_free_node() çağırmalı
    // Ya da alternatif olarak, veriyi heap'e kopyalayıp döndürebiliriz
    
    // Güvenli yaklaşım: Veriyi yeni alana kopyala
    void* data_copy = heap_alloc(&kernel_heap, stack->data_size);
    if (data_copy) {
        memcpy(data_copy, data, stack->data_size);
    }
    
    // Node'u sil
    heap_free(&kernel_heap, to_remove);
    
    return data_copy;
}

// Peek at top data without removing
void* stack_peek(Stack* stack) {
    if (!stack || stack->count == 0) {
        return NULL;
    }
    
    return stack->top->data;
}

// Get number of elements in stack
size_t stack_count(Stack* stack) {
    return stack ? stack->count : 0;
}

// Get total size of all data in stack
size_t stack_total_size(Stack* stack) {
    return stack ? stack->total_size : 0;
}

// Get data size per element
size_t stack_data_size(Stack* stack) {
    return stack ? stack->data_size : 0;
}

// Check if stack is empty
bool stack_is_empty(Stack* stack) {
    return stack ? (stack->count == 0) : true;
}

// Check if stack would be full with given max count
bool stack_is_full(Stack* stack, size_t max_count) {
    return stack ? (stack->count >= max_count) : true;
}

// Push data with automatic memory copy
int stack_push_copy(Stack* stack, const void* data) {
    return stack_push(stack, data); // Already copies data
}

// Pop entire node (user manages the node)
StackNode* stack_pop_node(Stack* stack) {
    if (!stack || stack->count == 0) {
        return NULL;
    }
    
    StackNode* to_remove = stack->top;
    
    // Top'u güncelle
    stack->top = stack->top->next;
    
    stack->count--;
    stack->total_size -= stack->data_size;
    
    // Node'un bağlantısını kes
    to_remove->next = NULL;
    
    return to_remove;
}

// Free a stack node
void stack_free_node(StackNode* node) {
    if (node) {
        heap_free(&kernel_heap, node);
    }
}

// Iterator functions
StackNode* stack_iterator_begin(Stack* stack) {
    return stack ? stack->top : NULL;
}

StackNode* stack_iterator_next(StackNode* current) {
    return current ? current->next : NULL;
}

void* stack_node_data(StackNode* node) {
    return node ? node->data : NULL;
}

// Duplicate entire stack
Stack* stack_duplicate(Stack* stack) {
    if (!stack) return NULL;
    
    Stack* new_stack = stack_create(stack->data_size);
    if (!new_stack) return NULL;
    
    // Geçici bir stack oluştur (sırayı korumak için)
    Stack* temp_stack = stack_create(stack->data_size);
    if (!temp_stack) {
        stack_destroy(new_stack);
        return NULL;
    }
    
    // Orijinal stack'ten elemanları temp'e aktar
    StackNode* current = stack->top;
    while (current) {
        stack_push(temp_stack, current->data);
        current = current->next;
    }
    
    // Temp'ten yeni stack'e aktar (sıra düzelir)
    while (!stack_is_empty(temp_stack)) {
        void* data = stack_pop(temp_stack);
        if (data) {
            stack_push(new_stack, data);
            heap_free(&kernel_heap, data); // Pop allocated memory
        }
    }
    
    stack_destroy(temp_stack);
    return new_stack;
}

// Reverse stack in-place
void stack_reverse(Stack* stack) {
    if (!stack || stack->count <= 1) return;
    
    StackNode* prev = NULL;
    StackNode* current = stack->top;
    StackNode* next = NULL;
    
    // Linked list reversal
    while (current) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    
    stack->top = prev;
}

// Remove first occurrence of data from stack
int stack_remove(Stack* stack, const void* data) {
    return stack_remove_first(stack, data);
}

// Remove first occurrence of data from stack
int stack_remove_first(Stack* stack, const void* data) {
    if (!stack || !data || stack->count == 0) {
        return -1;
    }
    
    StackNode* current = stack->top;
    StackNode* prev = NULL;
    
    while (current) {
        // Compare data using memcmp
        if (memcmp(current->data, data, stack->data_size) == 0) {
            // Found the element to remove
            if (prev == NULL) {
                // Removing top element
                stack->top = current->next;
            } else {
                // Removing middle element
                prev->next = current->next;
            }
            
            // Free the node
            heap_free(&kernel_heap, current);
            
            // Update stack info
            stack->count--;
            stack->total_size -= stack->data_size;
            
            return 0; // Success
        }
        
        prev = current;
        current = current->next;
    }
    
    return -1; // Element not found
}

// Remove all occurrences of data from stack
int stack_remove_all(Stack* stack, const void* data) {
    if (!stack || !data || stack->count == 0) {
        return -1;
    }
    
    int removed_count = 0;
    StackNode* current = stack->top;
    StackNode* prev = NULL;
    
    while (current) {
        StackNode* next = current->next;
        
        // Compare data using memcmp
        if (memcmp(current->data, data, stack->data_size) == 0) {
            // Found element to remove
            if (prev == NULL) {
                // Removing top element
                stack->top = next;
            } else {
                // Removing middle element
                prev->next = next;
            }
            
            // Free the node
            heap_free(&kernel_heap, current);
            
            // Update stack info
            stack->count--;
            stack->total_size -= stack->data_size;
            removed_count++;
            
            // Don't update prev since we removed current
        } else {
            // Move prev only if we didn't remove current
            prev = current;
        }
        
        current = next;
    }
    
    return removed_count > 0 ? removed_count : -1;
}

// Check if stack contains specific data
bool stack_contains(Stack* stack, const void* data) {
    if (!stack || !data || stack->count == 0) {
        return false;
    }
    
    StackNode* current = stack->top;
    
    while (current) {
        if (memcmp(current->data, data, stack->data_size) == 0) {
            return true;
        }
        current = current->next;
    }
    
    return false;
}

// Find and return pointer to data in stack (without removing)
void* stack_find(Stack* stack, const void* data) {
    if (!stack || !data || stack->count == 0) {
        return NULL;
    }
    
    StackNode* current = stack->top;
    
    while (current) {
        if (memcmp(current->data, data, stack->data_size) == 0) {
            return current->data;
        }
        current = current->next;
    }
    
    return NULL;
}