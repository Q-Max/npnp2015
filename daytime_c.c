#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/

#define SERV_PORT 12346
#define MAXLINE 4096

void str_cli(FILE *fp, int sockfd);
int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;
	if (argc != 2)printf("usage: tcpcli <IPaddress>");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	connect(sockfd, (__CONST_SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	str_cli(stdin, sockfd); /* do it all */
	exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
	char sendline[MAXLINE]="READY", recvline[MAXLINE];
	write(sockfd, sendline, strlen (sendline));
	if (read(sockfd, recvline, MAXLINE) == 0) {
		printf("str_cli: server terminated prematurely");
		exit(0);
	}
	/*printf("get string ");*/
	fputs(recvline, stdout);
	/*
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		printf("send string %s\n",sendline);
		write(sockfd, sendline, strlen (sendline));
		if (read(sockfd, recvline, MAXLINE) == 0) {
			printf("str_cli: server terminated prematurely");
			exit(0);
		}
		printf("get string ");
		fputs(recvline, stdout);
	}*/
}