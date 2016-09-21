#include "main.h"
#include "../../Interfaces/mysql-interface.h"
#include "../../Interfaces/nsf-sff-interface.h"
#include <stdio.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <arpa/inet.h>

uint32_t ipStringToInt(char *ipString) {
    return inet_addr(ipString);
}

char* ipIntToString(uint32_t ipInt) {
    return inet_ntoa(*(struct in_addr *)&ipInt);
}

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
    attach_inspection_result((packet + ipHeaderLen), PASS, metadataNum, metadataCodes);
    memcpy((void *)(packet + ipHeaderLen + resultHeaderLen), (void *)(data + ipHeaderLen), dataLen - ipHeaderLen); //Deteach IP Header of outter encapsulation in data
    
    /* Send inspection result packet to SFF */
    sendPacket(packet); 

    free(packet);
}

void mysqlSelectCallback(MYSQL_RES *sqlResult) {
    int rowNum = MysqlGetNumRows(sqlResult);
    int i = 0;

    for(; i < rowNum; i++) {
        MYSQL_ROW row = MysqlGetRow(sqlResult);
        printf("idx: %s, sourceIP: %s, destinationIP: %s, action: %s\n", row[0], row[1], row[2], row[3]);
   }
}

/* Insert query has no result */
void mysqlInsertCallback(MYSQL_RES *sqlResult /* NULL */) {
    printf("Insert Success\n");
}

/* Update query has no result */
void mysqlUpdateCallback(MYSQL_RES *sqlResult /* NULL */) {
    printf("Update Success\n");
}

/* Delete query has no result */
void mysqlDeleteCallback(MYSQL_RES *sqlResult /* NULL */) {
    printf("Delete Success\n");
}


int main(int argc, char *args[]) {

    if(argc < 2) {
        printf("Usage: nsf [interface name]\n");
        return -1;
    }

    /* Firewall-SFF Connection */
    printf("Start Firewall %s\n", args[1]);
    start_listening(args[1], &processPacket);

    if(MysqlInitialize()) {
        // MysqlInsertQuery("firewall_rule", "saddr, daddr, action", "1523, 1342, 128", mysqlInsertCallback);
        // MysqlUpdateQuery("firewall_rule", "saddr, daddr", "111, 111", "1=1", mysqlUpdateCallback);
        // MysqlDeleteQuery("firewall_rule", "idx=1", mysqlDeleteCallback);
        // MysqlSelectQuery("firewall_rule", "idx, saddr, daddr, action", NULL, 1, mysqlSelectCallback);

    }
}
