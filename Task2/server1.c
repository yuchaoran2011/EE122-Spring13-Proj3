#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <limits.h>

int main(int argc, char *argv[]){

	int socketFD;
	char buf[141];
	int bytesReceived;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_in clientinfo;
	int clientinfolen;

	int count;

	int packetsReceived1 = 0, packetsReceived2 = 0;

	
	//Get connection info
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;


	socketFD = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketFD < 0){
		printf("Error: Could not get socket.\n");
		exit(1);
	}
	
	if(getaddrinfo("10.10.66.90", "56890", &hints, &servinfo) != 0 || servinfo == NULL){
	    printf("Error: Could not get connection info with router.\n");
	    exit(1);
	}


	if(bind(socketFD, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
		printf("Error: Could not bind socket.\n");
		exit(1);
	}


	while (1) {

		clientinfolen = sizeof(struct sockaddr_in);
		if (recvfrom(socketFD, &buf, 141, 0, &clientinfo, &clientinfolen) == -1) {
			printf("Receiving failed!\n");
			exit(1);
		}

		packetsReceived1 += 1;
		printf("Flow: %c\n", buf[128]);
		printf("Count: %d\n", *(int *)(buf+129));
		printf("Time Stamp: %lf\n", *(double *)(buf+133));
		printf("%d\n", packetsReceived1);
		printf("\n");
	}	
}

	
	

