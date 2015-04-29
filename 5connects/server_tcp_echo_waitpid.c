#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>
#include <sys/wait.h>
#include  <sys/ioctl.h>  
#include  <signal.h>  


#define LISTENQ 256
#define MAXLINE 4096
#define SERV_PORT 12347

 void str_echo(int sockfd);
 void sig_child(int signo);

int main(int argc, char **argv)
{
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket (AF_INET, SOCK_STREAM, 0);
	char dst[100];
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY); // for any interface
	servaddr.sin_port = htons (SERV_PORT); //user define
	bind(listenfd, (__SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	int optval=1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval));
	listen(listenfd, LISTENQ);

	signal(SIGCHLD,sig_child);
	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (__SOCKADDR_ARG) &cliaddr, &clilen);
		printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
		if ( (childpid = fork()) == 0) { /* child process */
			close(listenfd);
			/* close listening socket */
			str_echo(connfd); /* process the request */
			exit (0); //end child process
		}
		close(connfd);
		/* parent closes connected socket */
	 }
 }
 void str_echo(int sockfd)
{
ssize_t n;
char buf[MAXLINE]; //MAXLINE is defined by user
again:
while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
	printf("get string %s\n",buf);
	write(sockfd, buf, n);
}
if (n < 0 && errno == EINTR) /* interrupted by a signal before any data was read*/
	goto again; //ignore EINTR
else if (n < 0)
	printf("str_echo: read error");
}
void sig_child(int signo){
	pid_t pid;
	int stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)printf("child %d terminated\n",pid);
	fflush(stdout);
	return;
}