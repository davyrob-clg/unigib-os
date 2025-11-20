#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define CHUNK_SIZE (10 * 1024 * 1024) // 10 Megabytes per chunk

int main() {
    void *ptr;
    size_t count = 0;
    size_t total_allocated = 0;

    printf("Attempting to allocate memory in %d byte chunks until failure...\n", CHUNK_SIZE);

    while (1) {
	usleep(1000);
        // Allocate a chunk of memory
        ptr = malloc(CHUNK_SIZE);

        // Check if malloc failed
        if (ptr == NULL) {
            fprintf(stderr, "\nFailed to allocate memory chunk %zu.\n", count + 1);
            
            // Print system error message if available (optional, not guaranteed by C standard)
            if (errno) {
                fprintf(stderr, "Reason: %s\n", strerror(errno));
            }
            break; // Exit the loop on failure
        }
        
        // If successful, we don't need to free it in this specific program
        // because the goal is to exhaust the memory.
        // In a real program, you must free memory when done.

        count++;
        total_allocated += CHUNK_SIZE;
        printf("Successfully allocated chunk %zu. Total allocated: %zu bytes.\n", count, total_allocated);
    }

    printf("\nTotal successfully allocated before failure: %zu bytes (approx %.2f MB).\n", total_allocated, (double)total_allocated / (1024 * 1024));
    
    // Note: The allocated memory is not freed in this program as it's designed
    // to exit after exhausting memory. The operating system reclaims all
    // memory resources when the program terminates.

    return 0;
}

