#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define PORT 3000
#define IP_ADDRESS "127.0.0.1"
#define IP4_HDRLEN 20 // IPv4 header len without options
#define ICMP_HDRLEN 8 // ICMP header len for echo req
#define PACKET_SIZE 64

struct icmp icmphdr; // ICMP-header
char data[] = "This is the ping.\n"; // The data we send as ping.
int sequence = 0; // A global variable for the sequence number.

unsigned short calculate_checksum(unsigned short *, int ); // Checksum Method.
ssize_t sendPing (int, struct sockaddr_in);
int recievePong(int, char*, int, struct sockaddr_in, socklen_t, struct timeval, struct timeval);

// run 2 programs using fork + exec
// command: make clean && make all && ./partb
int main()
{
    // Creates socket named "socketFD". FD for file descriptor.
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);

    // Check if we were successful in creating socket.
    if(socketFD == -1) {
        printf("(-) Could not create socket! -> socket() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    else {
        printf("(=) Socket created successfully.\n");
    }

    // Create sockaddr_in for IPv4 for holding ip address and port and clean it.
    struct sockaddr_in serverAddress;
    memset(&serverAddress, '\0', sizeof(serverAddress));

    // Assign port and address to "serverAddress".
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // Short, network byte order.
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // Convert address to binary.
    if (inet_pton(AF_INET, IP_ADDRESS, &serverAddress.sin_addr) <= 0)
    {
        printf("(-) Failed to convert IPv4 address to binary! -> inet_pton() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }

    //Create connection with server.
    int connection = connect(socketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

    // Check if we were successful in connecting with server.
    if(connection == -1) {
        printf("(-) Could not connect to server! -> connect() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return// EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    else {
        printf("(=) Connection with server established.\n\n");
    }

    char *args[2];
    // compiled watchdog.c by makefile
    args[0] = "./watchdog";
    args[1] = NULL;
    int status;
    int pid = fork();
    if (pid == 0)
    {
        printf("in child \n");
        execvp(args[0], args);
    }
    wait(&status); // waiting for child to finish before exiting
    printf("child exit status is: %d", status);
    return 0;
}
