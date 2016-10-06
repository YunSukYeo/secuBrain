#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>


int main(int argc, char* argv[]) {
	printf("src: %s, dest: %s\n", argv[1], argv[2]);
	int s, j;

	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(s == -1) {
		printf("socket create error\n");
		return;
	}

	/*target address*/
	struct sockaddr_ll socket_address;

	/*buffer for ethernet frame*/
	void* buffer = (void*)malloc(ETH_FRAME_LEN);
 
	/*pointer to ethenet header*/
	unsigned char* etherhead = buffer;
	
	/*userdata in ethernet frame*/
	unsigned char* data = buffer + 14;
	
	/*another pointer to ethernet header*/
	struct ethhdr *eh = (struct ethhdr *)etherhead;
 
	int send_result = 0;

	/*our MAC address 00:0c:29:5e:de:b9*/
	unsigned char src_mac[6] = {0x00, 0x0c, 0x29, 0x5e, 0xde, 0xb9};

	/*other host MAC address b2:5d:71:d3:9f:bd*/
	unsigned char dest_mac[6] = {0xb2, 0x5d, 0x71, 0xd3, 0x9f, 0xbd};

	/*prepare sockaddr_ll*/

	/*RAW communication*/
	socket_address.sll_family   = PF_PACKET;	
	/*we don't use a protocoll above ethernet layer
	  ->just use anything here*/
	socket_address.sll_protocol = htons(ETH_P_IP);	

	/*index of the network device
	see full code later how to retrieve it*/
	socket_address.sll_ifindex  = 2;

	/*ARP hardware identifier is ethernet*/
	socket_address.sll_hatype   = ARPHRD_ETHER;
	
	/*target is another host*/
	socket_address.sll_pkttype  = PACKET_OTHERHOST;

	/*address length*/
	socket_address.sll_halen    = ETH_ALEN;		
	/*MAC - begin*/
	socket_address.sll_addr[0]  = 0xb2;
	socket_address.sll_addr[1]  = 0x5d;		
	socket_address.sll_addr[2]  = 0x71;
	socket_address.sll_addr[3]  = 0xd3;
	socket_address.sll_addr[4]  = 0x9f;
	socket_address.sll_addr[5]  = 0xbd;
	/*MAC - end*/
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/


	/*set the frame header*/
	memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	eh->h_proto = 0x00;
	/*fill the frame with some data*/
	for (j = 0; j < 1500; j++) {
		data[j] = (unsigned char)((int) (255.0*rand()/(RAND_MAX+1.0)));
	}

	/*send the packet*/
	send_result = sendto(s, buffer, ETH_FRAME_LEN, 0, 
		      (struct sockaddr*)&socket_address, sizeof(socket_address));
	if (send_result == -1) { 
		printf("send message failed\n");
	}
}
