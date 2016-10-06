#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "udpPacketGenerator.h"

int main(int argc, char* argv[]) {
  if(argc < 6) {
    printf("Usage: ./generator interface srcIPv4Address destIPv4Address srcPort destPort");
  }
  char interface[20], srcIPv4Address[20], destIPv4Address[20];


  generateUdpPacket(argv[1], argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), 
    "TestMessage");
}
