#include "myftp.h"


char* read_socket(int sockfd, pid_t pid) {
    int buffer_size = 1;  // Initial buffer size
    char *buffer = malloc(buffer_size);

    ssize_t bytes_read;
    size_t total_read = 0;
    
    while (1) {
        bytes_read = read(sockfd, buffer + total_read, 1);

        if (bytes_read <= 0) { // Check for socket closure or error
            if (bytes_read == 0) {
                // Socket closed
                fprintf(stderr, "Child %d: client unexpectadly closed connection, exiting\n", pid);

            } else {
                fprintf(stderr, "Child %d: Error: read command from control socket failed STRERR: %s, ERRNO: %d, exiting\n",pid, strerror(errno), errno);
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

void send_ack(int controlfd, int pid){
    if (write(controlfd, "A\n", 2) != 2){
        fprintf(stderr, "Child %d: Error: acknowledgment to client failed, STRERR: %s, ERRNO: %d ... child exiting\n", pid, strerror(errno), errno);
        exit(1);
    }
}

int rls(int connfd, int pid, int controlfd){
    pid_t inner_pid = fork();

    if (inner_pid < 0){
        fprintf(stderr, "Child %d: Error: fork in rls failed, STRERR: %s, ERRNO: %d, exiting\n", pid, strerror(errno), errno);
        return 1;
    }

    if (inner_pid == 0){
        dup2(connfd, STDOUT_FILENO);
        close(connfd);
        close(controlfd);
        execlp("ls", "ls", "-l", (char *) NULL);
        exit(1);
    }

    close(connfd);
    int result;
    waitpid(inner_pid, &result, 0);

    if (WIFEXITED(result) && WEXITSTATUS(result) == 0){
        send_ack(controlfd, pid);
        return 0;
    }

    fprintf(stderr, "Child %d: Error: ls command on server failed ... child exiting\n", pid);
    return 1;
}

int handle_data_commands(int controlfd, pid_t pid){
    char *buffer;

    int datafd;
    struct sockaddr_in servAddr;
    socklen_t addrlen = sizeof(servAddr);

    datafd = socket(AF_INET, SOCK_STREAM, 0);
    if (datafd < 0) {
        fprintf(stderr, "Child %d: Error: data Socket creation failed, STRERR: %s, ERRNO: %d ... exiting\n", pid, strerror(errno), errno);
        return 1;
    }

    // initialize server address structure
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(0);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);
    char acknowledgement[32];

    // bind socket to server address
    if (bind(datafd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
        fprintf(stderr, "Child %d: Error: Binding data socket, STRERR: %s, ERRNO: %d\n", pid, strerror(errno), errno);
        close(datafd);
        return 1;
    }

    // Retrieve the assigned port number
    if (getsockname(datafd, (struct sockaddr *)&servAddr, &addrlen) < 0) {
        fprintf(stderr, "Child %d: Error: getting data socket name, STRERR: %s, ERRNO: %d\n", pid, strerror(errno), errno);
        close(datafd);
        return 1;
    }
    int assignedPort = ntohs(servAddr.sin_port);
    printf("Child %d: Assigned ephemeral port: %d\n", pid, assignedPort);
    sprintf(acknowledgement, "A%d\n", assignedPort);

    if (write(controlfd, acknowledgement, strlen(acknowledgement)) != strlen(acknowledgement)) {
        fprintf(stderr, "Child %d: Error: sending acknowledgement with portnum, STRERR: %s, ERRNO: %d\n", pid, strerror(errno), errno);
        close(datafd);
        return 1;
    }

    // listen for connections

    if (listen(datafd, 1) < 0) {
        fprintf(stderr, "Child %d: Error: Listening data socket, STRERR: %s, ERRNO: %d\n", pid, strerror(errno), errno);
        close(datafd);
        return 1;
    }

    int connfd = accept(datafd, (struct sockaddr *)&servAddr, &addrlen);
    if (connfd < 0) {
        fprintf(stderr, "Child %d: Error: accepting data connection, STRERR: %s, ERRNO: %d\n", pid, strerror(errno), errno);
        close(datafd);
        return 1;
    }
    close(datafd);

    buffer = read_socket(controlfd, pid);
    printf("command recived: %s.\n", buffer);

    if (buffer[0] == 'L'){
        free(buffer);
        return rls(connfd, pid, controlfd);
    } 
    
    else if (buffer[0] == 'G') { // get

    } else if (buffer[0] == 'P') { // put

    }

    close(connfd);
    

    return 0;
}

void command_loop(int connectfd, pid_t pid){
    char *buffer;
    char *path;

    while(1){
        buffer = read_socket(connectfd, pid);
        printf("command recived: %s.\n", buffer);

        if (buffer[0] == 'D') {// Establish data connection and send port number
            if (handle_data_commands(connectfd, pid) == 1) break;
            
        } else if (buffer[0] == 'C') {
            path = &buffer[1];
            if (chdir(path) != 0){
                fprintf(stderr, "Child %d: Error: bad path given, continuing...\n", pid);
                if (write(connectfd, "ENo such file or directory\n", 27) != 27){
                    fprintf(stderr, "Child %d: Error: writing Err to client failed, STRERR: %s, ERRNO: %d ... child exiting\n", pid, strerror(errno), errno);
                    exit(1);
                }

            }else{
                printf("Child %d: changed current directory to %s\n", pid, path);
                send_ack(connectfd, pid);
            }

        } else if (buffer[0] == 'Q') {
            fprintf(stderr, "Child %d: quitting...\n", pid);
            send_ack(connectfd, pid);
            break;
        }
        free(buffer);
        
        
    }
    if (close(connectfd) == -1) {
        fprintf(stderr, "Child %d: Error: Closing connection socket, STRERR: %s, ERRNO: %d\n", pid, strerror(errno), errno);
        exit(1);
    }
    free(buffer);
    exit(0);
}



void handle_connection(int connectfd, struct sockaddr_in clientAddr, int listenfd) {

    pid_t pid = getpid();
    
    if (close(listenfd) == -1) fprintf(stderr, "Error: Closing listen socket, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);

    char hostName[NI_MAXHOST];
    int hostEntry = getnameinfo((struct sockaddr*)&clientAddr, sizeof(clientAddr), hostName, sizeof(hostName), NULL, 0, NI_NUMERICSERV);

    if (hostEntry != 0) {
        fprintf(stderr, "Error: getnameinfo, STRERR: %s, ERRNO: %d\n", gai_strerror(hostEntry), hostEntry);
        exit(-1);
    }
    printf("Child %d: Connection accepted from host %s\n", pid, hostName);

    command_loop(connectfd, pid);

}





int main(int argc, char const *argv[]){

//some socket creation code reused from asisgnent 8
    int listenfd, connectfd;
    struct sockaddr_in servAddr, clientAddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        fprintf(stderr, "Error: Socket creation, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }
    // initialize server address structure
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(MY_PORT_NUMBER);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // bind socket to server address
    if (bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
        fprintf(stderr, "Error: Binding, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        close(listenfd);
        exit(1);
    }

    // listen for connections
    if (listen(listenfd, BACKLOG) < 0) {
        fprintf(stderr, "Error: Listening, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        close(listenfd);
        exit(1);
    }
    
    int length = sizeof(clientAddr);

    //forever loop
    while (1) {
        connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
        
        if (connectfd == -1) {
            fprintf(stderr, "Error: Accepting connection, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
            continue;  // or decide to exit based on the type of error
        }
        int pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Error: Forking failed, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
            exit(1);
        }
        if (pid == 0) {            // child process
            handle_connection(connectfd, clientAddr, listenfd);  // Call the new function here
        } else {  // parent process
            printf("Parent: spawned child %d, waiting for new connection\n", pid);
            close(connectfd);  // Close the connected socket in the parent
            int status;
            pid_t done = waitpid(-1, &status, WNOHANG);
            if (done > 0) {
                printf("Parent detected termination of child process %d, exit code: %d\n", done, WEXITSTATUS(status));
            }
        } 
    }
    // should never reach here
    close(listenfd);
    return 0;
}

