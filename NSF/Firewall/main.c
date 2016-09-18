#include "main.h"
#include <stdio.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


void processPacket(uint8_t *data, int dataLen, int protocol) {
    if(protocol != TUNNELING_PROTOCOL) return;

    int ipHeaderLen = sizeof(struct iphdr);
    struct iphdr *ipheader = (struct iphdr *) (data + ipHeaderLen); /* After outter IP header */
    printIPHeader(ipheader);
    ///////////////conduct inspection //////////////////////////

    int metadataNum = 0;
    int resultHeaderLen = (2 /*resultCode(1) + metadataNum(1)*/ + metadataNum ) * sizeof(uint8_t);
    int packetLen = ipHeaderLen + resultHeaderLen + (dataLen - ipHeaderLen);

    uint8_t *packet = (uint8_t *)malloc(packetLen);
    uint8_t metadataCodes[1] = {DDOS_INSPECTION};

    /* Attach our header & outter trunneling header */
    attach_outter_encapsulation(packet, FIREWALL_IP, SFF_IP, TUNNELING_PROTOCOL, packetLen);
    attach_inspection_result((packet + ipHeaderLen), DENY, metadataNum, metadataCodes);
    memcpy((void *)(packet + ipHeaderLen + resultHeaderLen), (void *)(data + ipHeaderLen), dataLen - ipHeaderLen); //Deteach IP Header of outter encapsulation in data
    
    /* Send inspection result packet to SFF */
    sendPacket(packet); 

    free(packet);
}

int main(int argc, char *args[]) {

    if(argc < 2) {
        printf("Usage: nsf [interface name]\n");
        return -1;
    }

    printf("Start Firewall %s\n", args[1]);
    start_listening(args[1], &processPacket);
}
