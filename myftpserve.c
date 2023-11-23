#include "myftp.h"




void handle_connection(int connectfd, struct sockaddr_in clientAddr, int listenfd) {


    pid_t pid = getpid();
    char buffer[256];
    char buf[80];

    if (close(listenfd) == -1) fprintf(stderr, "Error: Closing listen socket, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);

    char hostName[NI_MAXHOST];
    int hostEntry = getnameinfo((struct sockaddr*)&clientAddr, sizeof(clientAddr), hostName, sizeof(hostName), NULL, 0, NI_NUMERICSERV);

    if (hostEntry != 0) {
        fprintf(stderr, "Error: getnameinfo, STRERR: %s, ERRNO: %d\n", gai_strerror(hostEntry), hostEntry);
        exit(-1);
    }
    
    printf("Child %d: Connection accepted from host %s\n", pid, hostName);

    
    

    int n = read(connectfd, buffer, 255);
    printf("Received command: %s", buffer);

    ssize_t write_res = write(connectfd, buf, 19);
    if (write_res == -1) {
        fprintf(stderr, "Error: Write to socket, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }
    
    if (close(connectfd) == -1) {
        fprintf(stderr, "Error: Closing connection socket, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }
    exit(0);
}


//This function is responsible for starting the server
int start_server(){

    int listenfd, connectfd;
    struct sockaddr_in servAddr, clientAddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        fprintf(stderr, "Error: Socket creation, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        exit(1);
    }
    // initialize server address structure
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(MY_PORT_NUMBER);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // bind socket to server address
    if (bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) == -1){
        fprintf(stderr, "Error: Binding, STRERR: %s, ERRNO: %d\n", strerror(errno), errno);
        close(listenfd);
        exit(1);
    }

    // listen for connections
    if (listen(listenfd, BACKLOG) == -1) {
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
        } 
    }
    // should never reach here
    close(listenfd);
    return 0;
}


int main(int argc, char const *argv[]){

    start_server();
    return 0;
}

