#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>	
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>   
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>  
#include <semaphore.h>

#define MSG_SIZE 50

//PIN
#define BTN1 27;
#define BTN2 0;
#define S1 26;
#define S2 23;
#define RED 8;
#define YELLOW 9;
#define GREEN 7;

#define SPI_CHANNEL	      0	// 0 or 1
#define SPI_SPEED 	2000000	// Max speed is 3.6 MHz when VDD = 5 V
#define ADC_CHANNEL       1	// Between 0 and 3


//RTU infomation
int RTUNum;
int switch1, switch2;
int button1, button2;
int LED1, LED2, LED3;
float ADCValue;
int preSwitch1, preSwitch2;
int preButton1, preButton2;
int preLED1, preLED2, preLED3;

//socket arguments
int sock, length, n, num;
int boolval = 1;			// for a socket option
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in addr;

uint16_t get_ADC(int channel);	// prototype
time_t timep; //time record
char buffer[MSG_SIZE]; //to store reveived command
char buffer2[MSG_SIZE]; //to store sent status


void error(const char *msg)
{
     perror(msg);
     exit(0);
}

uint16_t get_ADC(int ADC_chan)
{
	uint8_t spiData[3];
	spiData[0] = 0b00000001; 
	spiData[1] = 0b10000000 | (ADC_chan << 4);
												
	spiData[2] = 0;	

	wiringPiSPIDataRW(SPI_CHANNEL, spiData, 3);
	
	return ((spiData[1] << 8) | spiData[2]);
}

void getTime()
{
    strcpy(buffer2, "RTU1:\n");  //or RTU2

    time(&timep);
    string s_temp = ctime(&timep);
    for (int i=11; i<=18; i++)
    {
        buffer2 += s_temp[i];
    }

    return;
}

void getSwitch()
{
    strcpy(buffer2, "Switch 1: ");
    if (switch1 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    buffer2 += "Switch 2: ";
    if (switch2 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    return;
}

void getButton()
{
    strcpy(buffer2, "Button 1: ");
    if (button1 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    buffer2 += "Button 2: ";
    if (button2 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    return;
}

void getLED()
{
    strcpy(buffer2, "LED 1: ");
    if (LED1 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    buffer2 += "LED 2: ";
    if (LED2 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    buffer2 += "LED 3: ";
    if (LED3 == 1) buffer2 += "ON\n";
    else buffer2 += "OFF\n";

    return;
}

void getADCValue()
{
    strcpy(buffer2, "ADC Value: ");
    
    uint16_t ADCvalue;
    ADCValue = -1;
    ADCvalue = get_ADC(ADC_CHANNEL);
    //ADCValue = ((3.300/1023)*ADCvalue)/2.0;
    char temp[MSG_SIZE];
    sprintf(temp, "%f", ADCValue);
    buffer2 += temp;
    buffer2 += "\n";

    return;
}

void* periodicUpdate(void *arg)
{
    while(1)
    {
            /******Check Status******/
            sem_wait(&my_sem);
            //switch status
            switch1 = digitalRead(S1);
            switch2 = digitalRead(S2);
            //button status
            button1 = digitalRead(BTN1);
            button2 = digitalRead(BTN2);
            //LED status
            LED1 = digitalRead(RED);
            LED2 = digitalRead(YELLOW);
            LED3 = digitalRead(GREEN);         
            sem_post(&my_sem);
            /************************/

            /******Sent message to Client******/
            //get status info
            bzero(buffer2, MSG_SIZE);
            getTime();
            n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
            bzero(buffer2, MSG_SIZE);
            getSwitch();
            n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);            
            bzero(buffer2, MSG_SIZE);
            getButton();
            n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
            bzero(buffer2, MSG_SIZE);
            getLED();
            n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
            bzero(buffer2, MSG_SIZE);
            getADCValue();
            n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);

            //if some events happend
            if (ADCValue>2 || ADCValue<1)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "Overload!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
            }

            if (ADCValue == -1)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "No Power!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
            }

            if (preSwitch1 != switch1)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "Switch 1 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
                preSwitch1 = switch1;          
            }

            if (preSwitch2 != switch2)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "Switch 2 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);
                preSwitch2 = switch2;          
            }

            if (preButton1 != button1)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "LED 1 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);          
                preButton1 = button1;
            }

            if (preButton2 != button2)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "Button 2 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);          
                preButton2 = button2;
            }

            if (preLED1 != LED1)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "LED 1 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);          
                preLED1 = LED1;
            }

            if (preLED2 != LED2)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "LED 2 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);          
                preLED2 = LED2;
            }

            if (preLED3 != LED3)
            {
                bzero(buffer2, MSG_SIZE);
                strcpy(buffer2, "LED 3 Change!\n");
                n = sendto(sock, buffer2, MSG_SIZE, 0, ( struct sockaddr*)&addr, fromlen);          
                preLED3 = LED3;
            }
            /**********************************/

            sleep(1);
    }
    pthread_exit(0);
}


