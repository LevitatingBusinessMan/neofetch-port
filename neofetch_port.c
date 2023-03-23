#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv) {
	uint16_t port = 1337;

	int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        return 1;
    }	

	struct sockaddr_in server;

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	int opton_value = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opton_value , sizeof(int)) < 0) {
		perror("Error setting socket_fd option");
		return 1;
	}

	//Bind and check for errors
	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Error binding to port");
		return 1;
	}

	if (listen(server_fd, 3) < 0) {
        perror("Error listening");
        return 1;
	}
    
	printf("Socket listening at %d\n", port);
	
	socklen_t addr_len = sizeof(server);

	int client_fd;
	while ((client_fd = accept(server_fd, (struct sockaddr *)&server, &addr_len))) {

		struct sockaddr_in src_addr;
		socklen_t addr_size;
		if (getpeername(client_fd, (struct sockaddr *)&src_addr, &addr_size) > 0) {
			perror("Error getting src ip");
			return 1;
		}

		char src_ip[INET_ADDRSTRLEN];
		strcpy(src_ip, inet_ntoa(src_addr.sin_addr));

		printf("Received connection from %s\n", src_ip);

		FILE* neofetch_fp = popen("/usr/bin/env neofetch", "r");
		if (neofetch_fp == NULL) {
			perror("Error executing neofetch");
    		return 1;
		}

		// neofetch | wc -c
		// is 1701 (on my machine)

		char buf[2000];
		int nread;
		if ((nread = fread(buf,  sizeof(*buf), sizeof(buf), neofetch_fp)) < 1) {
			fprintf(stderr, "Failed reading neofetch output");
			return 1;
		}

		write(client_fd, buf, nread);

		pclose(neofetch_fp);
		close(client_fd);
	}
}
