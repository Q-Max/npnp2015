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
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family= AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port= htons(SERV_PORT);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	
	/* initialize */
	maxi = -1;
	/* index into client[] array */
	/* for create UDP socket*/
	udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	maxfd = max(listenfd,udpfd)+1;
	for (i = 0; i < FD_SETSIZE; i++)client[i] = -1;
	/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	for ( ; ; ) {
		rset = allset;
		FD_SET(listenfd, &rset);
		FD_SET(udpfd, &rset);
		/* structure assignment */
		if((nready = select(maxfd+1, &rset, NULL, NULL, NULL))<0){
			if(errno == EINTR)
				continue;
			else
				puts("select error");
		} /*# of ready descriptors */
		if (FD_ISSET(listenfd, &rset)) {/* new client connection */

			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;
					/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)printf("too many clients"); /*cannot accept clients*/
			FD_SET(connfd, &allset);
			if (connfd > maxfd)
				maxfd = connfd;
			if (i > maxi)
				maxi = i;
			if (--nready <= 0)
				continue;
		}
		for (i = 0; i <= maxi; i++) {
			if ( (sockfd = client[i]) < 0)
				continue;	/* check all clients for data */
					/*Skip empty client[]*/
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = read(sockfd, line, MAXLINE)) == 0) {
				/* connection closed by client */
				close(sockfd);
				FD_CLR(sockfd, &allset);
				client[i] = -1;
				} else{
					printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					puts(line);
					write(sockfd, line, n);
				}
				if (--nready <= 0)
				break;
				/* no more readable descriptors */
			}
		}
		if (FD_ISSET(udpfd, &rset)) {
			len = sizeof(cliaddr);
			n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
			sendto(udpfd, mesg, n, 0, (struct sockaddr *) &cliaddr, len);
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
