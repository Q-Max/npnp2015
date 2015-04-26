#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <sys/select.h>
#include <sys/time.h>

/*#define SERV_PORT 12346*/
#define MAXLINE 4096

void str_cli(FILE *fp, int sockfd);
int max(int num1, int num2) ;
int tx=0;
int main(int argc, char **argv)
{
	int SERV_PORT;
	int sockfd;
	
	struct sockaddr_in servaddr;
	if (argc != 3){
		printf("usage: tcpcli <IPaddress> <Port>");
		exit(0);
	}
	SERV_PORT=atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	connect(sockfd, (__CONST_SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	str_cli(stdin, sockfd); /* do it all */
	exit(0);
}

/*void str_cli(FILE *fp, int sockfd)
{
	char sendline[MAXLINE], recvline[MAXLINE];
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		printf("send string %s\n",sendline);
		write(sockfd, sendline, strlen (sendline));
		if (read(sockfd, recvline, MAXLINE) == 0) {
			printf("str_cli: server terminated prematurely");
			exit(0);
		}
		printf("get string ");
		fputs(recvline, stdout);
	}
}*/
void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, stdineof;
	fd_set rset;
	char sendline[MAXLINE], recvline[MAXLINE];
	stdineof = 0; /*use for test readable*/
	FD_ZERO(&rset);
	for ( ; ; ) {
		if (stdineof == 0)	FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) { /* socket is readable */
			if (read(sockfd, recvline, MAXLINE) == 0) {
				if (stdineof == 1)
					return;
				/* normal termination */
				else {
					printf("str_cli: server terminated prematurely");
					exit(0);
				}
			}
			printf("receive : ");
			fputs(recvline, stdout);
		}
		if (FD_ISSET(fileno(fp), &rset)) { /* input is readable */
			if (fgets(sendline, MAXLINE, fp) == NULL) { //EOF
				stdineof = 1;
				
				/* send FIN */
				FD_CLR(fileno(fp), &rset);
				continue;
			}
			if(!tx)write(sockfd, sendline, strlen(sendline));
			tx++;
			shutdown(sockfd, SHUT_WR);
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