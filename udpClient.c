#include <stdio.h>		/* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_addr() */
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset() */
#include <unistd.h>		/* for close() */

#define ECHOMAX 255		/* Longest string to echo */

struct request{
	char client_ip[16]; /* To hold client IP address in dotted decimal */
	int inc;			/* Incarnation number of client */
	int client;			/* Client number */
	int req;			/* Request number */
	char c;				/* Random character client sends to server */
};

void DieWithError(const char *errorMessage)	/* External error handling function */
{
	perror(errorMessage);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;							/* Socket descriptor */
	struct sockaddr_in echoServAddr;	/* Echo server address */
	struct sockaddr_in fromAddr;		/* Source address of echo */
	unsigned short echoServPort;		/* Echo server port */
	unsigned int fromSize;				/* In-out of address size for recvfrom() */
	char *servIP;						/* IP address of server */
	struct request clientReqPointer;	/* Pointer to clientRequest */
	char echoBuffer[5];					/* Buffer for receiving echoed string */
	int echoStringLen;					/* Length of string to echo */
	int respStringLen;					/* Length of received response */

	if ((argc < 3) || (argc > 4))		/* Test for correct number of arguments */
	{
		fprintf(stderr,"Usage: %s <Server IP> <Port Number> <Client Number>\n", argv[0]);
		exit(1);
	}
	
	strcpy(clientReqPointer.client_ip, "111.111.111.111");
	clientReqPointer.inc = 10;
	clientReqPointer.client = atoi(argv[3]);
	clientReqPointer.req = 0;
	clientReqPointer.c = 'a';	

	servIP = argv[1];			/* First arg: server IP address (dotted quad) */

	if (argc == 4)
		echoServPort = atoi(argv[2]);	/* Use given port, if any */
	else
		echoServPort = 7;	/* 7 is the well-known port for the echo service */

	/* Create a datagram/UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));		/* Zero out structure */
	echoServAddr.sin_family = AF_INET;					/* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);	/* Server IP address */
	echoServAddr.sin_port	= htons(echoServPort);		/* Server port */

	/* Send the string to the server */
	if (sendto(sock, &clientReqPointer, sizeof(clientReqPointer), 0, (struct sockaddr *)
		&echoServAddr, sizeof(echoServAddr)) != sizeof(clientReqPointer))
			DieWithError("sendto() sent a different number of bytes than expected");
  
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, 5, 0, 
	(struct sockaddr *) &fromAddr, &fromSize)) != 5)
		DieWithError("recvfrom() failed");

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	/* null-terminate the received data */
	//echoBuffer[respStringLen] = '\0';
	printf("Received: %s\n", echoBuffer);	/* Print the echoed arg */
	
	close(sock);
	exit(0);
}
