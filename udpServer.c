#include <stdio.h>		/* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset() */
#include <unistd.h>		/* for close() */

#define ECHOMAX 255		/* Longest string to echo */
#define NUMCLIENTS 10

struct request{
	char client_ip[16]; /* To hold client IP address in dotted decimal */
	int inc;			/* Incarnation number of client */
	int client;			/* Client number */
	int req;			/* Request number */
	char c;				/* Random character client sends to server */
};

struct client{
	char processorNumber[16];
	int inc;
	int client;
	int req;
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
	struct request clientRequest;
	struct client clients[NUMCLIENTS];
	unsigned short echoServPort;		/* Server port */
	int recvMsgSize;					/* Size of received message */
	char message[5];					/* String that holds the server data */
	int i, tableSize;					/* Loop counter and size of client table */
	int found;							/* bool used if client is in client table */

	if (argc != 2)			/* Test for correct number of parameters */
	{
		fprintf(stderr,"Usage:	%s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

	echoServPort = atoi(argv[1]);	/* First arg:  local port */
	for(i = 0; i < 5; i++) {		/* Initialize message so it is empty */
		message[i] = ' ';
	}
	
	tableSize = 0;

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
			
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(echoClntAddr);

		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, &clientRequest, sizeof(clientRequest), 0,
			(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
			DieWithError("recvfrom() failed");

		printf("Handling client...\nIP: %s\nIncarnation Number: %d\nClient Number: %d\nRequest number: %d\nData: %c\n\n", clientRequest.client_ip, clientRequest.inc, clientRequest.client, clientRequest.req, clientRequest.c);
		
		for(i = 0; i < NUMCLIENTS; i++) {
			if(clientRequest.inc == clients[i].inc && clientRequest.client == clients[i].client && 
			!strcmp(clientRequest.client_ip, clients[i].processorNumber)) {
				if(clientRequest.req < clients[i].req) {
					found = 1;
					break;
				}
				else if(clientRequest.req == clients[i].req) {
					found = 1;
					
					/* Send unaltered message back to the client */
					if (sendto(sock, message, sizeof(message), 0, 
					(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(message))
						DieWithError("sendto() sent a different number of bytes than expected");
					break;
				}
				else {
					found = 1;
					
					clients[i].req = clientRequest.req;
					for(i = 4; i > 0; i--) {
						message[i] = message[i-1];
					}
					message[0] = clientRequest.c;
					
					/* Send updated message back to the client */
					if (sendto(sock, message, sizeof(message), 0, 
					(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(message))
						DieWithError("sendto() sent a different number of bytes than expected");
					break;
				}
			}
		}
		
		if(!found) {
			clients[tableSize].inc = clientRequest.inc;
			clients[tableSize].client = clientRequest.client;
			clients[tableSize].req = clientRequest.req;
			strcpy(clients[tableSize].processorNumber, clientRequest.client_ip);
		}
	}
	/* NOT REACHED */
}
