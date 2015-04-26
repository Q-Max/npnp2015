#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* bzero()*/
#include <string.h>
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 256
#define MAXLINE 4096
#define SERV_PORT 12346

 void str_echo(int sockfd);

int main(int argc, char **argv)
{
	char dst[100];
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket (AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY); // for any interface
	servaddr.sin_port = htons (SERV_PORT); //user define
	bind(listenfd, (__SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (__SOCKADDR_ARG) &cliaddr, &clilen);
		/*const char *inet_ntop (int family, const void *addrptr, char *strptr, size_t len);*/
		
		printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
		if ( (childpid = fork()) == 0) { /* child process */
			close(listenfd);
			/* close listening socket */
			str_echo(connfd); /* process the request */
			exit (0); //end child process
		}
		close(connfd);
		/* parent closes connected socket */
		usleep(5000000);
		puts("QQ");
		if ( (childpid = fork()) == 0) { /* child process */
			system("netstat");
			
			exit (0); //end child process
		}
		wait(childpid);
		printf("netstat finished!!\n");

	 }
 }
 void str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAXLINE]; //MAXLINE is defined by user
	read(sockfd,buf,MAXLINE);
	/*printf("%s\n",buf);*/
	time_t t1=time(NULL);
	char *now=ctime(&t1);
	strcpy(buf,now);	
	/*printf("%s, %s",now,buf);*/
	write(sockfd,buf,strlen(buf));
/*
again:
while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
	printf("get string %s\n",buf);
	write(sockfd, buf, n);
}
*/
/*if (n < 0 && errno == EINTR)*/ /* interrupted by a signal before any data was read*/
/*	goto again; //ignore EINTR
else if (n < 0)
	printf("str_echo: read error");*/
}