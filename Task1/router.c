#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#define QUEUE1_SIZE 32
#define QUEUE2_SIZE 32

                 
int socketFD, socketFD2;
char msg[141];

struct addrinfo hints, hints2;
struct addrinfo *servinfo, *servinfo2;
int socketFD, socketFD2;
char *IP1 = "10.10.67.189", *IP2 = "10.10.67.189", *IP3 = "10.10.67.189", *IP;
char *Port1 = "56790", *Port2 = "56890", *Port3 = "56800", *Port;

int i,j, pktsent1=0, pktrecvd1=0, pktrecvd2=0, pktsent2=0;
int sum1=0, sum2=0, count1=0, count2=0;

struct addrinfo *out1Info = NULL, *out2Info = NULL;




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





/* A linked list structure  */
struct queueNode
{
    char * msg;
    struct queueNode *prev;
    struct queueNode *next;
};
struct queueNode *head1, *middle1, *middle2, *head2, *tail1, *tail2,*temp;




void *threadfunction1()
	{
		struct sockaddr_in from;
		int from_len;
		struct queueNode *current;
		int length;


		socketFD = socket(AF_INET, SOCK_DGRAM, 0);
		if (socketFD < 0){
			printf("Error: Could not get socket.\n");
			exit(1);
		}
	
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;
	
		if(getaddrinfo(IP1, Port1, &hints, &servinfo) != 0 || servinfo == NULL){
		    printf("Error: Could not find self-info.\n");
		    exit(1);
		}

		if (bind(socketFD, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
			printf("Error: Could not bind.\n");
			exit(1);
		}



		while(1) {	
			recvfrom(socketFD, &msg, 141, 0, &from, &from_len);	
			current = (struct queueNode*)malloc(sizeof(struct queueNode));
			printf("%c\n", msg[128]);
			if (msg[128] == '1') {
				length = pktrecvd1 - pktsent1;
				if (length > QUEUE1_SIZE) {

					sum1 += QUEUE1_SIZE;
					count1 += 1;
					printf("Queue1 avg size: %f\n", ((float)sum1)/count1);
					printf("Queue2 avg size: %f\n", ((float)sum2)/count2);
					continue;
				}
				pktrecvd1 += 1;	
				length+=1;
				sum1 += length;
				count1 += 1;
				printf("Queue1 avg size: %f\n", ((float)sum1)/count1);
				printf("Queue2 avg size: %f\n", ((float)sum2)/count2);		
				current->msg = msg;
				current->prev = head1;
				current->next = head1->next;
				head1->next->prev = current;
				head1->next = current;
			}
			else {
				length = pktrecvd2 - pktsent2;
				if (length > QUEUE2_SIZE) {
					//pktrecvd2 += 1;
					sum2 += QUEUE2_SIZE;
					count2 += 1;
					printf("Queue1 avg size: %f\n", ((float)sum1)/count1);
					printf("Queue2 avg size: %f\n", ((float)sum2)/count2);
					continue;
				}
				pktrecvd2 += 1;
				length += 1;		
				sum2 += length;
				count2 += 1;
				printf("Queue1 avg size: %f\n", ((float)sum1)/count1);
				printf("Queue2 avg size: %f\n", ((float)sum2)/count2);
				//printf("%c\t%d\n",msg[1], length);
				current->msg = msg;
				current->prev = head2;
				current->next = head2->next;
				head2->next->prev = current;
				head2->next = current;
			}	
		}
		return NULL;
}





void *threadfunction2() {
	char * queue;
	int index = 0;
	float L = 10;
	double timestamp; 
	struct timespec sleepTime = {0, 10000000};

	//Get connection info
	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family = AF_INET;
	hints2.ai_socktype = SOCK_DGRAM;
	hints2.ai_flags = AI_PASSIVE;
	

	socketFD2 = socket(AF_INET, SOCK_DGRAM, 0);
		if (socketFD2 < 0){
			printf("Error: Could not get socket2.\n");
			exit(1);
		}

	
	char * tosend = NULL;
	while (1) {
		if (tail1->prev->prev != head1) {
			temp = tail1->prev;
			IP = IP2;
			Port = Port2;
			middle1 = tail1->prev->prev;
			middle1->next = tail1;
			tail1->prev = middle1;
			tosend = middle1->msg;	
			pktsent1 += 1;		
		} 
		else if (tail2->prev->prev != head2) {
			temp = tail2->prev;
			IP = IP3;
			Port = Port3;
			middle2 = tail2->prev->prev;
			middle2->next = tail2;
			tail2->prev = middle2;
			tosend = middle2->msg;			
			pktsent2 += 1;
		}
		else {
			continue;
		}


		if (getaddrinfo(IP, Port, &hints2, &servinfo2) != 0 || servinfo2 == NULL) {
			printf("Error: Could not get connection info.\n");
			exit(1);
		}

		if (sendto(socketFD2, tosend, 141, 0, servinfo2->ai_addr, servinfo2->ai_addrlen) < 0) {
			printf("Sending failed!\n");
			exit(1);
		}
		free(temp);

		sleepTime.tv_nsec = poisson(L) * 1000000;
		nanosleep(&sleepTime, NULL);
	}
}






int main(int argc, char *argv[]) {

	pthread_t threadID1, threadID2;
	void *exit_status;
	
	head1 = (struct queueNode*)malloc(sizeof(struct queueNode));
	tail1 = (struct queueNode*)malloc(sizeof(struct queueNode));
	middle1 = (struct queueNode*)malloc(sizeof(struct queueNode));
	head1->prev = head1;
	head1->next = middle1;
	head1->msg = NULL;
	middle1->prev = head1;
	middle1->next = tail1;
	middle1->msg = NULL;
	tail1->prev = middle1;
	tail1->next = tail1;
	tail1->msg = NULL;

	head2 = (struct queueNode*)malloc(sizeof(struct queueNode));
	tail2 = (struct queueNode*)malloc(sizeof(struct queueNode));
	middle2 = (struct queueNode*)malloc(sizeof(struct queueNode));
	head2->prev = head2;
	head2->next = middle2;
	head2->msg = NULL;
	middle2->prev = head2;
	middle2->next = tail2;
	middle2->msg = NULL;
	tail2->prev = middle2;
	tail2->next = tail2;
	tail2->msg = NULL;


	pthread_create(&threadID1, NULL, threadfunction1, NULL);
	pthread_create(&threadID2, NULL, threadfunction2, NULL);
	pthread_join(threadID1, &exit_status);
	pthread_join(threadID2, &exit_status);
	

	return 0;
}

	
	

