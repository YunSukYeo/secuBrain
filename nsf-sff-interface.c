#include "nsf-sff-interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <errno.h>
/*
	BUF_LEN, SFF_LISTEN_PORT, SFF_LISTEN_ADDR

*/
bool start_listening() {
	char buffer[BUF_LEN];
	struct sockaddr_in server_addr, client_addr;
	char temp[20];
	int server_fd, client_fd;

	int len, msg_size, portno = 1;

	/* First call to socket() function */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Initialize socket structure */
	bzero((char *) &server_addr, sizeof(server_addr));
	portno = SFF_LISTEN_PORT;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SFF_LISTEN_ADDR);
	server_addr.sin_port = htons(portno);

	/* Now bind the host address using bind() call.*/
	if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		printf("SFF: ERROR on binding\n");
		exit(1);
	}

	if(listen(server_fd, 5) < 0) {
		printf("SFF: Can't listening connect\n");
		exit(0);
	}

	bzero((char *)buffer, sizeof(buffer));
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


		msg_size = read(client_fd, buffer, BUF_LEN);
		write(client_fd, buffer, msg_size);
		close(client_fd);
		printf("SFF: %s client closed.\n", temp);
	}
}