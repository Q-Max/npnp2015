#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* bzero()*/
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <sys/wait.h>
#include <sys/stat.h> /* st cwd*/
#include <sys/types.h>
#include <math.h> /* ceil*/

#define MAXLINE 2048

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
		strcat(cwd,"/Download/");
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
	char tmp[1024];

	FILE *fp2;
	int i,j,k,l;
	char sendline[MAXLINE], recvline[MAXLINE];
again:

	write(sockfd, "ready", strlen ("ready")+1);	
	if (read(sockfd, recvline, MAXLINE) == 0) {
		printf("str_cli: server terminated prematurely\n");
		exit(0);
	}
	fputs(recvline, stdout);
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		//printf("send string %s.%d",sendline,strlen(sendline));
		write(sockfd, sendline, strlen (sendline)+1);
		if(!strcmp(sendline,"L\n")){
			while(1){
				if (read(sockfd, recvline, MAXLINE) == 0) {
					printf("str_cli: server terminated prematurely\n");
					exit(0);
				}				
				if(!strcmp(recvline,"end")){
					goto again;
				}
				fputs(recvline, stdout);
				write(sockfd, " ", 1);			
			}
		}
		else if(!strcmp(sendline,"E\n"))break;
		else if(!strcmp(sendline,"C\n")){
			//to do
			if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
				printf("str_cli: server terminated prematurely\n");
				exit(0);
			}
			puts("Enter directory name :");
			fgets(sendline, MAXLINE, fp) ;
			sendline[strlen(sendline)-1]='\0';
			write(sockfd, sendline, strlen (sendline)+1);
			if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
					printf("str_cli: server terminated prematurely\n");
					exit(0);
			}
			puts(recvline);
			write(sockfd, " ", 2);
			if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
					printf("str_cli: server terminated prematurely\n");
					exit(0);
			}
			if(!strcmp(recvline,"end")){
				goto again;
			}				
		}
		else if(!strcmp(sendline,"U\n")){
			// to do
			if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
				printf("str_cli: server terminated prematurely\n");
				exit(0);
			}
			puts("Enter file directory:");
			fgets(sendline, MAXLINE, fp) ;
			sendline[strlen(sendline)-1]='\0';
			if((fp2 = fopen(sendline,"r"))==NULL){
				write(sockfd, "error", strlen("error")+1);
				goto again;
			}
			else{
				write(sockfd, sendline, strlen(sendline)+1);
				if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
					printf("str_cli: server terminated prematurely\n");
					exit(0);
				}
				if(!strcmp(recvline,"fperr")){
					goto again;
				}
				fseek(fp2, 0L, SEEK_END);
				k=ftell(fp);
				fseek(fp2, 0L, SEEK_SET);
				j=(int)(ceil((double)k/(double)MAXLINE));
				i=0;
				//printf("%d,%d,%s\n",k,j,process_cwd);
				while (!feof(fp2)) {
					
					i++;
					printf("%d\n",i);
					if(i==j){
						write(sockfd,"last", 5);
						if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
							printf("str_cli: server terminated prematurely\n");
							exit(0);
						}
						sprintf(tmp,"%d",k%MAXLINE);
						write(sockfd, tmp, strlen(tmp)+1);
						if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
							printf("str_cli: server terminated prematurely\n");
							exit(0);
						}
						fread(sendline,MAXLINE,1,fp2);
						for(l=0;l<k%MAXLINE;l++)putchar(sendline[l]);
						write(sockfd, sendline, k%MAXLINE);
						if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
							printf("str_cli: server terminated prematurely\n");
							exit(0);
						}
						fclose(fp2);
						puts("qq");
						break;
					}
					else {
						fread(sendline,MAXLINE,1,fp2);	
						write(sockfd, sendline, MAXLINE);
						if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
							printf("str_cli: server terminated prematurely\n");
							exit(0);
						}
					}
				}
			}
			if(!strcmp(recvline,"end")){
				goto again;
			}				
		}
		else if(!strcmp(sendline,"D\n")){
			// to do
			if (read(sockfd, recvline, MAXLINE) == 0) {/* read ready signal*/
				printf("str_cli: server terminated prematurely\n");
				exit(0);
			}
			puts("Enter file name :");
			fgets(sendline, MAXLINE, fp) ;
			sendline[strlen(sendline)-1]='\0';
			write(sockfd, sendline, strlen (sendline)+1);
			puts("Save as :");
			fgets(sendline, MAXLINE, fp) ;
			sendline[strlen(sendline)-1]='\0';
			strcpy(tmp, cwd);
			strcat(tmp,sendline);
			if((fp2 = fopen(tmp,"w"))!=NULL){
				k=0;
				while(1){
					k++;
					printf("%d\n",k);
					if (read(sockfd, recvline, MAXLINE) == 0) {
						printf("str_cli: server terminated prematurely\n");
						exit(0);
					}
					if(!strcmp(recvline,"fperr")){
						puts("file error");
						goto again;
					}
					else if(!strcmp(recvline,"last")){
						puts("QQ");
						write(sockfd, " ", 2);
						if (read(sockfd, recvline, MAXLINE) == 0) {
							printf("str_cli: server terminated prematurely\n");
							exit(0);
						}
						i = atoi(recvline);
						puts(recvline);
						write(sockfd, " ", 2);

						if (read(sockfd, recvline, MAXLINE) == 0) {
							printf("str_cli: server terminated prematurely\n");
							exit(0);
						}
						puts(recvline);
						fwrite(recvline, i, 1, fp2);
						fclose(fp2);
						puts(recvline);
						write(sockfd, " ", 2);
						break;
					}
					fwrite(recvline, MAXLINE, 1, fp2);
					write(sockfd, " ", 2);
					// to do
				}
			}
			if (read(sockfd, recvline, MAXLINE) == 0) {
				printf("str_cli: server terminated prematurely\n");
				exit(0);
			}
			if(!strcmp(recvline,"end")){
					goto again;
			}
		}
		else goto again;
	}
	printf("send string EOF\n");
	sendline[0]=EOF;
	sendline[1]='\0';
	write(sockfd, sendline, strlen (sendline));
}