int main(int argc, char *argv[])
{
    /******check if input has port number******/
    if (argc < 2)
    {
	    printf("usage: %s port\n", argv[0]);
        exit(0);
    }
    /******************************************/
    
    /******Setup******/
    if (wiringPiSetup()<0)
    {
        printf("wiringPi Setup failed!\n");
        exit(-1);
    }

    // Configure the SPI
	if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
		printf("wiringPiSPISetup failed\n");
		return -1 ;
	}
    /**************************/

    /******Initialization******/
    pinMode(S1, INPUT); //awitch 1 as input
    pinMode(S2, INPUT); //switch 2 as input

    pinMode(BTN1, INPUT); //button 1 as input
    pinMode(BTN2, INPUT); //button 2 as input

    pinMode(RED, OUTPUT);   //red LED as output
    pinMOde(YELLOW, OUTPUT);    //yellow LED as output
    pinMode(GREEN, OUTPUT); //green LED as output

    pullUpDnControl(S1, PUD_DOWN);
    pullUpDnControl(S2, PUD_DOWN);
    pullUpDNControl(BTN1, PUD_DOWN);
    pullUpDNControl(BTN1, PUD_DOWN);

    digitalWrite(RED, 0);
    digitalWrite(YELLOW, 0);
    digitalWrite(GREEN, 0);

    sem_init(&my_sem, 0, 1); //init semaphore

    preButton1=0;
    preButton2=0;
    preLED1=0;
    preLED2=0;
    preLED3=0;
    preSwitch1=0;
    preSwitch2=0;
    /**************************/

    /******Socket Connection******/
    sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
    if (sock < 0)
	    error("Opening socket");

    length = sizeof(server);			// length of structure
    bzero(&server,length);			// sets all values to zero. memset() could be used
    server.sin_family = AF_INET;		// symbol constant for Internet domain
    server.sin_addr.s_addr = INADDR_ANY;		// IP address of the machine on which
										    // the server is running
    server.sin_port = htons(atoi(argv[1]));	// port number

    // binds the socket to the address of the host and the port number
    if (bind(sock, (struct sockaddr *)&server, length) < 0)
        error("binding");

    // change socket permissions to allow broadcast
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
    {
        printf("error setting socket options\n");
   		exit(-1);
   	}

    fromlen = sizeof(struct sockaddr_in);	// size of structure
    /*****************************/


    /******Thread to send current RTU logs******/
    pthread_t ptr;
    pthread_create(&ptr, NULL, periodicUpdate, NULL);
    /*******************************************/

    /******Receive commands******/
    while(1)
    {
        bzero(buffer,MSG_SIZE);		// sets all values to zero. memset() could be used

	    // receive from a client
	    n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &fromlen);
        if (n < 0)
    	    error("recvfrom"); 

        printf("Received a command. It says: %s", buffer);

        // if the buffer from a cilent is WHOIS
        if(strcmp(buffer, "LED1ON\n") == 0)
        {
            digitalWrite(RED, 1);
            sem_wait(&my_sem);
            LED1 = 1;
            sem_post(&my_sem);
        }

        if(strcmp(buffer, "LED2ON\n") == 0)
        {
            digitalWrite(YELLOW, 1);
            sem_wait(&my_sem);
            LED2 = 1;
            sem_post(&my_sem);
        }

        if(strcmp(buffer, "LED3ON\n") == 0)
        {
            digitalWrite(GREEN, 1);
            sem_wait(&my_sem);
            LED3 = 1;
            sem_post(&my_sem);
        }

        if(strcmp(buffer, "LED1OFF\n") == 0)
        {
            digitalWrite(RED, 0);
            sem_wait(&my_sem);
            LED1 = 0;
            sem_post(&my_sem);
        }

        if(strcmp(buffer, "LED2OFF\n") == 0)
        {
            digitalWrite(YELLOW, 0);
            sem_wait(&my_sem);
            LED2 = 0;
            sem_post(&my_sem);
        }

        if(strcmp(buffer, "LED3OFF\n") == 0)
        {
            digitalWrite(GREEN, 0);
            sem_wait(&my_sem);
            LED3 = 0;
            sem_post(&my_sem);
        }
    }
    /****************************/

    return 0;
}