#include "nsf-sff-interface.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "errno.h"
/*
	BUF_LEN, SFF_LISTEN_PORT, SFF_LISTEN_ADDR

*/
bool start_listening() {
	char buffer[BUF_LEN];
	struct sockaddr_in server_addr, client_addr;
	char temp[20];
	int server_fd, client_fd;

	int len, msg_size, option = 1;

	if((server_fd == socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("SFF: Cant't open stream socket\n");
		exit(0);
	}
	
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	memset(&server_addr, 0x00, sizeof(server_addr));

	/* server_addr setting */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SFF_LISTEN_ADDR); /*htonl(INADDR_ANY)*/;
	server_addr.sin_port = htons(3210);

	printf("2: %d\n", server_fd);
	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("SFF: Can't bind local address: %d\n", errno);
		exit(0);
	}

	if(listen(server_fd, 5) < 0) {
		printf("SFF: Can't listening connect: %d\n", errno);
		exit(0);
	}

	memset(buffer, 0x00, sizeof(buffer));
	printf("SFF: wating connection request.\n");
	len = sizeof(client_addr);

	while(true) {
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
		if(client_fd < 0) {
			printf("SFF: accept failed.\n");
			exit(0);
		}

		inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
		printf("SFF: %s client connected.\n", temp);


		msg_size = read(client_fd, buffer, 1024);
		write(client_fd, buffer, msg_size);
		close(client_fd);
		printf("SFF: %s client closed.\n", temp);
	}
}