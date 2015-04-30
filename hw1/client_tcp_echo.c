#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <sys/wait.h>
#include <sys/stat.h> /* st cwd*/
#include <sys/types.h>

#define MAXLINE 4096

char cwd[1024];

void str_cli(FILE *fp, int sockfd);
int main(int argc, char **argv)
{
	int i;
	int sockfd[5];
	int childpid;
	struct sockaddr_in servaddr;
	
	if (argc != 3){

		printf("usage: tcpcli <IPaddress> <Port>");
		exit(0);
	}
	int SERV_PORT=atoi(argv[2]);
	
	if (getcwd(cwd, sizeof(cwd)) != NULL){
		strcat(cwd,"/Download");
		fprintf(stdout, "Current working dir: %s\n", cwd);
	}
	else
		perror("getcwd() error");
	struct stat st = {0};
	if(stat(cwd, &st)==-1){
		mkdir(cwd, 0700);
	}
	for(i=0;i<1;i++)
	{		
		sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT);
		inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
		connect(sockfd[i], (__CONST_SOCKADDR_ARG) &servaddr, sizeof(servaddr));		
		
	}

	str_cli(stdin, sockfd[i-1]); /* do it all */		
	puts("QQ");
	exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
	write(sockfd, "ready", strlen ("ready"));
	char sendline[MAXLINE], recvline[MAXLINE];
	if (read(sockfd, recvline, MAXLINE) == 0) {
		printf("str_cli: server terminated prematurely\n");
		exit(0);
	}	
	fputs(recvline, stdout);
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		//printf("send string %s.%d",sendline,strlen(sendline));
		write(sockfd, sendline, strlen (sendline)+1);
		if(!strcmp(sendline,"L\n")){
			while(true){
				if (read(sockfd, recvline, MAXLINE) == 0) {
					printf("str_cli: server terminated prematurely\n");
					exit(0);
				}
				else if(!strcmp(recvline,"end")
					break;
				fputs(recvline, stdout);
			}
		}
		
	}
	printf("send string EOF\n");
	sendline[0]=EOF;
	sendline[1]='\0';
	write(sockfd, sendline, strlen (sendline));
}