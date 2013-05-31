#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define NUM_PACKETS 300

int received[NUM_PACKETS];


int repeat(int seq) {
	if (received[seq] == 1)
		return 1;
	return 0;
}


int main(int argc, char *argv[]){

	int socketFD;
	char buf[141];
	int bytesReceived;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_in clientinfo;
	int clientinfolen;

	int packetsReceived1 = 0;

	
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
	
	if(getaddrinfo("169.254.204.75", "56890", &hints, &servinfo) != 0 || servinfo == NULL){
	    printf("Error: Could not get connection info with router.\n");
	    exit(1);
	}


	if(bind(socketFD, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
		printf("Error: Could not bind socket.\n");
		exit(1);
	}
	

	int j = 0;
	for (;j<NUM_PACKETS;j++)
		received[j] = 0;

	
	while (1) {
		clientinfolen = sizeof(struct sockaddr_in);
		if (recvfrom(socketFD, &buf, 141, 0, &clientinfo, &clientinfolen) == -1) {
			printf("Receiving failed!\n");
			exit(1);
		}

		

		printf("Flow: %c\n", buf[128]);
		printf("Data: %c\n", buf[0]);
		printf("Count: %d\n", *(int *)(buf+129));
		
		if (!(repeat(*(int *)(buf+129))))
			packetsReceived1 += 1;
			received[*(int *)(buf+129)] = 1;

		printf("Time Stamp: %lf\n", *(double *)(buf+133));
		printf("%d\n", packetsReceived1);
		printf("\n");
	}
}

	
	

