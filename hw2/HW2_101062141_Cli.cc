#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <sys/select.h>
#include <sys/time.h>


#define MAXLINE 4096
#define connect 0

void dg_cli(FILE *fp, int sockfd, struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (argc != 3){
		printf("usage: tcpcli <IP address> <Port>\n");
		exit(0);
	}
	int SERV_PORT=atoi(argv[2]);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("Error");
	}
	dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	exit(0);
}
void dg_cli(FILE *fp, int sockfd, struct sockaddr *pservaddr, socklen_t servlen) {
	int n;
	char sendline[MAXLINE], recvline[MAXLINE + 1];
	sendline[0] = connect;
	sendto(sockfd, sendline, 1, 0, pservaddr, servlen);
	if((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0){
		printf("timeout\n");		
	}
	recvline[n] = '\0';
	/* null terminate */
	//printf("%s %d\n",recvline,strlen(recvline));
	fputs(recvline, stdout);
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		//printf("%d\n",sizeof(struct sockaddr_in));
		sendline[strlen(sendline)-1] = '\0';
		sendto(sockfd, sendline, strlen(sendline)+1, 0, pservaddr, servlen);
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		recvline[n] = '\0';
		/* null terminate */
		fputs(recvline, stdout);
	}
}
