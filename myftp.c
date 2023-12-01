#include "myftp.h"

char* read_socket(int sockfd) {
    int buffer_size = 1;  // Initial buffer size
    char *buffer = malloc(buffer_size);

    ssize_t bytes_read;
    size_t total_read = 0;
    
    while (1) {
        bytes_read = read(sockfd, buffer + total_read, 1);

        if (bytes_read <= 0) { // Check for socket closure or error
            if (bytes_read == 0) {
                // Socket closed
                fprintf(stderr, "Server unexpectadly closed connection, exiting\n");
            } else {
                fprintf(stderr, " Error: read command from control socket failed STRERR: %s, ERRNO: %d, exiting\n", strerror(errno), errno);
            }
            free(buffer);
            close(sockfd);
            exit(1);
        }

        if (buffer[total_read] == '\n'){
            break;
        }
        total_read ++;

        if (total_read >= buffer_size - 1) {
            buffer_size *= 2;  // Double the buffer size
            char *new_buffer = realloc(buffer, buffer_size);
            buffer = new_buffer;
        }
    }
    buffer[total_read] = '\0'; // Null-terminate the string
    return buffer;
}

int write_control(int controlfd, char* command, char* pathname, int needAck) {
    char *message;
    int message_length;

    if (pathname == NULL) {
        message_length = strlen(command) + 2;
        message = malloc(message_length);
        if (message == NULL) {
            fprintf(stderr, "Error: Memory allocation failed ... exiting ...\n");
            exit(1);
        }
        sprintf(message, "%s\n", command);
    } else {
        message_length = strlen(command) + strlen(pathname) + 2;
        message = malloc(message_length);
        if (message == NULL) {
            fprintf(stderr, "Error: Memory allocation failed ... exiting ...\n");
            exit(1);
        }
        sprintf(message, "%s%s\n", command, pathname);
    }

    // Write message to controlfd
    if (write(controlfd, message, strlen(message)) != strlen(message)) {
        fprintf(stderr, "Error: writing to server, STRERR: %s, ERRNO: %d ... exiting ...\n", strerror(errno), errno);
        exit(1);
    }

    free(message); // Free the allocated memory

    // Check for acknowledgment if needed
    if (needAck) {
        char *ack = read_socket(controlfd);
        if (ack[0] == 'E') {
            // Error response from server
            fprintf(stdout, "Error response from server: %s\n", ack + 1);
            return 1;
        }
        
        free(ack); 
    }
    return 0;
}

int client_connect(char const* address, char const* port){

    int sockfd; //socket file descriptor
    struct addrinfo hints;
    struct addrinfo *actualdata;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    // go from address to addrinfo struct
    int err = getaddrinfo(address, port, &hints, &actualdata);
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

    printf("Connected to server %s\n", address);
    
    return sockfd;
}

int start_data(int controlfd, char* hostname){
    //do data connection
    if (write_control(controlfd, "D", NULL, 0) == -1) return -1;

    char* server_response = read_socket(controlfd);
    char* port = &server_response[1];

    int datafd = client_connect(hostname, port);
    printf("conencted the data connection\n");

    free(server_response);

    return datafd;

}


int command_loop(int controlfd, char *hostname){

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
            free(command);

            write_control(controlfd, "Q", NULL, 1);

            printf("Exiting program.\n");
            
            break;

        } else if (strcmp(token, "cd") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'cd' command requires a pathname.\n");
            } else {
                // Handle 'cd' command
            }
        } else if (strcmp(token, "rcd") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'rcd' command requires a pathname.\n");
            } else {
                // Handle 'rcd' command
            }
        } else if (strcmp(token, "ls") == 0) {

            // Handle 'ls' command
        } else if (strcmp(token, "rls") == 0) {
            start_data(controlfd, hostname);
            // Handle 'rls' command
        } else if (strcmp(token, "get") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'get' command requires a pathname.\n");
            } else {
                // Handle 'get' command
            }
        } else if (strcmp(token, "show") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'show' command requires a pathname.\n");
            } else {
                // Handle 'show' command
            }
        } else if (strcmp(token, "put") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'put' command requires a pathname.\n");
            } else {
                // Handle 'put' command
            }
        } else if (strcmp(token, "") == 0){
            continue;
        } else {
            printf("Command '%s' is unknown - ignored\n", command);
        }

        free(command);
        command = NULL; 
        command_size = 0; 
    }

    return 0;
}




int main(int argc, char *argv[]) {

    int port, controlfd;

    if (argc != 3 || (port = atoi(argv[1])) <= 0){
        fprintf(stdout, "Usage: %s <port> <hostname | IP address>\n", argv[0]);
        return 1;
    }

    char *hostname = argv[2];

    

    controlfd = client_connect(hostname, argv[1]);
    command_loop(controlfd, hostname);

    return 0;
}




