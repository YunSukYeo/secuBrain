#ifndef __CONSTANTS__
#define __CONSTANTS__


#define bool int
#define true 1
#define false 0

#define SFF_INTERFACE "sff0"
#define NSF_INTERFACE "eth0"
#define BUF_LEN 128

/*-------------Inspection Result----------------*/
#define PASS 0x00000000
#define DENY 0x00000001
#define MIRROR 0x00000002
#define ADVANCED 0x00000003

/*-------------Metadata Code For Advanced Action-------------*/
#define DDOS_INSPECTION 0x00000000
#define DPI_INSPECTION 0x00000001

/*-------------IP Setting------------*/
#define SFF_IP "10.0.0.100"
#define FIREWALL_IP "10.0.0.200"

/*-------------Protocol Definition------------*/
#define SENDER_PROTOCOL 145
#define TUNNELING_PROTOCOL 146

/*-------------Running Interface Mode-----------------*/
#define SFF_MODE 0
#define FIREWALL_MODE 1
#define DDOS_MITIGATOR_MODE 2
#define DPI_MODE 3


#endif
