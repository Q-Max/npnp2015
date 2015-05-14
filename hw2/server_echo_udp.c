#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <sys/select.h>
#include <time.h>
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>

#define LISTENQ 256
#define MAXLINE 4096

int max(int num1, int num2) ;
int main(int argc, char **argv)
{
	int i, maxi, maxfd, listenfd, connfd, sockfd,udpfd;
	const int on=1;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	socklen_t len;
	fd_set rset, allset;
	char mesg[MAXLINE];
	char dst[100];
	if (argc != 2){
		printf("usage: serv <Port>\n");
		exit(0);
	}
	int SERV_PORT=atoi(argv[1]);
	char line[MAXLINE];
	//socklen_t clilen;
	// struct sockaddr_in cliaddr, servaddr;
	// listenfd = socket(AF_INET, SOCK_STREAM, 0);
	// bzero(&servaddr, sizeof(servaddr));
	// servaddr.sin_family= AF_INET;
	// servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// servaddr.sin_port= htons(SERV_PORT);
	//setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	// bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//listen(listenfd, LISTENQ);
	
	/* initialize */
	maxi = -1;
	/* index into client[] array */
	/* for create UDP socket*/
	struct sockaddr_in servaddr;
	udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	maxfd = udpfd + 1;
	for (i = 0; i < FD_SETSIZE; i++)client[i] = -1;
	/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	for ( ; ; ) {
		rset = allset;
		FD_SET(udpfd, &rset);
		 /*# of ready descriptors */
		
		if (FD_ISSET(udpfd, &rset)) {
			len = sizeof(servaddr);
			n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
			sendto(udpfd, mesg, n, 0, (struct sockaddr *) &servaddr, len);
		}
	}
}
int max(int num1, int num2) 
{
	if (num1 > num2)
		return num1;
	else
		return  num2;
	return 0; 
}
