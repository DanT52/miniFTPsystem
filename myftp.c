#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_SIZE 5000

int command_loop();

int main(int argc, char *argv[]) {

    int port;
    if (argc != 3 || (port = atoi(argv[1])) <= 0){
        fprintf(stdout, "Usage: %s <port> <hostname | IP address>\n", argv[0]);
        return 1;
    }

    char *hostname = argv[2];

    printf("Port: %d\n", port);
    printf("Hostname/IP: %s\n", hostname);

    command_loop();



    return 0;
}

int command_loop(){

char command[MAX_COMMAND_SIZE];

    while (printf("MYFTP> ") && fgets(command, MAX_COMMAND_SIZE, stdin)) {
        
        // Remove newline character at the end of input
        command[strcspn(command, "\n")] = 0;

        char *token = strtok(command, " ");
        if (token == NULL) {
            continue;
        }

        if (strcmp(token, "exit") == 0) {
            printf("Exiting program.\n");
            break;

        } else if (strcmp(token, "cd") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Error: 'cd' command requires a pathname.\n");
            } else {
                // Handle 'cd' command
            }
        } else if (strcmp(token, "rcd") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Error: 'rcd' command requires a pathname.\n");
            } else {
                // Handle 'rcd' command
            }
        } else if (strcmp(token, "ls") == 0) {
            // Handle 'ls' command
        } else if (strcmp(token, "rls") == 0) {
            // Handle 'rls' command
        } else if (strcmp(token, "get") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Error: 'get' command requires a pathname.\n");
            } else {
                // Handle 'get' command
            }
        } else if (strcmp(token, "show") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Error: 'show' command requires a pathname.\n");
            } else {
                // Handle 'show' command
            }
        } else if (strcmp(token, "put") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Error: 'put' command requires a pathname.\n");
            } else {
                // Handle 'put' command
            }
        } else {
            printf("Invalid command.\n");
        }
    }

    return 0;
}