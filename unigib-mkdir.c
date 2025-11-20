#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_DIRNAME_LEN 256

int main() {
    char dirname[MAX_DIRNAME_LEN];
    int status;

    printf("UniGib Make your own directory with a system call");
    printf("Enter the name of the directory to create: ");

    // Read input from the user securely
    if (fgets(dirname, sizeof(dirname), stdin) == NULL) {
        perror("Failed to read directory name");
        return EXIT_FAILURE;
    }

    // Remove the newline character from the end of the input
    dirname[strcspn(dirname, "\n")] = 0;

    // Check if the input is empty
    if (strlen(dirname) == 0) {
        fprintf(stderr, "Directory name cannot be empty.\n");
        return EXIT_FAILURE;
    }

    // Attempt to create the directory with rwxr-xr-x permissions (0755)
    // The actual permissions will be modified by the process's umask
    status = mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (status == 0) {
        printf("Directory '%s' created successfully!\n", dirname);
    } else {
        // Error handling
        fprintf(stderr, "Unable to create directory '%s': ", dirname);
        perror(""); // perror() prints a descriptive error message based on errno
        
        // Specific error messages
        if (errno == EEXIST) {
            fprintf(stderr, "Error: The directory or a file with that name already exists.\n");
        } else if (errno == ENOENT) {
            fprintf(stderr, "Error: The parent directory does not exist or the path is invalid.\n");
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

