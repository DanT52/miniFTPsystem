#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>


#define MAX_COMMAND_SIZE 5000

#define MY_PORT_NUMBER 49999
#define MY_PORT_NUMBER_STR "49999"
#define BACKLOG 4