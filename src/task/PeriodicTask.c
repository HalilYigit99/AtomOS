#include <task/PeriodicTask.h>
#include <memory/memory.h>
#include <stream/OutputStream.h>
#include <list.h>

List* periodic_tasks;

PeriodicTask* createPeriodicTask(uint32_t functionAddr, uint32_t interval) {
    if (!periodic_tasks) {
        periodic_tasks = list_create();
        if (!periodic_tasks) {
            return NULL; // Failed to create task list
        }
    }

    PeriodicTask* task = (PeriodicTask*)kmalloc(sizeof(PeriodicTask));
    if (!task) {
        return NULL; // Memory allocation failed
    }

    task->functionAddr = functionAddr;
    task->interval = interval;
    task->lastExecution = 0; // Initialize to 0 or current timestamp
    task->isActive = false; // Initially inactive

    list_add(periodic_tasks, task);
    return task;
}

void startPeriodicTask(PeriodicTask* task) {
    if (!task) {
        return; // Invalid task
    }

    task->isActive = true;
    // Additional logic to schedule the task can be added here
}

void stopPeriodicTask(PeriodicTask* task) {
    if (!task) {
        return; // Invalid task
    }

    task->isActive = false;
    // Additional logic to unschedule the task can be added here
}

void executePeriodicTasks(uint32_t currentTime) {
    if (!periodic_tasks || list_size(periodic_tasks) == 0) {
        return; // No tasks to execute
    }

    for (ListNode* node = list_iterator_begin(periodic_tasks); node; node = list_iterator_next(node)) {
        PeriodicTask* task = (PeriodicTask*)node->data;
        if (!task || !task->isActive) {
            continue; // Skip inactive tasks
        }

        // Check if the task should be executed
        if (currentTime - task->lastExecution < task->interval) {
            continue; // Not time to execute yet
        }

        // Call the function at functionAddr
        if (task->functionAddr) {
            void (*taskFunction)(void) = (void (*)(void))task->functionAddr;
            taskFunction(); // Execute the task function
        }

        task->lastExecution = currentTime; // Update last execution time
    }

}
