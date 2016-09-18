#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#define PROTOCOL_NUM 145

u_int16_t get_checksum(u_int16_t* buf, int nwords) {
	u_int32_t sum;
	for (sum=0; nwords>0; nwords--) sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (u_int16_t)(~sum);
}


int main(int argc, char* args[]) {
	if(argc < 3) {
		printf("usage: ./ipPacketGenerator [srcIP] [destIP]\n");
		return -1;
	}
	
	char *srcIP = args[1];
	char *destIP = args[2];

	printf("Send packet from %s to %s\n", srcIP, destIP);

	int sd = socket(PF_INET, SOCK_RAW, PROTOCOL_NUM);
	if(sd < 0) {
		printf("socket() error\n");
		return -1;
	}

	int turn_on = 1;
	int turn_off = 0;

	if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &turn_on, sizeof(turn_on)) < 0){
		printf("setsockopt() error\n");
		return -1;
	}

	struct iphdr iphdr;     /* Source */
	struct sockaddr_in din; /* Destination */

	din.sin_family = AF_INET;
	din.sin_port = 0;
	din.sin_addr.s_addr = inet_addr(destIP);

	iphdr.ihl = 5;
	iphdr.version = 4;
	iphdr.tos = 0;
	iphdr.tot_len = htons(sizeof(struct iphdr));
	iphdr.id = htons(rand() % 65535);
	iphdr.frag_off = 0;
	iphdr.ttl = 64;
	iphdr.protocol = PROTOCOL_NUM;
	iphdr.saddr = inet_addr(srcIP);
	iphdr.daddr = inet_addr(destIP);

	if(sendto(sd, &iphdr, sizeof(iphdr), 0,
		(struct sockaddr*) &din, sizeof(din)) < 0) {
		printf("sendto() error\n");
		return -1;
	}

	return 0;
}
