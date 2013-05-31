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


char received[NUM_PACKETS];
int counter = 0;


double uniform(double a, double b)
{
	return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}


int ACK() {
	int j = 0;
	while (received[j] != 0 && j<NUM_PACKETS)
		j++;
	return j;
}



void *threadfunction1(){

	int socketFD;
	char buf[141];
	int bytesReceived;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_in clientinfo;
	int clientinfolen;

	int packetsReceived1 = 0, packetsReceived2 = 0;
	struct timespec sleepTime = {0, 100};
	int b;

	
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
	
	if(getaddrinfo("10.10.67.189", "56800", &hints, &servinfo) != 0 || servinfo == NULL){
	    printf("Error: Could not get connection info with router.\n");
	    exit(1);
	}


	if(bind(socketFD, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
		printf("Error: Could not bind socket.\n");
		exit(1);
	}
	
	b = 5;
	int temp;
	int count = 0;
	while (1) {
		clientinfolen = sizeof(struct sockaddr_in);
		if (count>=5000) {
			if (b==5) {
				b = 15;
			} else {
				b =5;
			}
			temp = uniform(0,b);
			count = temp;
		} else {
			temp = uniform(0,b);
			count += temp;
		}
		

		sleepTime.tv_nsec = temp*1000000;
		nanosleep(&sleepTime, NULL);

		if (recvfrom(socketFD, &buf, 141, 0, &clientinfo, &clientinfolen) == -1) {
			printf("Receiving failed!\n");
			exit(1);
		}
		
		printf("Flow: %c\n", buf[128]);
		printf("Data: %c\n", buf[0]);
		printf("Count: %d\n", *(int *)(buf+129));

		if (received[*(int *)(buf+129)] == 0) {
			received[*(int *)(buf+129)] = 1;
			packetsReceived1 += 1;
		}

		printf("Time Stamp: %lf\n", *(double *)(buf+133));
		printf("%d\n", packetsReceived1);
		printf("\n");
	}
}







void *threadfunction2() {
	int socketFD, sockfd2;
	char msg[141];
	int ack;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	
	struct timespec sleepTime = {0, 100};
	
	
	//Get sockets
	socketFD = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket < 0){
		printf("Error: Could not get socket.");
		exit(1);
	}
	
	//Get connection info
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo("10.10.64.227", "56700", &hints, &servinfo) != 0 || servinfo == NULL) {
	    printf("Error: Could not find router.");
	    exit(1);
	}
	

	ack = ACK();
	while (ack <= NUM_PACKETS) { 

		if (sendto(socketFD, &ack, 4, 0, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
			printf("Sending failed!\n");
			exit(1);
		}
		ack = ACK();
		printf("%d\n", ack);
	}
}





int main(int argc, char *argv[]) {

	pthread_t threadID1, threadID2;
	void *exit_status;

	for (;counter<NUM_PACKETS; counter++)
		received[counter] = 0;

	pthread_create(&threadID1, NULL, threadfunction1, NULL);
	pthread_create(&threadID2, NULL, threadfunction2, NULL);
	pthread_join(threadID1, &exit_status);
	pthread_join(threadID2, &exit_status);
	
	return 0;
}

	

	
	

