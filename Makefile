CC = gcc
CFLAGS = 

all: myftp myftpserve

myftp: myftp.c myftp.h
	$(CC) $(CFLAGS) -o myftp myftp.c

myftpserve: myftpserve.c myftp.h
	$(CC) $(CFLAGS) -o myftpserve myftpserve.c

clean:
	rm -f myftp myftpserve
