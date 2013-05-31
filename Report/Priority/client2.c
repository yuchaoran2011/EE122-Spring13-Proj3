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



int R = DEFAULT_INTERVAL;
int numACK = 1;
int WINDOW_SIZE = 60;
int indicator = 0;
double timePassed = 0.0, TIME_OUT = 0;


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
	double tstart, tend, ttime;
	
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
	
	if (getaddrinfo("169.254.204.75", "56790", &hints, &servinfo) != 0 || servinfo == NULL) {
	    printf("Error: Could not find router.");
	    exit(1);
	}


	

	int attempt = 0;
	int count = 1;
	int memory = count, temp = 0;
	int i;
	double timestamp; 
	while (numACK < NUM_PACKETS) {
        
		if (count==memory+WINDOW_SIZE || count == NUM_PACKETS) {
			count = numACK;
			memory = count;
			temp = indicator;
            
            
            tstart = (double)clock()/CLOCKS_PER_SEC;
            while (timePassed < TIME_OUT && indicator == temp) {
                tend = (double)clock()/CLOCKS_PER_SEC;

                timePassed = tend - tstart;
                //printf("%lf\n",TIME_OUT);
                //printf("Time passed : %lf\n",timePassed);
                //printf("insie while loop\n");
            }
            if (timePassed >= TIME_OUT)
            {
            	TIME_OUT *= 2;
            }
            
		}
		for (i=0; i<128; i++)
			msg[i] = '2';   // message content 
		msg[128] = '2';   // flow number
		memcpy(msg+129, &count, 4);

		timestamp = clock()/(double)CLOCKS_PER_SEC;
		memcpy(msg+133, &timestamp, 8);


		temp = indicator;
		//printf("%d\n",WINDOW_SIZE);

		if (sendto(socketFD, msg, 141, 0, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
			
			printf("Sending failed!\n");
			exit(1);
		}
        attempt += 1;
        if (attempt>=WINDOW_SIZE) {
            //printf("%f\n", (float)(numACK+(float)WINDOW_SIZE/10)/attempt);
        } else {
            //printf("%f\n", (float)(numACK)/attempt);
        }
        
		sleepTime.tv_nsec = poisson((float)R)*1000000;
		nanosleep(&sleepTime, NULL);
		count += 1;
        //printf("%d\n",numACK);
        if (numACK == NUM_PACKETS) {
            //printf("inside break\n");
            //printf("outside while\n");
            
            break;
        }
        
	}
}









void *threadfunction2() {
	int socketFD;
	int prev;
	int ackNum;
	double ts;
	char ack[12];

	
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
	
	if(getaddrinfo("169.254.94.80", "56700", &hints, &servinfo) != 0 || servinfo == NULL){
	    printf("Error: Could not get connection info with router.\n");
	    exit(1);
	}


	if(bind(socketFD, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
		printf("Error: Could not bind socket.\n");
		exit(1);
	}
	
	
	prev = 1;
	while (1) {
		clientinfolen = sizeof(struct sockaddr_in);

		if (recvfrom(socketFD, &ack, 12, 0, &clientinfo, &clientinfolen) == -1) {
			printf("Receiving failed!\n");
			exit(1);
		}

		memcpy(&ackNum,&ack,4);
		memcpy(&ts, ack+4, 8);
		//printf("%lf\n", ts);
		TIME_OUT = TIME_OUT * 0.875 + 0.125 * (clock()/(double)CLOCKS_PER_SEC - ts);
        //printf("%lf\n",TIME_OUT);
		

        

		indicator += 1;
		

		if (ackNum == prev) {
        
			if ((WINDOW_SIZE / 2)<=1) {
                WINDOW_SIZE = 1;
            } else if (ackNum !=1){
                WINDOW_SIZE /= 2;
            }
		} else {
			if (ackNum-prev >= WINDOW_SIZE) {
                
				prev = ackNum;
				WINDOW_SIZE += 1;
			}
		}
        if (numACK < ackNum) {
            numACK = ackNum;
        }
        printf("%d\n",WINDOW_SIZE);
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

