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


#define DEFAULT_INTERVAL 6
#define DEFAULT_MSG "0"
#define NUM_PACKETS 300

int R = 2*DEFAULT_INTERVAL;
int allowed_sending = 0;



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



void *threadfunction1() {

	int socketFD, sockfd2;
	int rate = DEFAULT_INTERVAL;
	char msg[141];
	
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
	
	if (getaddrinfo("10.10.66.90", "56790", &hints, &servinfo) != 0 || servinfo == NULL) {
	    printf("Error: Could not find router.");
	    exit(1);
	}


	

	int count = 1;
	int i;
	double timestamp; 
	while (count <= NUM_PACKETS && allowed_sending == 1) { 

		for (i=0; i<128; i++)
			msg[i] = '1';   // message content 
		msg[128] = '1';   // flow number
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
    printf("sent done\n");
}
	


void *threadfunction2() {
	struct timespec sleepTime = {5, 0};

	while (1) {
		if ((allowed_sending) == 1)
			allowed_sending = 0;
		else 
			allowed_sending = 1;
		sleepTime.tv_nsec = poisson((float)R)*1000000;
		nanosleep(&sleepTime, NULL);
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
	

