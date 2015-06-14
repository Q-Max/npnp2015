#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>/* mkdir */
#include <sys/types.h>/* mkdir */
#include <math.h> /* ceil*/
#include <dirent.h> /* DIR*/

#define CONNECT 0
#define regist 1
#define login 2
#define lobby 3
#define lobby2 4
#define logout 5
#define ack_QQ 6
#define file_c 7
#define FILE_UPDATE_DATA 8
#define NORMAL 9
#define EXIT 10
#define FILE_UPDATE_DATA_PROCESS 11
#define FILE_GET_FILES 12
#define FILE_UPDATE_DATA_PROCESS_DONE 13
#define FILE_DOWNLOAD_1 14

#define MAXLINE 4096

char bin[] = "/bin/ls";
char cwd[1024];

void str_cli(FILE *fp, int sockfd);
void file_check(int sockfd);
int main(int argc, char **argv)
{
	int i;
	int sockfd;
	struct sockaddr_in servaddr;
	if (argc != 3){
		printf("usage: tcpcli <IPaddress> <port>");
		exit(0);
	}	
	int SERV_PORT = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	connect(sockfd, (__CONST_SOCKADDR_ARG) &servaddr, sizeof(servaddr));	
	if (getcwd(cwd, sizeof(cwd)) != NULL){ /*set workspace*/
		strcat(cwd,"/Workspace");
		fprintf(stdout, "Current working dir: %s\n", cwd);
	}
	else
		perror("getcwd() error");
	struct stat st = {0};
	if(stat(cwd, &st)==-1){ /*if not exist, mkdir*/
		mkdir(cwd, S_IRWXU|S_IRWXG|S_IRWXO);
	}
	str_cli(stdin, sockfd); /* do it all */		
	exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
	char sendline[MAXLINE], recvline[MAXLINE];
	sendline[0] = CONNECT;
	write(sockfd, sendline, 1);
	while (read(sockfd, recvline, MAXLINE)) {
		if(recvline[0]==EXIT)
			return;
		else if(recvline[0]==FILE_UPDATE_DATA)
		{
			puts("QQ");
			file_check(sockfd);
			continue;
		}
		else if(recvline[0]==FILE_GET_FILES)
		{
			puts("QQ");
			
			fputs(recvline+1, stdout);
			sendline[0] = ack_QQ;
			write(sockfd, sendline, 1);
			continue;
		}
		fputs(recvline+1, stdout);
		if(fgets(sendline, MAXLINE, fp) != NULL){
			sendline[strlen(sendline)-1] = '\0';
			write(sockfd, sendline, strlen (sendline)+1);
		}
		// if (read(sockfd, recvline, MAXLINE) == 0) {
		// 	printf("str_cli: server terminated prematurely");
		// 	exit(0);
		// }
		
	}
	printf("send string EOF\n");
	sendline[0]=EOF;
	sendline[1]='\0';
	write(sockfd, sendline, strlen (sendline));
}

void file_check(int sockfd){
	char dst[200],temp[200];
	strcpy(dst, bin);
	dst[strlen(dst)+1]='\0';
	dst[strlen(dst)] = ' ';
	strcat(dst, cwd);
	strcat(dst, " -Al");
	char size[10],name[30];
	FILE *fp = popen(dst, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		return;
	}
	for(int i = 0; fgets(dst, sizeof(dst)-1, fp) != NULL; i++){
		if(!i)
			continue;
		if(dst[0]=='d')
			continue;
		else{
			sscanf(dst,"%*s %*s %*s %*s %s %*s %*s %*s %s",size,name);
			sprintf(temp+1,"%s %s\n",name,size);
			printf("%s %s\n",name,size);
			temp[0] = FILE_UPDATE_DATA_PROCESS;
			write(sockfd,temp,strlen(temp)+2);
			read(sockfd, temp, 1);
			if(temp[0]!=ack_QQ)
				return;
		}
		
	}
	temp[0] = FILE_UPDATE_DATA_PROCESS_DONE;
	write(sockfd, temp, 1);
	read(sockfd, temp, 1);
	write(sockfd, temp, 1);
	return;

}