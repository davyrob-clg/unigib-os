#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MB_TO_BYTES(mb) (mb * 1024 * 1024)

// Structure to track allocated blocks
typedef struct MemoryBlock {
    void *pointer;
    size_t size_mb;
    struct MemoryBlock *next;
} MemoryBlock;

MemoryBlock *head = NULL; // Head of the linked list of allocated blocks

// Function to add a new block to the tracking list
void add_block(void *ptr, size_t size_mb) {
    MemoryBlock *new_block = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    if (new_block == NULL) {
        perror("Failed to allocate memory for tracking structure");
        exit(EXIT_FAILURE);
    }
    new_block->pointer = ptr;
    new_block->size_mb = size_mb;
    new_block->next = head;
    head = new_block;
}

// Function to remove and free a block from the tracking list
void remove_block(void *ptr) {
    MemoryBlock *current = head;
    MemoryBlock *prev = NULL;

    while (current != NULL && current->pointer != ptr) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Error: Pointer not found in the allocated list.\n");
        return;
    }

    if (prev == NULL) {
        head = current->next;
    } else {
        prev->next = current->next;
    }

    free(current->pointer); // Free the actual allocated memory
    free(current); // Free the tracking structure
    printf("Freed a block of %zu MB.\n", current->size_mb);
}

// Function to print all allocated blocks
void print_blocks() {
    MemoryBlock *current = head;
    if (current == NULL) {
        printf("No memory blocks currently allocated.\n");
        return;
    }
    printf("--- Current Allocated Blocks ---\n");
    while (current != NULL) {
        printf("Address: %p, Size: %zu MB\n", current->pointer, current->size_mb);
        current = current->next;
    }
    printf("--------------------------------\n");
}

// Function to free all remaining blocks before exiting
void free_all_blocks() {
    MemoryBlock *current = head;
    while (current != NULL) {
        MemoryBlock *next = current->next;
        free(current->pointer);
        free(current);
        current = next;
    }
    head = NULL;
    printf("All allocated memory freed.\n");
}

int main() {
    char command[50];
    size_t size_mb;
    void *ptr;

    printf("UniGib Interactive Memory Allocator (MB)\n");
    printf("Commands: allocate <MB>, free <address>, list, quit\n");

    while (1) {
        printf("> ");
        if (scanf("%s", command) != 1) continue;

        if (strcmp(command, "allocate") == 0) {
            if (scanf("%zu", &size_mb) != 1) {
                printf("Invalid size. Usage: allocate <MB>\n");
                continue;
            }
            size_t size_bytes = MB_TO_BYTES(size_mb);
            ptr = malloc(size_bytes); // Allocate memory
            if (ptr == NULL) {
                perror("Memory allocation failed");
            } else {
                add_block(ptr, size_mb);
		memset(ptr, 1, size_bytes);
                printf("Allocated %zu MB at address %p\n", size_mb, ptr);
            }
        } else if (strcmp(command, "free") == 0) {
            unsigned long address_val;
            if (scanf("%lx", &address_val) != 1) { // Read address as hex
                printf("Invalid address. Usage: free <address>\n");
                continue;
            }
            void *address_ptr = (void *)address_val;
            remove_block(address_ptr);
        } else if (strcmp(command, "list") == 0) {
            print_blocks();
        } else if (strcmp(command, "quit") == 0) {
            free_all_blocks();
            break;
        } else {
            printf("Unknown command. Use 'allocate', 'free', 'list', or 'quit'.\n");
        }
    }

    return 0;
}

