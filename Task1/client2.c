#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>


#define DEFAULT_INTERVAL 3
#define DEFAULT_MSG "0"
#define WINDOW_SIZE 10
#define NUM_PACKETS 300

int R = DEFAULT_INTERVAL;
int numACK = 1;



int poisson(float lamda) {
	int k = 0;
	float p = 1, L, u;
	L= exp(-lamda);
	while (1) {
		k++;
		u = (float) rand()/RAND_MAX;
		p = p * u;
		if (p < L) 
			break;
	}
	return k-1;
}



void *threadfunction1(){

	int socketFD, sockfd2;
	int rate = DEFAULT_INTERVAL;
	char msg[141];
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	
	struct timespec sleepTime = {0, 100};//10000000
	
	
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
	
	if (getaddrinfo("10.10.65.209", "56790", &hints, &servinfo) != 0 || servinfo == NULL) {
	    printf("Error: Could not find router.");
	    exit(1);
	}


	

	int count = 0;
	int memory = count;
	int i;
	double timestamp; 
	while (count <= memory+WINDOW_SIZE && count < NUM_PACKETS || numACK < NUM_PACKETS) { 
		if (count==memory+WINDOW_SIZE || count == NUM_PACKETS) {
			count = numACK;
			memory = count;
		}
		for (i=0; i<128; i++)
			msg[i] = '2';   // message content 
		msg[128] = '2';   // flow number
		memcpy(msg+129, &count, 4);
		timestamp = clock()/(double)CLOCKS_PER_SEC;

		memcpy(msg+133, &timestamp, 8);
		

		if (sendto(socketFD, msg, 141, 0, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
			printf("Sending failed!\n");
			exit(1);
		}

		sleepTime.tv_nsec = poisson((float)R)*1000000;
		nanosleep(&sleepTime, NULL);
		count += 1;
	}
}









void *threadfunction2() {
	int socketFD;
	int ack;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_in clientinfo;
	int clientinfolen;

	
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
	
	if(getaddrinfo("10.10.67.189", "56890", &hints, &servinfo) != 0 || servinfo == NULL){
	    printf("Error: Could not get connection info with router.\n");
	    exit(1);
	}


	if(bind(socketFD, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
		printf("Error: Could not bind socket.\n");
		exit(1);
	}
	
	
	while (1) {
		clientinfolen = sizeof(struct sockaddr_in);
		if (recvfrom(socketFD, &ack, 4, 0, &clientinfo, &clientinfolen) == -1) {
			printf("Receiving failed!\n");
			exit(1);
		}
		numACK = ack;
	}
}
	
	


int main(int argc, char *argv[]) {

	pthread_t threadID1, threadID2;
	void *exit_status;


	pthread_create(&threadID1, NULL, threadfunction1, NULL);
	pthread_create(&threadID2, NULL, threadfunction2, NULL);
	pthread_join(threadID1, &exit_status);
	pthread_join(threadID2, &exit_status);	

	return 0;
}

