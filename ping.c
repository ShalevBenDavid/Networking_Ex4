// Created by Ron and Shalev

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define IP4_HDRLEN 20 // IPv4 header len without options
#define ICMP_HDRLEN 8 // ICMP header len for echo req
#define PACKET_SIZE 64

struct icmp icmphdr; // ICMP-header
char data[] = "This is the ping.\n"; // The data we send as ping.
int sequence = 0; // A global variable for the sequence number.

unsigned short calculate_checksum(unsigned short *, int ); // Checksum Method.
ssize_t sendPing (int, struct sockaddr_in);
int receivePong(int, char*, int, struct sockaddr_in, socklen_t, struct timeval, struct timeval);

int main(int argc, char *argv[])
{
    /*------------------------------ Create dest_in ------------------------------*/
    struct sockaddr_in dest_in; // Struct for holding the info for destination.
    memset(&dest_in, 0, sizeof(struct sockaddr_in)); // Resting "dest_in".
    dest_in.sin_family = AF_INET; // Defining as IPv4 communication.
    dest_in.sin_addr.s_addr = inet_addr(argv[1]); // Assigning it the IP address we got as an argument.
    // The port is irrelevant for Networking and therefore was zeroed.

    if (inet_pton(AF_INET, argv[1], &dest_in.sin_addr) <= 0)
    {
        printf("(-) Failed to convert IPv4 address to binary! -> inet_pton() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return EXIT_FAILURE (defined as 1 in stdlib.h).
    }

//==================
// Create RAW Socket
//==================

    int sock;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) // Check if we were successful in creating the socket.
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }
    // Connect to the address specified by dest_in.
    int connection = connect(sock, (struct sockaddr *) &dest_in, sizeof(dest_in));
    // Check if we were successful in connecting with the destination.
    if (connection == -1) {
        printf("(-) Could not connect to server! -> connect() failed with error code: %d\n", errno);
        exit(EXIT_FAILURE); // Exit program and return// EXIT_FAILURE (defined as 1 in stdlib.h).
    } else {
        printf("(=) Connection with server established.\n\n");
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    /*------------------------------ Send Ping And Get The Pong Response ------------------------------*/
    socklen_t len = sizeof(struct sockaddr_in); // Save the dest_in length.
    struct timeval start, end;
    ssize_t bytes_received; // Will help keep track of the bytes received.
    char reply[PACKET_SIZE] = {0}; // A buffer to hold the replay (pong).

    printf("PING %s (%s): %d data bytes\n",
           argv[1], argv[1], PACKET_SIZE - ICMP_HDRLEN);
    // An infinite do-while loop to send ping and receive pong.
    do {
        // Start the timer.
        gettimeofday(&start, 0);
        // Create the packet and send the ping.
        sendPing (sock, dest_in);
        // Receive the ping and save in "reply".
        bytes_received = receivePong(sock, reply, sizeof(reply), dest_in, len, start, end);
        // Reset reply.
        bzero(reply, PACKET_SIZE);
        sleep(1); // Wait for a second before repeating process.
    } while (bytes_received != 0);

    // Close the raw socket descriptor.
    close(sock);

    return 0;
}

//===================
//Send Ping
//===================

ssize_t sendPing (int sock, struct sockaddr_in dest_in) {
    int dataLen = (int) strlen(data) + 1;

    //===================
    // ICMP header
    //===================

    /*------------------------------ Initialize icmphdr ------------------------------*/
    icmphdr.icmp_type = ICMP_ECHO;      // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_code = 0;    // Message Code (8 bits): echo request
    icmphdr.icmp_id = 18;    // Identifier (16 bits): some number to trace the response.
    icmphdr.icmp_seq = sequence++;    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_cksum = 0;  // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation

    char packet[PACKET_SIZE] = {0};     // Combine the packet
    memcpy((packet), &icmphdr, ICMP_HDRLEN);     // Next, ICMP header
    memcpy(packet + ICMP_HDRLEN, data, dataLen);     // After ICMP header, add the ICMP data.

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), sizeof(packet));
    memcpy((packet), &icmphdr, ICMP_HDRLEN);
    // Send the packet using sendto() for sending datagrams.
    ssize_t bytes_sent = sendto(sock, packet, sizeof (packet), 0, (struct sockaddr*)&dest_in, sizeof(dest_in));

    if (bytes_sent == -1)
    {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    }
    return bytes_sent;
}

//===================
// Receive Pong
//===================

int receivePong(int sock, char* reply, int sizToSend, struct sockaddr_in dest_in, socklen_t len, struct timeval start, struct timeval end) {
    // Receive the packet using recvfrom() for receiving datagrams.
    int bytes_received = (int) recvfrom(sock, reply, sizToSend, 0, (struct sockaddr*)&dest_in, &len);
    gettimeofday(&end, 0); // End the timer.
    if (bytes_received > 0)
    {
        struct iphdr *iphdr = (struct iphdr *)reply;
        struct icmphdr *icmphdrP = (struct icmphdr *)(reply + sizeof(struct iphdr));

        char destinationIP[32] = { '\0' };
        inet_ntop(AF_INET, &dest_in.sin_addr.s_addr, destinationIP, sizeof(destinationIP));

        float milliseconds = (float )(end.tv_sec - start.tv_sec) * 1000.0f +
                             (float) (end.tv_usec - start.tv_usec) / 1000.0f;
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%f ms\n",
               ntohs(iphdr -> tot_len)-IP4_HDRLEN, destinationIP, icmphdrP -> un.echo.sequence++, iphdr -> ttl, milliseconds);
    }
    return bytes_received;
}

//===================
// Calculate checksum
//===================

unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry-outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}