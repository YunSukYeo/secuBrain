#include "nsf-sff-interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <errno.h>
/*
	BUF_LEN, SFF_LISTEN_PORT, SFF_LISTEN_ADDR

*/
bool start_listening() {

	int source_address_size , data_size, destination_address_size, bytes_sent;
    struct sockaddr_ll source_address, destination_address;
    unsigned char *buffer=malloc(65535);

    int sock_raw = socket( PF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ; //For receiving
    int sock = socket( PF_PACKET , SOCK_RAW , IPPROTO_RAW) ;            //For sending

    memset(&source_address, 0, sizeof(struct sockaddr_ll));
    source_address.sll_family = AF_PACKET;
    source_address.sll_protocol = htons(ETH_P_ALL);
    source_address.sll_ifindex = if_nametoindex("sff0");
    if (bind(sock_raw, (struct sockaddr*) &source_address, sizeof(source_address)) < 0) {
        perror("bind failed\n");
        close(sock_raw);
    }

    memset(&destination_address, 0, sizeof(struct sockaddr_ll));
    destination_address.sll_family = AF_PACKET;
    destination_address.sll_protocol = htons(ETH_P_ALL);
    destination_address.sll_ifindex = if_nametoindex("eth0");
    if (bind(sock, (struct sockaddr*) &destination_address, sizeof(destination_address)) < 0) {
      perror("bind failed\n");
      close(sock);
    }
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth0");
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("bind to eth0");
        }

    while(1)
    {
        source_address_size = sizeof (struct sockaddr);
        destination_address_size = sizeof (struct sockaddr);
        //Receive a packet
        data_size = recvfrom(sock_raw , buffer , 65536 , 0 ,(struct sockaddr *) &source_address , (socklen_t*)&source_address_size);

        if(data_size <0 )
        {
            printf("Recvfrom error , failed to get packets\n");
            return 1;
        }
        else{
        	printf("Received %d bytes\n",data_size);

        //Huge code to process the packet (optional)

        //Send the same packet out
        bytes_sent=write(sock,buffer,data_size);
        printf("Sent %d bytes\n",bytes_sent);
         if (bytes_sent < 0) {
            perror("sendto");
            exit(1);
         }

        }
    }
    close(sock_raw);
}