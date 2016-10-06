#ifndef __NSF_SFF_INTERFACE__
#define __NSF_SFF_INTERFACE__

#include "constants.h"

    bool start_ethernet_listening(char *ifname);
    bool start_ip_listening(char *hostName, char *hostIP);
    bool start_udp_listening(char *hostName, char *hostIP, int serverPort);

#endif
