



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <semaphore.h>

#define MSG_SIZE 40			// message size

sem_t sem;

void* readMessages(void*);
void* sendMessages(void*);

void displayChoice();
void sendCommand();
void printHistory();

char buffer[MSG_SIZE],ip[MSG_SIZE],receivedIP[MSG_SIZE], ipHolder[MSG_SIZE];
char broadcastMSG[MSG_SIZE];

int sock, length, n, r;
int boolval = 1;		// for a socket option
// Use this to find IP
struct ifreq ifr;
char eth0[] = "wlan0";
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in from; // From the client
int port_number;
int flagg = 0;

typedef struct {
    char* time;
	char* switchStatus;
	char* buttonStatus;
	char* LEDsStatus;
	char* adcValue;
	char* event;
}logList; 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

logList allLogs[1000];


void sendCommand();
void printStatus();
int main(void){
	int option;
	int option;
	pthread_t sender, reader;
	
	
	if (argc == 2){
        port_number = atoi(argv[1]);
    } else {
        port_number = 2000; // set default if not provided
    }

	// Creates socket. Connectionless.
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        error("Opening socket");
	}

    length = sizeof(server);                // length of structure
    bzero(&server,length);                  // sets all values to zero. memset() could be used
    server.sin_family = AF_INET;            // symbol constant for Internet domain
    server.sin_port = htons(port_number);	// port number
    server.sin_addr.s_addr = INADDR_ANY;	// IP address of the machine on which the server is running



    // gets the host name and the IP
	bzero(&ifr, sizeof(ifr));	// Set all values to zero
    ifr.ifr_addr.sa_family = AF_INET;	// Type of address to retrieve - IPv4 IP address
    strncpy((char* )&ifr.ifr_name , eth0 , IFNAMSIZ-1);		// Copy the interface name in the ifreq structure
	// Get IP address
    if (ioctl(sock,SIOCGIFADDR,&ifr) == -1) {
		error("Cannot get IP address");
		close(sock);
		exit(0);
	}
	// Converts the internet host address in network byte order to a string in IPv4 dotted-decimal notation
    strcpy(ip,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    strcpy(ipHolder,ip);
    int current_board_number = parseIP(ipHolder);
    printf("IP : %s\n",ip);
    printf("Board Number : %d\n",current_board_number);


    // binds the socket to the address of the host and the port number
    if (bind(sock, (struct sockaddr *)&server, length) < 0){
        error("binding");
	}
    // set broadcast option
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
   	{
        printf("error setting socket options\n");
        exit(-1);
   	}
    fromlen = sizeof(struct sockaddr_in);	// size of structure
	
	
	sem_init(&sem, 0, 1);
	//Start the pthreads
	//pthread_create(&sender, NULL, sendMessages, NULL);
	pthread_create(&reader, NULL, readMessages, NULL);
	while(1){
		displayChoice();
		scanf("%d",option);
	
		handleUserInput(option);
		switch(option){
			case 1:
				sendCommand();
			case 2:
				printStatus();
			case 3:
				break;
		}
	}
	return 0;
}

void displayChoice(){
	printf("\nHistorian Program options: \n");
	printf("1)Send a command to the RTUs\n");
	printf("2)Print the event history since start\n");
	printf("3)Exit\n");
}

void printStatus(){
	
}

void sendCommand(){
	bzero(broadcastMSG, MSG_SIZE);
	
	printf("Command option:\n LED1ON\n LED2ON \n LED3ON \n LED1OFF\n LED2OFF \n LED3OFF \n Please enter the command:");
	scanf("%s",broadcastMSG);
	
	from.sin_addr.s_addr = inet_addr("128.206.19.255");
	n = sendto(sock, broadcastMSG, MSG_SIZE, 0, (struct sockaddr *)&from, fromlen);
}

void* readMessages(void* agr){
	int count = 1;
	while(1){
		logList log;
		bzero(&buffer,MSG_SIZE); // clear the buffer to NULL
		n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, &fromlen);
		if(!strncmp(buffer, "LED1ON", 6) || !strncmp(buffer, "LED2ON", 6) || !strncmp(buffer, "LED3ON", 6)){}
		else if(!strncmp(buffer, "LED1OFF", 6) || !strncmp(buffer, "LED2OFF", 6) || !strncmp(buffer, "LED3OFF", 6)){}
		else{
			printf("%s\n",buffer);
			if(count == 1){
				log.time = buffer;
			}
			else if(count == 2){
				log.switchStatus = buffer;
			}
			else if(count == 3){
				log.buttonStatus = buffer;
			}
			else if(count == 4){
				log.LEDsStatus = buffer;
			}
			else if(count == 5){
				log.adcValue = buffer;
			}
			else if(count == 6){
				log.event = buffer;
			}
			
			sem_wait(&sem);
				allLogs[i] = log;
				allLogs[i+1] = NULL;
			sem_post(&sem);
		}
		count++;
	}
}

logList *sortLogEntry(char* buffer){
	
	logList localLogs[1000];
	
	sem_wait(&sem);
		memcpy(localLogs,allog, sizeof(logList));
	sem_post(&sem);
	
	
	 int i, j;
   for (i = 0; i < n-1; i++)      
   {
		for (j = 0; j < n-i-1; j++) {
		  if (arr[j] > arr[j+1])
            swap(&arr[j], &arr[j+1]);
		}    
   }
     
	
	return log;
}