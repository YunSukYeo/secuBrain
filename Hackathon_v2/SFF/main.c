#include "main.h"
#include <stdio.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void processPacket(uint8_t *data, int dataLen, int protocol) {
    if(protocol != SENDER_PROTOCOL && protocol != TUNNELING_PROTOCOL) return;

    int ipHeaderLen = sizeof(struct iphdr);
    if(protocol == SENDER_PROTOCOL) {
        printf("\nReceived From Sender\n");

        int packetLen = ipHeaderLen + dataLen;
        uint8_t *packet = (uint8_t *) malloc(packetLen);

        attach_outter_encapsulation(packet, SFF_IP, FIREWALL_IP, TUNNELING_PROTOCOL, packetLen);

        struct iphdr *iphdr = (struct iphdr *)data;
        printIPHeader(iphdr);

        memcpy((void *)(packet + ipHeaderLen), (void *)data, dataLen);
        sendPacket(packet);
        free(packet);
    } else if (protocol == TUNNELING_PROTOCOL) {
         printf("\nReceived From NSF\n");

        struct iphdr *outter_iphdr = (struct iphdr *)data;
        uint8_t actionCode = *(uint8_t *)(data + ipHeaderLen);
        uint8_t metadataNum = *(uint8_t *)(data + ipHeaderLen + 1);
        int i = 0;

        printf("\n\n/**********INSPECTION RESULT**********/\n");
        printf("actionCode: %02x, metadataNum: %02x\n", actionCode, metadataNum);
        for (i = 0; i < metadataNum; i++) {
            printf("metaData: %02x\n", *(uint8_t *)(data + ipHeaderLen + 2 + i));
        }
        printf("/*************************************/\n");

        struct iphdr *origin_iphdr = (struct iphdr *)(data + ipHeaderLen + 2 + metadataNum);
        printIPHeader(origin_iphdr);

        if(actionCode == PASS) {
            sendPacket(data + ipHeaderLen + (2 + metadataNum) * sizeof(uint8_t));
        } else if (actionCode == DENY) {
            // Drop Packet & report
        } else if (actionCode == MIRROR) {
            sendPacket(data + ipHeaderLen + (2 + metadataNum) * sizeof(uint8_t));
            // Do more inspection
        } else if (actionCode == ADVANCED) {
            // Do more inspection
        }
    }
}

int main(int argc, char *args[]) {

    if(argc < 2) {
        printf("Usage: nsf [interface name]\n");
        return -1;
    }

    printf("Start SFF %s\n", args[1]);
    start_listening(args[1], &processPacket);
}
