/* TODO: 
 * - Client IP address								√
 * - Incarnation number - increment with probability 0.5 after a random number of reqs sent	
 * - Request logic on client and server side		√
 * - Simulating failures							
 * - Serverside: keeping track of P, I, C values 	√
 * - Client dealing with timeouts
 */

#include <stdio.h>		/* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_addr() */
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset() */
#include <unistd.h>		/* for close() */
#include <time.h>		/* for srand(time()) */
#include <netdb.h>

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

int GetIP(char *IP) {
	char hostname[128];
	struct addrinfo hints, *res;
	struct in_addr addr;
	int err;

	gethostname(hostname, sizeof(hostname));
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
		return 1;
	}

	addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
	
	strcpy(IP, inet_ntoa(addr));
	
	freeaddrinfo(res);
	
	printf("IP: %s\n", IP);
	
	return 0;
}

int main(int argc, char *argv[])
{
	int sock;							/* Socket descriptor */
	struct sockaddr_in echoServAddr;	/* Echo server address */
	struct sockaddr_in fromAddr;		/* Source address of echo */
	unsigned short echoServPort;		/* Echo server port */
	unsigned int fromSize;				/* In-out of address size for recvfrom() */
	char *servIP;						/* IP address of server */
	struct request clientRequest;		/* Pointer to clientRequest */
	struct timeval timer;				/* Timeval struct for timeouts */
	char echoBuffer[6];					/* Buffer for receiving echoed string */
	char clientIP[16];					/* String that holds the client's IP */
	int echoStringLen;					/* Length of string to echo */
	int respStringLen;					/* Length of received response */
	int i;								/* Loop counter */

	if ((argc < 3) || (argc > 4))		/* Test for correct number of arguments */
	{
		fprintf(stderr,"Usage: %s <Server IP> <Port Number> <Client Number>\n", argv[0]);
		exit(1);
	}

	servIP = argv[1];			/* First arg: server IP address (dotted quad) */

	if (argc == 4)
		echoServPort = atoi(argv[2]);	/* Use given port, if any */
	else
		echoServPort = 7;	/* 7 is the well-known port for the echo service */
	
	srand(time(0));						/* Seed the random character generator */
	timer.tv_sec = 5;					/* Set timeout to five seconds */
	timer.tv_usec = 0;
	
	if(GetIP(clientIP) != 0)
		DieWithError("Error retrieving IP Address");
	

	/* Create a datagram/UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");
	
	/* Set the timeout interval of the socket */
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timer, sizeof(struct timeval)) != 0)
		DieWithError("setsockopt() failed");

	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));		/* Zero out structure */
	echoServAddr.sin_family = AF_INET;					/* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);	/* Server IP address */
	echoServAddr.sin_port	= htons(echoServPort);		/* Server port */

	int k;
	
	do {

		k = rand();
		if(k%2 == 0) {
			k = 20;
		}

	for(i = 0; i < k%21; i++) {

		strcpy(clientRequest.client_ip, clientIP);
		clientRequest.inc = 10;
		clientRequest.client = atoi(argv[3]);
		clientRequest.req = i;
		clientRequest.c = rand()%26+97;
		
		/* Send the request to the server */
		if (sendto(sock, &clientRequest, sizeof(clientRequest), 0, (struct sockaddr *)
		&echoServAddr, sizeof(echoServAddr)) != sizeof(clientRequest))
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
		echoBuffer[5] = '\0';
		printf("Received %d: %s\n", i+1, echoBuffer);	/* Print the echoed arg */
		sleep(0.1);
	}
	} while (k != 20);
	
	close(sock);
	exit(0);
}
