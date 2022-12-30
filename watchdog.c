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

#define PORT 3000

int main()
{
    int SocketFD = socket(AF_INET, SOCK_STREAM, 0);

    // Check if we were successful in creating socketFD.
    if (SocketFD == -1) {
        printf("(-) Could not create socket! -> socket() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    else {
        printf("(=) Socket created successfully.\n");
    }

    // Check if address is already in use.
    int enableReuse = 1;
    if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(enableReuse)) ==  -1)  {
        printf("setsockopt() failed with error code: %d\n" , errno);
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
    if (bind(SocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        printf("(-) Failed to bind address && port to socket! -> bind() failed with error code: %d\n", errno);
        close(SocketFD); // close the socket.
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    else {
        printf("(=) Binding was successful!\n");
    }

    // Make server start listening and waiting, and check if listen() was successful.
    if (listen(SocketFD, 2) == -1) { // We allow no more than MAX_CONNECTIONS queue connections requests.
        printf("Failed to start listening! -> listen() failed with error code : %d\n", errno);
        close(SocketFD); // close the socket.
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    printf("(=) Waiting for incoming TCP-connections...\n");


    //----------------------------------Get TCP Connection From The Sender---------------------------------
    // Create sockaddr_in for IPv4 for holding ip address and port of client and cleans it.
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    unsigned int clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(SocketFD, (struct sockaddr*)&clientAddress, &clientAddressLen); // Accept connection.
    if (clientSocket == -1) {
        printf("(-) Failed to accept connection. -> accept() failed with error code: %d\n", errno);
        close(SocketFD);
        close(clientSocket);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }
    else {
        printf("(=) Connection established.\n\n");
    }

    /*printf("hello partb");
    while (timer < 10seconds)
    {
        recv();
        timer = 0seconds;
    }
    send("timeout")*/

        return 0;
}