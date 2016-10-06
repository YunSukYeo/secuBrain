#include "nsf-sff-interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <string.h>
#include <net/if.h>

#define MAX_IP_SIZE 65535
#define MAX_UDP_PAYLOAD_SIZE 65507
#define PROTOCOL_NUM 145
#define ETHER_TYPE 0x0800
#define SFF_IP "10.0.0.100"
#define FIREWALL_IP "10.0.0.200"

//#define ETH_FRAME_LEN 1518

void attach_outter_encapsulation_and_send(char *srcIP, char *destIP, char *contents, int dataLen) {
    int sockfd = socket(PF_INET, SOCK_RAW, PROTOCOL_NUM);
    int ipHeaderLen = sizeof(struct iphdr);
    int packetLen = ipHeaderLen + dataLen;
    char *packet = (char *)malloc(packetLen);

    printf("ipHeaderLen: %d\n", ipHeaderLen);
    printf("Packet Size: %d\n", packetLen);
    memset(packet, 0, packetLen);

    if(sockfd < 0) {
         printf("socket() error\n");
         return;
    }

    int turn_on = 1;
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &turn_on, sizeof(turn_on)) < 0) {
        printf("setsockopt() error\n");
        return;
    }

    struct iphdr iphdr;     /* Source */
    struct sockaddr_in din; /* Destination */

    din.sin_family = AF_INET;
    din.sin_port = 0;
    din.sin_addr.s_addr = inet_addr(destIP);

    iphdr.ihl = 5;
    iphdr.version = 4;
    iphdr.tos = 0;
    iphdr.tot_len = htons(packetLen);
    iphdr.id = htons(rand() % 65535);
    iphdr.frag_off = 0;
    iphdr.ttl = 64;
    iphdr.protocol = PROTOCOL_NUM;
    iphdr.saddr = inet_addr(srcIP);
    iphdr.daddr = inet_addr(destIP);

    memcpy(packet, &iphdr, ipHeaderLen);
    memcpy((packet + ipHeaderLen), contents, dataLen);

    if(sendto(sockfd, packet, packetLen, 0,
            (struct sockaddr*) &din, sizeof(din)) < 0) {
        printf("sendto() error\n");
        return;
    }

    free(packet);
}

bool start_ehternet_listening(char *ifName) {
    printf("ifname: %s\n", ifName);
    fflush(stdout);
   
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE));
    if(sockfd == -1) {
        printf("socket() error\n");
        fflush(stdout);
        return false;
    }

    char buffer[ETH_FRAME_LEN] = {0};
    char sender[INET6_ADDRSTRLEN];
    int receivedBytes = 0;
    int sockopt;

    struct ether_header *eh = (struct ether_header *) buffer;
    struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ether_header));
    struct ifreq ifopts;
    struct sockaddr_storage sender_addr;

    /* promisicuous mode */
    strncpy(ifopts.ifr_name, ifName, strlen(ifName) + 1);
    ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
    //ifopts.ifr_flags |= IFF_PROMISC;
    //ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

    /* Allow the socket to be reused - incase connection is closed prematurely */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    /* Bind to device */
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, strlen(ifName)) == -1)	{
        perror("SO_BINDTODEVICE");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int packetCounter = 0;
    while(true) {
        printf("------------------------Packet: %d-----------------------------\n", packetCounter++);
        fflush(stdout);
        receivedBytes = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
        if(receivedBytes == -1) {
            printf("recvfrom() error\n");
            return false;
        }
        buffer[receivedBytes] = '\0';

        printf("Received Bytes: %d\n", receivedBytes);
        printf("Destination MAC: %x:%x:%x:%x:%x:%x\n",
                                        eh->ether_dhost[0],
                                        eh->ether_dhost[1],
                                        eh->ether_dhost[2],
                                        eh->ether_dhost[3],
                                        eh->ether_dhost[4],
                                        eh->ether_dhost[5]);

        ((struct sockaddr_in *)&sender_addr)->sin_addr.s_addr = iph->saddr;
        inet_ntop(AF_INET, &((struct sockaddr_in*)&sender_addr)->sin_addr, sender, sizeof sender);

        printf("iphdr.ihl = %d\n", iph->ihl);
        printf("iphdr.version = %d\n", iph->version);
        printf("iphdr.tos = %d\n", iph->tos);
        printf("iphdr.tot_len = %d\n", htons(iph->tot_len));
        printf("iphdr.frag_off = %d\n", iph->frag_off);
        printf("iphdr.ttl = %d\n", iph->ttl);
        printf("iphdr.protocol = %d\n", iph->protocol);
        printf("ipAddress: %s\n", sender);

        if(iph->protocol == PROTOCOL_NUM)
            attach_outter_encapsulation_and_send(SFF_IP, FIREWALL_IP, (char *)(iph), receivedBytes - sizeof(struct ether_header));
        fflush(stdout);
    }
    return true;
}


bool start_ip_listening(char *hostName, char *hostIP) {
    printf("Start %s:%s listening\n", hostName, hostIP);
    fflush(stdout);

    int sd = socket(PF_INET, SOCK_RAW, PROTOCOL_NUM);
    if (sd < 0) {
        printf("socket() error\n");
        return -1;
    }

    char buffer[MAX_IP_SIZE] = {0};
    int receivedBytes = 0;
    struct ifreq ifopts;
    ifopts.ifr_flags |= IFF_PROMISC;
    ioctl(sd, SIOCSIFFLAGS, &ifopts);
    while(true) {
        if((receivedBytes = recv(sd, buffer, MAX_IP_SIZE, 0)) < 0) {
            printf("recvfrom() failed\n");
            return -1;
        }
        buffer[receivedBytes] = '\0';


        printf("read succeeded\n");
        printf("pkt_size = %d\n", receivedBytes);

        struct iphdr* iphdr = (struct iphdr*) buffer;
        printf("iphdr.ihl = %d\n", iphdr->ihl);
        printf("iphdr.version = %d\n", iphdr->version);
        printf("iphdr.tos = %d\n", iphdr->tos);
        printf("iphdr.tot_len = %d\n", htons(iphdr->tot_len));
        printf("iphdr.frag_off = %d\n", iphdr->frag_off);
        printf("iphdr.ttl = %d\n", iphdr->ttl);
        printf("iphdr.protocol = %d\n", iphdr->protocol);
        fflush(stdout);
    }

    return true; 
}


bool start_udp_listening(char *hostName, char *hostIP, int serverPort) {
    printf("start listening from %s %s:%d\n", hostName, hostIP, serverPort);
    int udpSocket, nBytes;
    char buffer[MAX_UDP_PAYLOAD_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size, client_addr_size;
    int i;

    /* Create UDP socket */
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /* Configure settings in address struct */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(hostIP);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    addr_size = sizeof(serverAddr);
    /* Bind socket with address struct */
    if((bind(udpSocket, (struct sockaddr *) &serverAddr, addr_size)) < 0) {
        printf("Bind Failed\n");
        close(udpSocket);
    }

    client_addr_size = sizeof(clientAddr);
    while(true) {
        nBytes = recvfrom(udpSocket, buffer, MAX_UDP_PAYLOAD_SIZE, 0, (struct sockaddr *) &clientAddr, &client_addr_size);
        buffer[nBytes] = '\0';
        if(strcmp(buffer, "end") == 0) {
            printf("Termination Signal Received\n");
            fflush(stdout);
            break;
        } else {
            printf("%s: %s\n", hostName, buffer);
            fflush(stdout);
        }
    }
    close(udpSocket);
    return true;
}
