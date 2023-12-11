#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>


bool isInteger(const char *str) {
    char c;
    int value;
    return sscanf(str, " %d %c", &value, &c) == 1;
}

int main(void) 
{
    int pipe1[2], pipe2[2], pipe3[2];
    pid_t child1, child2;

    // Create a pipe
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(pipe3) == -1)
    {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    child1 = fork();

    if (child1 < 0) 
    {
        perror("child1 fork failed");
        exit(EXIT_FAILURE);
    } else if (child1 == 0) //child 1
    {
    	// Child process
	    printf("(child1) pid = %d\n", getpid());
	
        close(pipe1[1]);  // Close the write end of the pipe
        close(pipe2[0]);
        close(pipe3[0]);
        close(pipe3[1]);

        // Read from the pipe
        int val;
        ssize_t read_status;

        while ((read_status = read(pipe1[0], &val, sizeof(int))) > 0) {
            if(read_status == -1)
            {
                perror("Read failed");
                exit(EXIT_FAILURE);
            } else {
                printf("Child1 received: %d\n", val);
                val *= val; // Square the value
                if (write(pipe2[1], &val, sizeof(int)) == -1) { // Send squared value to child 2
                    perror("Write failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
        close(pipe1[0]);
        close(pipe2[1]);
    } 
    else 
    {
        child2 = fork();

        if (child2 < 0) 
        {
            perror("child 2 fork failed");
            exit(EXIT_FAILURE);
        } else if (child2 == 0) //child 2
        {
            // Child process
            printf("(child2) pid = %d\n", getpid());
        
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[1]);
            close(pipe3[0]);

            // Read from the pipe
            int val;
            ssize_t read_status;

            while ((read_status = read(pipe2[0], &val, sizeof(int))) > 0) {
                if(read_status == -1)
                {
                    perror("Read failed");
                    exit(EXIT_FAILURE);
                } else {
                    printf("Child2 received: %d\n", val);
                    val += 1; // add 1
                    if (write(pipe3[1], &val, sizeof(int)) == -1) { // Send value to parent
                        perror("Write failed");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            close(pipe2[0]);
            close(pipe3[1]);
        } 
        else 
        {
            // Parent process
            printf("(parent) pid = %d\n", getpid());
        
            close(pipe1[0]);  // Close the read end of the pipe
            close(pipe2[0]);
            close(pipe2[1]);
            close(pipe3[1]);
            
            // Sleep for 3 seconds before writing to the pipe
            //sleep(3);

            int input;
            char buffer[50];
            int val;

            while(fgets(buffer, sizeof(buffer), stdin) != NULL) {
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character if present
                if (!isInteger(buffer)) {
                    printf("Error: Input is not an integer\n");
                    continue;
                }

                sscanf(buffer, "%d", &input); // Convert string to integer

                printf("%d\n", input);

                if(write(pipe1[1], &input, sizeof(int)) == -1) // Send input to child 1
                {
                    perror("Write failed");
                    exit(EXIT_FAILURE);
                }

                if (read(pipe3[0], &val, sizeof(int)) < 0) {
                    perror("Read failed");
                    exit(EXIT_FAILURE);
                } else {
                    printf("parent received: %d\n", val);
                }
            }


            // Close the write end of the pipe in the parent
            close(pipe1[1]);
            close(pipe3[0]);

            int status1, status2;

            // Wait for the child to finish
            waitpid(child1, &status1, 0);
            waitpid(child2, &status2, 0);
        }
    }

    return 0;
}