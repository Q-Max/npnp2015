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
#define SERV_PORT 12346

int main(int argc, char **argv)
{
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE],j;
	ssize_t n;
	fd_set rset, allset;
	char line[MAXLINE];
	char hello[]="Hello!\n\0";
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family= AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port= htons(SERV_PORT);
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	maxfd = listenfd;
	/* initialize */
	maxi = -1;
	/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)client[i] = -1;
	/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	for ( ; ; ) {
		rset = allset;
		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL); /*# of ready descriptors */
		if (FD_ISSET(listenfd, &rset)) {/* new client connection */

			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
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
					//close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else {
					printf("receive : %s\n",line);
					write(sockfd, line, n);
					for(j=0;j<10;j++){
						printf("send : %s\n",hello);
						write(sockfd, hello, (ssize_t)strlen(hello)+1);
						usleep(1000000);
					}
				}
				if (--nready <= 0)
				break;
				/* no more readable descriptors */
			}
		}
	}
}