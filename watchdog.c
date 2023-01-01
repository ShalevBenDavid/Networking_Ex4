#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>

#define PORT 3012

int main() {
//============================================
// Create TCP Socket And Connect To better_ping
//============================================

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);

    // Check if we were successful in creating socketFD.
    if (socketFD == -1) {
        printf("(-) Could not create socket! -> socket() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    } else {
        printf("(=) Socket created successfully.\n");
    }

    // Check if address is already in use.
    int enableReuse = 1;
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(enableReuse)) == -1) {
        printf("setsockopt() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }

    // Create sockaddr_in for IPv4 for holding ip address and port and clean it.
    struct sockaddr_in serverAddress;
    memset(&serverAddress, '\0', sizeof(serverAddress));

    // Assign port and address to "serverAddress".
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // Short, network byte order.
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding port and address to socket and check if binding was successful.
    if (bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("(-) Failed to bind address && port to socket! -> bind() failed with error code: %d\n", errno);
        close(socketFD); // close the socket.
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    } else {
        printf("(=) Binding was successful!\n");
    }

    // Make server start listening and waiting, and check if listen() was successful.
    if (listen(socketFD, 1) == -1) { // We allow no more than MAX_CONNECTIONS queue connections requests.
        printf("Failed to start listening! -> listen() failed with error code : %d\n", errno);
        close(socketFD); // close the socket.
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    printf("(=) Waiting for incoming TCP-connections...\n");


    //----------------------------------Get TCP Connection From The Sender---------------------------------
    // Create sockaddr_in for IPv4 for holding ip address and port of client and cleans it.
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    unsigned int clientAddressLen = sizeof(clientAddress);
    int pingSocket = accept(socketFD, (struct sockaddr *) &clientAddress, &clientAddressLen); // Accept connection.
    if (pingSocket == -1) {
        printf("(-) Failed to accept connection. -> accept() failed with error code: %d\n", errno);
        close(socketFD);
        close(pingSocket);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    } else {
        printf("(=) Connection established.\n\n");
    }

//=========================
// Make Socket Non-Blocking
//=========================

   int flags=fcntl(pingSocket, F_GETFL, 0);  // DO CHECK ERRORS!
    flags |= O_NONBLOCK;
    fcntl(pingSocket, F_SETFL, flags);  // SERIOUSLY*/

//=================================
// Create Timer And Check If Timeout
//=================================

    time_t start, current;
    int timer = 0;
    char buffer[6];
    while (timer < 10) {
        if (recv(pingSocket, buffer, 6, 0) > 0) {
            start = time(NULL);
        }
        current = time(NULL);
        timer = (int) difftime(current, start);
    }
    send(socketFD, "timeout", 7, 0);
    close(socketFD);
    close(pingSocket);
    exit(EXIT_FAILURE);
}