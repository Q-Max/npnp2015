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
#include <sys/stat.h>/* mkdir */
#include <sys/types.h>/* mkdir */

#define LISTENQ 256
#define MAXLINE 2048

void str_echo(int sockfd, struct sockaddr_in cliaddr);
void sig_child(int signo);
char welMsg[] = "[C]hange directory [L]ist [U]pload [D]ownload [E]xit\n";
char bin[] = "/bin/ls";
char cwd[1024];
int main(int argc, char **argv)
{
	if (argc != 2){
		printf("usage: server <Port>\n");
		exit(0);
	}
	int SERV_PORT=atoi(argv[1]);

	if (getcwd(cwd, sizeof(cwd)) != NULL){
		strcat(cwd,"/Upload");
		fprintf(stdout, "Current working dir: %s\n", cwd);
	}
	else
		perror("getcwd() error");
	struct stat st = {0};
	if(stat(cwd, &st)==-1){
		mkdir(cwd, 0700);
	}
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
			str_echo(connfd, cliaddr); /* process the request */
			exit (0); //end child process
		}
		close(connfd);
		/* parent closes connected socket */
	 }
 }
 void str_echo(int sockfd, struct sockaddr_in cliaddr)
{
	FILE *fp;
	ssize_t n;
	char buf[MAXLINE]; //MAXLINE is defined by user
	char dst[100];
	char path[1035];

again:
	while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
		printf("%sstrlen : %d\n",buf,strlen(buf));
		if(!strcmp(buf,"ready")){
			printf("%s\n",buf);
			printf("client from %s, port %hu ready\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
			write(sockfd, welMsg, strlen(welMsg));
			continue;
		}
		else if(!strcmp(buf,"L\n")){
			strcpy(dst, bin);
			dst[strlen(dst)+1]='\0';
			dst[strlen(dst)] = ' ';
			strcat(dst, cwd);
			strcat(dst, " -F");
			puts(dst);
			fp = popen(dst, "r");
			if (fp == NULL) {
				printf("Failed to run command\n" );
				exit(1);
			}

			/* Read the output a line at a time - output it. */
			while (fgets(path, sizeof(path)-1, fp) != NULL) {
				write(sockfd, path, strlen(path));
			}

			/* close */
			pclose(fp);
			write(sockfd, welMsg, strlen(welMsg));
			continue;
		}
		printf("get string %s\n",buf);
		write(sockfd, buf, n);
	}
	if (n < 0 && errno == EINTR) /* interrupted by a signal before any data was read*/
		goto again; //ignore EINTR
	else if (n < 0)	{
		printf("str_echo: read error\n");
		printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
	}
}
void sig_child(int signo){
	pid_t pid;
	int stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)printf("child %d terminated\n",pid);
	fflush(stdout);
	return;
}