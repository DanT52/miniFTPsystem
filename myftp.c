#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    int port;
    if (argc != 3 || (port = atoi(argv[1])) <= 0){
        fprintf(stdout, "Usage: %s <port> <hostname | IP address>\n", argv[0]);
        return 1;
    }

    char *hostname = argv[2];

    printf("Port: %d\n", port);
    printf("Hostname/IP: %s\n", hostname);


    return 0;
}