#include "myftp.h"



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

int client_connect(char const* address){

    int sockfd; //socket file descriptor
    struct addrinfo hints;
    struct addrinfo *actualdata;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    // go from address to addrinfo struct
    int err = getaddrinfo(address, MY_PORT_NUMBER_STR, &hints, &actualdata);
    if (err != 0){
        fprintf(stderr,"Error: %s\n", gai_strerror(err));
        exit(1);
    }
    //create socket
    sockfd = socket(actualdata->ai_family, actualdata->ai_socktype, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Error: Socket creation, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }

    //connect to server with socket
    if (connect(sockfd, actualdata->ai_addr, actualdata->ai_addrlen) == -1){
        fprintf(stderr, "Error: Connecting, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }
    //done with addrinfo
    freeaddrinfo(actualdata);

    char cmd[] = "B\n";
    int n = write(sockfd, cmd, strlen(cmd));
    char buffer[20];
    int bytes_read = read(sockfd, buffer, sizeof(buffer));

    if (bytes_read == -1) {
        fprintf(stderr, "Error: Reading from socket, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        close(sockfd);
        exit(1);
    }
    buffer[19] = '\0'; //terminate string
    printf("%s", buffer);
    if (close(sockfd) == -1) {
        fprintf(stderr, "Error: Closing socket, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }

    exit(0); //sucess
}

int command_loop(){

char *command = NULL;
size_t command_size = 0;

    while (printf("MYFTP> ") && getline(&command, &command_size, stdin) != -1) {
        
        // Remove newline character at the end of input
        command[strcspn(command, "\n")] = 0;

        char *token = strtok(command, " ");
        if (token == NULL) {
            continue;
        }

        if (strcmp(token, "exit") == 0) {
            printf("Exiting program.\n");
            free(command);
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

            client_connect("localhost");
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

        free(command);
        command = NULL; 
        command_size = 0; 
    }

    return 0;
}

