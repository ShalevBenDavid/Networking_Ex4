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
#define MAX_SIZE 200

unsigned short calculate_checksum(unsigned short *paddress, int len); // Checksum Method.
ssize_t sendPing (int, char*, struct sockaddr_in);
int receivePong(int sock, char *, struct sockaddr_in, socklen_t, struct timeval, struct timeval);

int main(int argc, char *argv[])
{
    struct icmp icmphdr; // ICMP-header
    char data[] = "This is the ping.\n";
    int dataLen = strlen(data) + 1;

    //===================
    // ICMP header
    //===================

    /*------------------------------ Initialize icmphdr ------------------------------*/
    icmphdr.icmp_type = ICMP_ECHO;      // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_code = 0;    // Message Code (8 bits): echo request
    icmphdr.icmp_id = 18;    // Identifier (16 bits): some number to trace the response.
    icmphdr.icmp_seq = 0;    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_cksum = 0;  // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation


    char packet[MAX_SIZE];     // Combine the packet
    memcpy((packet), &icmphdr, ICMP_HDRLEN);     // Next, ICMP header
    memcpy(packet + ICMP_HDRLEN, data, dataLen);     // After ICMP header, add the ICMP data.

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + dataLen);
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    /*------------------------------ Create dest_in ------------------------------*/
    struct sockaddr_in dest_in; // Struct for holding the info for destination.
    memset(&dest_in, 0, sizeof(struct sockaddr_in)); // Resting "dest_in".
    dest_in.sin_family = AF_INET; // Defining as IPv4 communication.
    dest_in.sin_addr.s_addr = inet_addr(argv[1]); // Assigning it the IP address we got as an argument.
    // The port is irrelevant for Networking and therefore was zeroed.

    /*------------------------------ Create Raw Socket ------------------------------*/
    int sock;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) // Check if we were successful in creating the socket.
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

    /*------------------------------ Send Ping And Get The Pong Response ------------------------------*/
    socklen_t len = sizeof(dest_in); // Save the dest_in length.
    struct timeval start, end;
    ssize_t bytes_received = -1; // Will help keep track of the bytes received.
    char reply[MAX_SIZE] = {0}; // A buffer to hold the replay (pong).

    // An infinite do-while loop to send ping and receive pong.
    do {
        // Start the timer.
        gettimeofday(&start, 0);
        sendPing (sock, packet, dest_in);
        bytes_received = receivePong(sock, reply, dest_in, len, start, end);
        sleep(1); // Wait for a second before repeating process.
    } while (bytes_received != 0);

    // Close the raw socket descriptor.
    close(sock);

    return 0;
}

//===================
//Send Ping
//===================
ssize_t sendPing (int sock, char* packet, struct sockaddr_in dest_in) {
    // Send the packet using sendto() for sending datagrams.
    ssize_t bytes_sent = sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
    if (bytes_sent == -1)
    {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    }
    return bytes_sent;
};

//===================
// Receive Pong
//===================
int receivePong(int sock, char* packet, struct sockaddr_in dest_in, socklen_t len, struct timeval start, struct timeval end) {
    // Receive the packet using recvfrom() for receiving datagrams.
    int bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len);
    gettimeofday(&end, 0); // End the timer.
    if (bytes_received > 0)
    {
        struct iphdr *iphdr = (struct iphdr *)packet;
        struct icmphdr *icmphdrP = (struct icmphdr *)(packet + sizeof(iphdr));
        char destinationIP[32] = { '\0' };
        inet_ntop(AF_INET, &dest_in.sin_addr.s_addr, destinationIP, sizeof(destinationIP));

        float milliseconds = (float )(end.tv_sec - start.tv_sec) * 1000.0f +
                             (float) (end.tv_usec - start.tv_usec) / 1000.0f;
        printf("%d bytes recieved from %s: icmp_seq=%d ttl=%d time=%f ms\n",
               ntohs(iphdr -> tot_len), destinationIP, icmphdrP -> un.echo.sequence++, iphdr -> ttl, milliseconds);
    }
    return bytes_received;
};


//------------------------------- Compute checksum -------------------------------
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