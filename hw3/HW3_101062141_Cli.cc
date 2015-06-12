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
#include <pthread.h>

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
#define FILE_GO_DOWNLOAD 15

#define LISTENQ 256
#define MAXLINE 4096
#define port2 2345

char bin[] = "/bin/ls";
char cwd[1024];
struct sockfd_sockaddr{
	int sockfd;
	struct sockaddr_in servaddr;
};

void *upload_server(void* arg);
void *upload_file(void* arg);
// void* upload_file2(int sockfd, struct sockaddr_in cliaddr, void* arg);
void str_cli(FILE *fp, int sockfd);
void file_check(int sockfd);
void download_file(struct sockaddr_in servaddr, char* filename);
int main(int argc, char **argv)
{
	pthread_t tid;
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
	pthread_create(&tid, NULL, &upload_server, (void*)&i);
	str_cli(stdin, sockfd); /* do it all */		
	exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
	char sendline[MAXLINE], recvline[MAXLINE];
	struct sockaddr_in servaddr;
	char filename[100];
	sendline[0] = CONNECT;
	write(sockfd, sendline, 1);
	while (read(sockfd, recvline, MAXLINE)) {
		if(recvline[0]==EXIT){
			break;
		}
		else if(recvline[0]==FILE_UPDATE_DATA)
		{
			file_check(sockfd);
			continue;
		}
		else if(recvline[0]==FILE_GET_FILES)
		{
			fputs(recvline+1, stdout);
			sendline[0] = ack_QQ;
			write(sockfd, sendline, 1);
			continue;
		}
		else if(recvline[0]==FILE_GO_DOWNLOAD)
		{
			// puts("QQ");
			servaddr = *((struct sockaddr_in*)(recvline+1));
			strcpy(filename, recvline+1+sizeof(struct sockaddr_in));
			// puts(filename);
			download_file(servaddr, filename);
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
	sendline[0]=EOF;
	sendline[1]='\0';
	write(sockfd, sendline, strlen (sendline));
	close(sockfd);
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
void *upload_server(void* arg)
{
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	int listenfd = socket (AF_INET, SOCK_STREAM, 0);
	int connfd;
	char dst[100];
	pid_t childpid;
	pthread_t tid;
	struct sockfd_sockaddr *ptr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY); // for any interface
	servaddr.sin_port = htons (port2); //user define
	bind(listenfd, (__SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	int optval=1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval));
	listen(listenfd, LISTENQ);
	for ( ; ; ) {
		
		
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (__SOCKADDR_ARG) &cliaddr, &clilen);
		printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
		ptr = (struct sockfd_sockaddr *)malloc(sizeof(struct sockfd_sockaddr ));
		ptr->sockfd = connfd;
		ptr->servaddr = cliaddr;
		pthread_create(&tid, NULL, &upload_file, (void*)ptr);
		
		// if ( (childpid = fork()) == 0) { /* child process */
		// 	close(listenfd);			
		// 	upload_file2(connfd, cliaddr, ptr); /* process the request */
		// 	exit (0); //end child process
		// }
		//close(connfd);
		/* parent closes connected socket */
	 }
}
/*
void* upload_file2(int sockfd, struct sockaddr_in cliaddr, void* arg){

	int sockfd2 = ((struct sockfd_sockaddr *)arg)->sockfd;
	struct sockaddr_in cliaddr2 = ((struct sockfd_sockaddr *)arg)->servaddr;
	if(sockfd2==sockfd)puts("Equal");
	ssize_t n = sizeof( struct sockaddr_in );
	if (memcmp((char*)&cliaddr2, (char*)&cliaddr, n) == 0) {
   		puts("Equal2");
	}

	FILE *fp;
	
	char buf[MAXLINE]; //MAXLINE is defined by user
	char dst[100];
	char process_cwd[1024];
	int i,j,k,l;
again:
	n = read(sockfd, buf, MAXLINE);
	if(strcmp(buf,"privatemessage")){
		strcpy(process_cwd, cwd);
		process_cwd[strlen(process_cwd)+1]='\0';
		process_cwd[strlen(process_cwd)] = '/';
		strcat(process_cwd,buf);
		if(!(fp=fopen(process_cwd,"r"))){
			write(sockfd, "fperr", strlen("fperr")+1);
			return NULL;
		}
		fseek(fp, 0L, SEEK_END);
		k=ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		j=(int)(ceil((double)k/(double)MAXLINE));
		i=0;
		while (!feof(fp)) {
			i++;
			if(i==j){
				puts("QQQ");
				write(sockfd,"last", 5);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}
				sprintf(dst,"%d",k%MAXLINE);
				write(sockfd, dst, strlen(dst)+1);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}
				fread(buf,MAXLINE,1,fp);
				write(sockfd, buf, k%MAXLINE);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}
				fclose(fp);
				break;
			}
			else {
				puts("QQQQQQQQQQQQ");
				fread(buf,MAXLINE,1,fp);	
				write(sockfd, buf, MAXLINE);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}

			}
		}
	}
	else{

	}
	write(sockfd, "end", strlen("end")+1);	
}
*/
void* upload_file(void* arg){

	int sockfd = ((struct sockfd_sockaddr *)arg)->sockfd;
	struct sockaddr_in cliaddr = ((struct sockfd_sockaddr *)arg)->servaddr;
	FILE *fp;
	ssize_t n;
	char buf[MAXLINE]; //MAXLINE is defined by user
	char dst[100];
	char process_cwd[1024];
	int i,j,k,l;
again:
	n = read(sockfd, buf, MAXLINE);
	if(strcmp(buf,"privatemessage")){
		strcpy(process_cwd, cwd);
		process_cwd[strlen(process_cwd)+1]='\0';
		process_cwd[strlen(process_cwd)] = '/';
		strcat(process_cwd,buf);
		if(!(fp=fopen(process_cwd,"r"))){
			write(sockfd, "fperr", strlen("fperr")+1);
			return NULL;
		}
		fseek(fp, 0L, SEEK_END);
		k=ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		j=(int)(ceil((double)k/(double)MAXLINE));
		i=0;
		while (!feof(fp)) {
			i++;
			if(i==j){
				write(sockfd,"last", 5);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}
				sprintf(dst,"%d",k%MAXLINE);
				write(sockfd, dst, strlen(dst)+1);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}
				fread(buf,MAXLINE,1,fp);
				write(sockfd, buf, k%MAXLINE);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}
				fclose(fp);
				break;
			}
			else {
				fread(buf,MAXLINE,1,fp);	
				write(sockfd, buf, MAXLINE);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return NULL;
				}

			}
		}
	}
	else{

	}
	write(sockfd, "end", strlen("end")+1);	
}
void download_file(struct sockaddr_in servaddr, char* filename){
	char file[100], recvline[MAXLINE];
	int i,k;
	strcpy(file, filename);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	servaddr.sin_port = htons (port2); 
	connect(sockfd, (__CONST_SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	write(sockfd, file, strlen(file));
	char cwd_process[1024];
	strcpy(cwd_process,cwd);
	cwd_process[strlen(cwd_process)+1] = '\0';
	cwd_process[strlen(cwd_process)] = '/';
	strcat(cwd_process, filename);
	FILE *fp2;
	if((fp2 = fopen(cwd_process,"w"))!=NULL){
		k=0;
		while(1){
			k++;
			if (read(sockfd, recvline, MAXLINE) == 0) {
				printf("str_cli: server terminated prematurely\n");
				exit(0);
			}
			if(!strcmp(recvline,"fperr")){
				puts("file error");
				return;
			}
			else if(!strcmp(recvline,"last")){
				write(sockfd, " ", 2);
				if (read(sockfd, recvline, MAXLINE) == 0) {
					printf("str_cli: server terminated prematurely\n");
					exit(0);
				}
				i = atoi(recvline);
				write(sockfd, " ", 2);
				if (read(sockfd, recvline, MAXLINE) == 0) {
					printf("str_cli: server terminated prematurely\n");
					exit(0);
				}
				fwrite(recvline, i, 1, fp2);
				fclose(fp2);
				write(sockfd, " ", 2);
				puts("Download complete");
				break;
			}
			fwrite(recvline, MAXLINE, 1, fp2);
			write(sockfd, " ", 2);
			// to do
		}

	}
}