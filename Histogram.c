/*****************************************************************************************************************/
// Shaan Saharan
/****************************************************************************************************************/
#define _POSIX_C_SOURCE 200809L // Define to get the features of the POSIX C source version 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define ALPHABET_SIZE 26 // Define the size of the alphabet for histogram calculation

// Signal handler for SIGCHLD to reap zombie processes
void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    // Wait for any child process to exit non-blockingly
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFSIGNALED(status)) {
            printf("PID %d terminated due to the following signal: %s\n", pid, strsignal(WTERMSIG(status)));
        }
    }
}

// Calculate the histogram of letter frequencies in a file
void calculate_histogram(const char *filename, int *histogram, int write_fd) {
    FILE *file = fopen(filename, "r"); // Open file for reading
    if (!file) {
        perror("Failed to open file"); // Error if file can't be opened
        close(write_fd);
        exit(1);
    }

    // Find the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Dynamically allocate memory for the entire file contents
    char *buffer = (char *)malloc(fileSize + 1); // Plus one for null-terminator
    if (!buffer) {
        perror("Failed to allocate memory for file buffer");
        fclose(file);
        exit(1);
    }

    // Read file into buffer
    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0'; // Null-terminate the buffer

    // Process characters from the buffer
    for (long i = 0; i < fileSize; ++i) {
        char ch = buffer[i];
        // Increment the corresponding histogram counter based on character
        if (ch >= 'a' && ch <= 'z') histogram[ch - 'a']++;
        else if (ch >= 'A' && ch <= 'Z') histogram[ch - 'A']++;
    }

    // Clean up
    free(buffer); // Free the dynamically allocated buffer
    fclose(file); // Close the file after reading
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No files inputted, exitting system...\n"); // Check for correct usage
        exit(1);
    }

    // Set up signal handling for SIGCHLD - sigaction
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; ++i) {
        int fd[2];
        // Create a pipe for communication
        if (pipe(fd) == -1) {
            perror("Pipe failed");
            exit(1);
        }

        pid_t pid = fork(); // Fork a new process
        int status;
        if (pid == -1) {
            perror("Fork failed");
            exit(1);
        } else if (pid > 0) {
            // Parent process
            close(fd[1]); // Close the unused write end of the pipe
            if (strcmp(argv[i], "SIG") == 0) {
                kill(pid, SIGINT);
            } else {
                // Wait for the specific child to finish
                waitpid(pid, &status, 0);
                int histogram[ALPHABET_SIZE] = {0}; // Initialize histogram
                if (read(fd[0], histogram, sizeof(histogram)) > 0) { // Check if read is successful
                    // Prepare filename for histogram output
                    char filename[100];
                    snprintf(filename, sizeof(filename), "file%d.hist", pid);
                    printf("PID %d successful with exit status: %d\n", pid, WEXITSTATUS(status));
                    FILE *fileOutput = fopen(filename, "w"); // Open file for writing histogram
                    if (!fileOutput) {
                        perror("Failed to open output file");
                        exit(1);
                    }
                    // Write the histogram to file
                    for (int j = 0; j < ALPHABET_SIZE; ++j) {
                        fprintf(fileOutput, "%c %d\n", 'A' + j, histogram[j]);
                    }
                    fclose(fileOutput); // Close the output file
                }
            }
            close(fd[0]); // Close the read end after reading
        } else {
            // Child process
            close(fd[0]); // Close the unused read end of the pipe
            // Sleep for 10 + 3*i seconds
            sleep(10 + 3 * (i - 1));

            // Dynamically allocate memory for the histogram
            int *histogram = malloc(ALPHABET_SIZE * sizeof(int));
            if (histogram == NULL) {
                perror("Failed to allocate memory for histogram");
                close(fd[1]);
                exit(1);
            }
            memset(histogram, 0, ALPHABET_SIZE * sizeof(int)); // Initialize histogram

            calculate_histogram(argv[i], histogram, fd[1]); // Calculate histogram for given file

            // Write histogram to pipe
            if (write(fd[1], histogram, ALPHABET_SIZE * sizeof(int)) == -1) {
                fprintf(stderr, "Broken Pipe\n");
                free(histogram); // Free the dynamically allocated histogram
                exit(1);
            }

            free(histogram); // Free the dynamically allocated histogram
            close(fd[1]); // Close the write end of the pipe
            exit(0);
        }
    }
    // Wait for all child processes to exit
    while (wait(NULL) > 0);
    return 0;
}
