#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Stack node structure - her node'un arkasında data_size kadar alan var
typedef struct StackNode {
    struct StackNode* next;  // Pointer to next node in stack
    // Buradan sonra data_size kadar alan var (flexible array member kullanımı)
    uint8_t data[];         // Gerçek veri buraya yazılır
} StackNode;

// Stack structure - LIFO mantığında çalışır
typedef struct Stack {
    StackNode* top;         // Stack'in tepesi (son eklenen eleman)
    size_t count;           // Stack'teki eleman sayısı
    size_t data_size;       // Her elemanın veri boyutu
    size_t total_size;      // Stack'teki toplam veri boyutu
} Stack;

// Stack creation and destruction
Stack* stack_create(size_t data_size);
void stack_destroy(Stack* stack);
void stack_clear(Stack* stack);

// LIFO operations
int stack_push(Stack* stack, const void* data);
void* stack_pop(Stack* stack);
void* stack_peek(Stack* stack);

// Stack information
size_t stack_count(Stack* stack);
size_t stack_total_size(Stack* stack);
size_t stack_data_size(Stack* stack);
bool stack_is_empty(Stack* stack);
bool stack_is_full(Stack* stack, size_t max_count);

// Advanced operations
int stack_push_copy(Stack* stack, const void* data);
StackNode* stack_pop_node(Stack* stack);
void stack_free_node(StackNode* node);

// Iterator functions (top to bottom traversal)
StackNode* stack_iterator_begin(Stack* stack);
StackNode* stack_iterator_next(StackNode* current);
void* stack_node_data(StackNode* node);

// Stack duplication
Stack* stack_duplicate(Stack* stack);

// Stack reversal (in-place)
void stack_reverse(Stack* stack);

int stack_remove(Stack* stack, const void* data);

int stack_remove_first(Stack* stack, const void* data);

#ifdef __cplusplus
}
#endif