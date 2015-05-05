#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>
#include <sys/wait.h>
#include  <sys/ioctl.h>  
#include  <signal.h>  
#include <sys/stat.h>/* mkdir */
#include <sys/types.h>/* mkdir */
#include <math.h> /* ceil*/

#define LISTENQ 256
#define MAXLINE 2048

void str_echo(int sockfd, struct sockaddr_in cliaddr);
void sig_child(int signo);
char welMsg[] = "[C]hange directory [L]ist [U]pload [D]ownload [E]xit\n";
char listMsg[] = "End with '*#' means it's a executable file, with '/' means it's a directory\n";
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
	char child_cwd[1024];
	char down_process[1024];
	int i,j,k,l;
	strcpy(child_cwd,cwd);

again:
	while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
		printf("%sstrlen : %lu\n",buf,strlen(buf));
		if(!strcmp(buf,"ready")){
			printf("%s\n",buf);
			printf("client from %s, port %hu ready\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
			write(sockfd, welMsg, strlen(welMsg)+1);
			continue;
		}
		else if(!strcmp(buf,"L\n")){
			strcpy(dst, bin);
			dst[strlen(dst)+1]='\0';
			dst[strlen(dst)] = ' ';
			strcat(dst,child_cwd);
			strcat(dst, " -F");
			puts(dst);
			fp = popen(dst, "r");
			if (fp == NULL) {
				printf("Failed to run command\n" );
				exit(1);
			}
			write(sockfd, listMsg, strlen(listMsg)+1);
			/* Read the output a line at a time - output it. */
			while ((fgets(path, sizeof(path)-1, fp)) != NULL) {
				printf("%s,%lu",path,strlen(path));
				write(sockfd, path, strlen(path)+1);
				if( (n = read(sockfd, buf, MAXLINE))<0){
					printf("str_echo: read error\n");
					printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
					return;
				}
			}
			/* close */
			pclose(fp);
			write(sockfd, "end", strlen("end")+1);
			continue;
		}
		else if(!strcmp(buf,"C\n")){
			//to do
			// //DIR* dir = opendir("mydir");
			// //if (dir)
			// //{
			// 	/* Directory exists. */
			// 	closedir(dir);
			// }
			// else if (ENOENT == errno)
			// {
			// 	/* Directory does not exist. */
			// }
			// else
			// {
			// 	/* opendir() failed for some other reason. */
			// }

			write(sockfd, "end", strlen("end")+1);
			continue;
		}
		else if(!strcmp(buf,"D\n")){
			//to do
			write(sockfd, "ready", strlen("ready")+1);
			if( (n = read(sockfd, buf, MAXLINE))<0){
				printf("str_echo: read error\n");
				printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
				return;
			}
			strcpy(down_process,child_cwd);
			down_process[strlen(down_process)+1]='\0';
			down_process[strlen(down_process)] = '/';
			strcat(down_process,buf);
			if(!(fp=fopen(down_process,"r"))){
				write(sockfd, "fperr", strlen("fperr")+1);
				goto again;
			}
			fseek(fp, 0L, SEEK_END);
			k=ftell(fp);
			fseek(fp, 0L, SEEK_SET);
			j=(int)(ceil((double)k/(double)MAXLINE));
			i=0;
			printf("%d,%d,%s\n",k,j,down_process);
			while (!feof(fp)) {
				
				i++;
				printf("%d\n",i);
				if(i==j){
					write(sockfd,"last", 5);
					if( (n = read(sockfd, buf, MAXLINE))<0){
						printf("str_echo: read error\n");
						printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
						return;
					}
					sprintf(dst,"%d",k%MAXLINE);
					write(sockfd, dst, strlen(dst)+1);
					if( (n = read(sockfd, buf, MAXLINE))<0){
						printf("str_echo: read error\n");
						printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
						return;
					}
					fread(buf,2048,1,fp);
					for(l=0;l<k%MAXLINE;l++)putchar(buf[l]);
					write(sockfd, buf, k%MAXLINE);
					if( (n = read(sockfd, buf, MAXLINE))<0){
						printf("str_echo: read error\n");
						printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
						return;
					}
					fclose(fp);
					puts("qq");
					break;
				}
				else {
					fread(buf,2048,1,fp);	
					write(sockfd, buf, MAXLINE);
					if( (n = read(sockfd, buf, MAXLINE))<0){
						printf("str_echo: read error\n");
						printf("connection from %s, port %hu closed\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
						return;
					}
				}
			}
			write(sockfd, "end", strlen("end")+1);
			continue;
		}
		else if(!strcmp(buf,"U\n")){
			//to do

			write(sockfd, "end", strlen("end")+1);
			continue;
		}
		else
			write(sockfd, welMsg, strlen(welMsg)+1);
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