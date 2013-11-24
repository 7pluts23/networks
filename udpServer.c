/*
 * File: udpServer.c
 * Authors: Rizwan Ahmad & Stephen Pluta 
 * Description: This file acts as a server which receives requests from the client
 *				modeled on udpClient.c and serves a string to the client as necessary
 */

#include <stdio.h>		/* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset() */
#include <unistd.h>		/* for close() */
#include <time.h>		/* for srand(time()) */

#define ECHOMAX 255		/* Longest string to echo */
#define NUMCLIENTS 10

struct request{
	char client_ip[16]; 		/* To hold client IP address in dotted decimal */
	int inc;					/* Incarnation number of client */
	int client;					/* Client number */
	int req;					/* Request number */
	char c;						/* Random character client sends to server */
};

/* Structure which is used to keep track of past clients */
struct client{
	char processorNumber[16];	/* Holds the IP address */
	int inc;					/* Incarnation number */
	int client;					/* Client number */
	int req;					/* Request number */
};

void DieWithError(const char *errorMessage) /* External error handling function */
{
	perror(errorMessage);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;							/* Socket */
	struct sockaddr_in echoServAddr;	/* Local address */
	struct sockaddr_in echoClntAddr;	/* Client address */
	unsigned int cliAddrLen;			/* Length of incoming message */
	char echoBuffer[ECHOMAX];			/* Buffer for echo string */
	struct request clientRequest;		/* Request received from the client */
	struct client clients[NUMCLIENTS];	/* Array used as the client table */
	unsigned short echoServPort;		/* Server port */
	int recvMsgSize;					/* Size of received message */
	char message[5];					/* String that holds the server data */
	int i, tableSize;					/* Loop counter and size of client table */
	int found;							/* bool used if client is in client table */
	int fail, drop, noRespond;			/* variables used when simulating failures */

	if (argc != 2)			/* Test for correct number of parameters */
	{
		fprintf(stderr,"Usage:	%s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

	echoServPort = atoi(argv[1]);	/* First arg:  local port */
	for(i = 0; i < 5; i++) {		/* Initialize message so it is empty */
		message[i] = ' ';
	}
	
	tableSize = 0;					/* Initially the client table is empty */
	drop = 0;
	noRespond = 0;
	srand(time(0));					/* Seed the random character generator */

	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));		/* Zero out structure */
	echoServAddr.sin_family = AF_INET;					/* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);	/* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort);		/* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("bind() failed");
  	
	for (;;)	/* Run forever */
	{
		found = 0;
		
		/* Randomly decide if there should be a failure */
		fail = rand()%10;
		if(fail == 9)
			drop = 1;
		else if(fail == 10)
			noRespond = 1;
			
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(echoClntAddr);

		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, &clientRequest, sizeof(clientRequest), 0,
			(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
			DieWithError("recvfrom() failed");

		printf("Handling client...\nIP: %s\nIncarnation Number: %d\nClient Number: %d\nRequest number: %d\nData: %c\n\n", clientRequest.client_ip, clientRequest.inc, clientRequest.client, clientRequest.req, clientRequest.c);
		
		/* Process the request */
		for(i = 0; i < NUMCLIENTS; i++) {
			/* If it was decided to drop the request, do nothing */
			if(drop) {
				found = 1;
				break;
			}
			
			/* If the client table is empty, break from the loop */
			if(!tableSize) {
				break;
			}
			
			/* See if the current request client exists in the client table */
			if(clientRequest.inc == clients[i].inc && clientRequest.client == clients[i].client && 
			!strcmp(clientRequest.client_ip, clients[i].processorNumber)) {
				
				/* If the client is in the table, and the received request is less than
				 * the last received request from that client, ignore it */
				if(clientRequest.req < clients[i].req) {
					found = 1;
					break;
				}
				/* If the client is in the table, and the received request is equal to
				 * the last received request from the client, send the string without changing it */
				else if(clientRequest.req == clients[i].req) {
					found = 1;
					
					if(noRespond)
						break;
					
					/* Send unaltered message back to the client */
					if (sendto(sock, message, sizeof(message), 0, 
					(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(message))
						DieWithError("sendto() sent a different number of bytes than expected");
					break;
				}
				/* If the client is in the table, and the received request is greater than
				 * the last received request from the client, add the character to the message
				 * and send the message back */
				else {
					found = 1;
					
					/* Add the character to the message string */
					clients[i].req = clientRequest.req;
					for(i = 4; i > 0; i--) {
						message[i] = message[i-1];
					}
					message[0] = clientRequest.c;
					
					if(noRespond)
						break;
					
					/* Send updated message back to the client */
					if (sendto(sock, message, sizeof(message), 0, 
					(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(message))
						DieWithError("sendto() sent a different number of bytes than expected");
					break;
				}
			}
		}
		
		/* If the client was not found in the table, add it to the table, add the character
		 * to message and send the message string back */
		if(!found) {
			clients[tableSize].inc = clientRequest.inc;
			clients[tableSize].client = clientRequest.client;
			clients[tableSize].req = clientRequest.req;
			strcpy(clients[tableSize].processorNumber, clientRequest.client_ip);
			
			/* Add the character to the message string */
			clients[i].req = clientRequest.req;
			for(i = 4; i > 0; i--) {
				message[i] = message[i-1];
			}
			message[0] = clientRequest.c;
			
			/* Send updated message back to the client */
			if (sendto(sock, message, sizeof(message), 0, 
			(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(message))
				DieWithError("sendto() sent a different number of bytes than expected");
			
			tableSize = (tableSize+1)%NUMCLIENTS;			
		}
		
		/* If a failure was simulated, print the failure and reset the boolean */
		if(drop) {
			printf("Failure: Dropping request\n");
			drop = 0;
		}
		if(noRespond) {
			noRespond = 0;
			printf("Failure: Not responding\n");
		}
	}
	/* NOT REACHED */
}
