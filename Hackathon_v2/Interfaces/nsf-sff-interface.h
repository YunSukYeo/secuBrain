#ifndef __NSF_SFF_INTERFACE__
#define __NSF_SFF_INTERFACE__

#include "constants.h"
#include <stdint.h> /* uint_8 */
#include <netinet/ip.h>


typedef void (*PacketProcessFunction) (uint8_t *, int, int);

/* Interface Name, function(data, dataLength, protocol) */

bool start_listening(char *ifName, PacketProcessFunction processFunction);
//bool start_ethernet_listening(char *, PacketProcessFunction);

void sendPacket(uint8_t *packet);
void attach_outter_encapsulation(uint8_t *destBuffer, char *srcIP, char *destIP, int whichProtocol, int packetLen);
void attach_inspection_result(uint8_t *destBuffer, uint8_t actionCode, uint8_t metadataNum, uint8_t *metadataCodes);
void printIPHeader(struct iphdr *);

#endif
