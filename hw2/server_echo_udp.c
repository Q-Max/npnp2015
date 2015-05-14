#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* bzero()*/
#include <string.h> /* strcpy*/
#include <sys/socket.h> /* socket*/
#include <sys/select.h>
#include <time.h>
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>
#include <vector>
#include <algorithm>
#include <string>

#define LISTENQ 256
#define MAXLINE 4096
#define WELCOME "*************Welcome*****************\n[R]egister\t[L]ogin\n"

#define connect 0
#define regist 1
#define login 2
#define regist2 3
#define ack 4
struct account{
	unsigned int id;
	std::string name;
	std::string pw;
};

std::vector<account> accounts;

int max(int num1, int num2) ;
int main(int argc, char **argv)
{
	int i, maxi, maxfd, listenfd, connfd, sockfd,udpfd;
	const int on=1;
	unsigned int id = 1;
	int state = 0;
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
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	// if(setsockopt(udpfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	// 	perror("Error");
	// }
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
			if((n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0 ){
				printf("timeout\n");
			}
			// mesg[0] = ACK:
			// sendto(udpfd, mesg, 1, 0, (struct sockaddr *) &servaddr, len);
			printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( servaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(servaddr.sin_port));
			if(mesg[0]==connect){
				strcpy(mesg, WELCOME);
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(mesg[0]==regist){
				strcpy(mesg , "Please input your username and password by <username>/<password> : \n");
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(mesg[0]==login){
				strcpy(mesg, WELCOME);
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
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
