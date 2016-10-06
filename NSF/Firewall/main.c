#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nsf-secu-controller-interface.h"
#include "../../Interfaces/nsf-sff-interface.h"
#include "../../Interfaces/mysql-interface.h"

int getCurrentTime() {
    /*TODO - implement time function, return current hour*/
}

void processPacket(uint8_t *data, int dataLen, int protocol) {
    if(protocol != TUNNELING_PROTOCOL) return;

    int ipHeaderLen = sizeof(struct iphdr);
    struct iphdr *ipheader = (struct iphdr *) (data + ipHeaderLen); /* After outter IP header */
            printIPHeader(ipheader);
    char where[256] = {0}, action = PASS;
    int n = sprintf(where, "`saddr`=\"%lu\" AND `daddr`=\"%lu\" AND `stime`<%d AND `etime`>%d"
                                , (unsigned long)(ipheader->saddr), (unsigned long)(ipheader->daddr), 13, 13);
    where[n] = '\0';
    MYSQL_RES *sqlResult = MysqlSelectQuery("firewall-rule", "`action`", where, false);

    if(MysqlGetNumRows(sqlResult) > 0) {
        MYSQL_ROW row = MysqlGetRow(sqlResult);
        action = (int) *(row[0]);
    }

    int metadataNum = 0;
    int resultHeaderLen = (2 /*resultCode(1) + metadataNum(1)*/ + metadataNum ) * sizeof(uint8_t);
    int packetLen = ipHeaderLen + resultHeaderLen + (dataLen - ipHeaderLen);

    uint8_t *packet = (uint8_t *)malloc(packetLen);
    uint8_t metadataCodes[1] = {DDOS_INSPECTION};

    /* Attach our header & outter trunneling header */
    attach_outter_encapsulation(packet, FIREWALL_IP, SFF_IP, TUNNELING_PROTOCOL, packetLen);
    attach_inspection_result((packet + ipHeaderLen), action, metadataNum, metadataCodes);
    memcpy((void *)(packet + ipHeaderLen + resultHeaderLen), (void *)(data + ipHeaderLen), dataLen - ipHeaderLen); //Deteach IP Header of outter encapsulation in data 

    /* Send inspection result packet to SFF */
    sendPacket(packet);

    free(packet);
}


void *nsf_sff_interface(void *arg) {
    char *interfaceName = (char *) arg;
    start_listening(interfaceName, &processPacket);
    return NULL;
}


int main(int argc, char *argv[]) {
    if(argc < 2) { 
        printf("Usage: ./firewall [interface name] \n");
        exit(-1);
    }

    pthread_t nsf_sff_interface_thread;
    int thread_id;

    if(MysqlInitialize()) {
        thread_id = pthread_create(&nsf_sff_interface_thread, NULL, nsf_sff_interface, (void *)argv[1]);
        if(thread_id < 0) {
            printf("thread create error \n");
            exit(-1);
        }

        start_confd();
    } else {
        fprintf(stderr, "\nMysqlInitialize() Failed\n");
    }


    return 0;
}
