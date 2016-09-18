#ifdef __UDP_PACKET_GENERATOR
#define __UDP_PACKET_GENERATOR
  int generateUdpPacket(char* interface, char* srcIPv4Address, char* destIPv4Address, int srcPort, int destPort, char* contents);
#endif
