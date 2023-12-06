#include "myftp.h"

char* read_socket(int sockfd) {
    int buffer_size = 1;  
    char *buffer = malloc(buffer_size);

    ssize_t bytes_read;
    size_t total_read = 0;
    
    while (1) {
        bytes_read = read(sockfd, buffer + total_read, 1);

        if (bytes_read <= 0) {
            if (bytes_read == 0) {
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
            buffer_size *= 2;
            char *new_buffer = realloc(buffer, buffer_size);
            buffer = new_buffer;
        }
    }
    buffer[total_read] = '\0';
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

    if (write(controlfd, message, strlen(message)) != strlen(message)) {
        fprintf(stderr, "Error: writing to server, STRERR: %s, ERRNO: %d ... exiting ...\n", strerror(errno), errno);
        exit(1);
    }

    free(message);

    if (needAck) {
        char *ack = read_socket(controlfd);
        if (ack[0] == 'E') {
            fprintf(stdout, "Error response from server: %s\n", ack + 1);
            free(ack);
            return 1;
        }
        
        free(ack); 
    }
    return 0;
}

int client_connect(char const* address, char const* port){

    int sockfd;
    struct addrinfo hints;
    struct addrinfo *actualdata;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int err = getaddrinfo(address, port, &hints, &actualdata);
    if (err != 0){
        fprintf(stderr,"Error: %s\n", gai_strerror(err));
        exit(1);
    }

    sockfd = socket(actualdata->ai_family, actualdata->ai_socktype, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Error: Socket creation, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (connect(sockfd, actualdata->ai_addr, actualdata->ai_addrlen) == -1){
        fprintf(stderr, "Error: Connecting, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }

    freeaddrinfo(actualdata);
    return sockfd;
}

int start_data(int controlfd, char* hostname){

    if (write_control(controlfd, "D", NULL, 0) == -1) return -1;

    char* server_response = read_socket(controlfd);
    char* port = &server_response[1];

    int datafd = client_connect(hostname, port);
    

    free(server_response);

    return datafd;

}
void run_more(int read_end) {
    int pidmore = fork();

    if(pidmore == -1) {
        fprintf(stderr, "Error: Fork for more, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (pidmore != 0){
        close(read_end);
        waitpid(pidmore, NULL, 0);
        return;
    }

    dup2(read_end, STDIN_FILENO);
    close(read_end);
    execlp("more", "more", "-20", NULL);
    fprintf(stderr, "Error: exec more, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
    exit(1);
}

void client_ls(){
    int pipefd[2];
    int pidls;

    if (pipe(pipefd) == -1){
        fprintf(stderr, "Error: Pipe creation, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        return;
    }

    pidls = fork();
    if (pidls == -1){
        fprintf(stderr, "Error: Fork for ls, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        return;
    }

    if (pidls != 0){
        close(pipefd[1]);
        run_more(pipefd[0]);
        waitpid(pidls, NULL, 0);
        return;
    }
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);
    execlp("ls", "ls", "-l", NULL);
    fprintf(stderr, "Error: exec ls, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
    exit(1);
}

void server_ls(int controlfd, char *hostname){
    int datafd = start_data(controlfd, hostname);
    if (write_control(controlfd, "L", NULL, 1) == 1){
        close(datafd);
        return;
    }
    run_more(datafd);
    close(datafd);
}

void server_show(int controlfd, char *hostname, char *path){
    int datafd = start_data(controlfd, hostname);
    if (write_control(controlfd, "G", path, 1) == 1){
        close(datafd);
        return;
    }
    run_more(datafd);
    close(datafd);
}

int recive_file(int controlfd, char *hostname, char *path){
    char buf[FILESENDBUF];
    int bytes;

    char *filename = strrchr(path, '/');
    filename = (!filename) ? path : &filename[1];


    int filefd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (filefd < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "Get File: Error File %s already exists\n", filename);
        } else {
            fprintf(stderr, "Get File: Error opening file %s, STRERR: %s, ERRNO: %d\n", filename, strerror(errno), errno);
        }
        close(filefd);
        return 1;
    }
    int datafd = start_data(controlfd, hostname);

    if (write_control(controlfd, "G", path, 1) == 1){
        close(datafd);
        close(filefd);
        unlink(filename);
        return 1;
    }

    while ((bytes = read(datafd, buf, FILESENDBUF)) > 0) {
        
        if (write(filefd, buf, bytes) != bytes){
            fprintf(stderr, "Get File: Error error writing to file, STRERR: %s, ERRNO: %d, aborted...\n", strerror(errno), errno);
            unlink(filename);
            break;
        }
    }
    if (bytes == -1){
        fprintf(stderr, "Get File: Error reciving file, STRERR: %s, ERRNO: %d\n aborted...", strerror(errno), errno);
        unlink(filename);
    } 


    close(filefd);
    close(datafd);
    return 0;

}

int send_file(int controlfd, char *hostname, char *path){
    char buf[FILESENDBUF];
    struct stat statbuf;
    int bytes, infile;


    if (stat(path, &statbuf) != 0) {
        if (errno == ENOENT) printf("put file: No such file or directory '%s'\n", path);
        else fprintf(stderr, "put file: Error getting file status, STRERR: %s, ERRNO: %d, exiting\n", strerror(errno), errno);
        return 1;
    }
    if (!(statbuf.st_mode & S_IRUSR)) {
        printf("put file: No read permission on file '%s'\n", path);
        return 1;
    }
    if (!(S_ISREG(statbuf.st_mode))) {
        printf("put file: The specified path is not a Regular File '%s'\n", path);
        return 1;
    }

    if ((infile = open(path, O_RDONLY, 0600)) < 0) {
        printf("put file: Could not open file for reading '%s'\n", path);
        close(infile);
        return 0;
    }

    int datafd = start_data(controlfd, hostname);

    if (write_control(controlfd, "P", path, 1) == 1){
        close(datafd);
        return 0;
    }

    while ((bytes = read(infile, buf, FILESENDBUF)) > 0) {
        if (write(datafd, buf, bytes) != bytes){

            fprintf(stderr, "put file: error Transmitting data, STRERR: %s, ERRNO: %d, aborting...\n", strerror(errno), errno);
            break;

        }
    }
    if (bytes == -1) fprintf(stderr, "put file: Error reading file, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);

    close(datafd);
    close(infile);
    return 0;
}

int command_loop(int controlfd, char *hostname){

char *command = NULL;
size_t command_size = 0;
    printf("Connected to server %s\n", hostname);

    while (printf("MYFTP> ") && getline(&command, &command_size, stdin) != -1) {
        
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
            char *path = strtok(NULL, " ");
            if (path == NULL) {
                printf("Command error: expecting a parameter: 'cd' command requires a pathname.\n");
            } else {
                if (chdir(path) == -1) {
                    if (errno == EACCES) fprintf(stderr, "Change directory: No access to the directory '%s'\n", path);
                    else if (errno == ENOENT) fprintf(stderr, "Change directory: Directory does not exist '%s'\n", path);
                    else if (errno == ENOTDIR) fprintf(stderr, "Change directory: Not a directory '%s'\n", path);
                    else fprintf(stderr, "Error: Change directory, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
                }
            }
        } else if (strcmp(token, "rcd") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'rcd' command requires a pathname.\n");
            } else {
                write_control(controlfd, "C", pathname, 1);
            }
        } else if (strcmp(token, "ls") == 0) client_ls();
        
        else if (strcmp(token, "rls") == 0) server_ls(controlfd, hostname);

        else if (strcmp(token, "get") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'get' command requires a pathname.\n");
            } else {
                recive_file(controlfd, hostname, pathname);
            }
        } else if (strcmp(token, "show") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'show' command requires a pathname.\n");
            } else {
                server_show(controlfd, hostname, pathname);
            }
        } else if (strcmp(token, "put") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname == NULL) {
                printf("Command error: expecting a parameter: 'put' command requires a pathname.\n");
            } else {
                send_file(controlfd, hostname, pathname);
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




