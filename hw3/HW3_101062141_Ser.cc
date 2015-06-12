#include <stdio.h>
#include <stdlib.h>
// #include <strings.h> /* bzero()*/
#include <cstring>
#include <sys/socket.h> /* socket*/
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>
#include <sys/wait.h>
#include  <sys/ioctl.h>  
#include  <fcntl.h> 
#include <unistd.h>
#include  <signal.h> 
#include <pthread.h>
#include <string>
#include <map>
#include <iostream>

#define LISTENQ 256
#define MAXLINE 4096
#define WELCOME "*************Welcome*****************\n[R]egister\t[L]ogin\n"
#define splitline "************************************************************\n"

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

void str_echo(struct myDefStruct* arg);
static void *doit(void *arg);
struct account{
	unsigned int id;
	std::string name;
	std::string pw;
	int use;
};
struct file_at_client{
	unsigned int size;
	unsigned int belong_to_client_account_index;
	std::string name;
	struct sockaddr_in servaddr;
};
account accounts[1000];
file_at_client files[1000];
std::map<std::string, int> client_state;
std::map<std::string, int> client_userid;
std::map<std::string, int> client_account_index;
int id_index;
int file_index;
#define LS "/bin/ls"
char cwd[1024];
struct myDefStruct
{
	struct sockaddr_in servaddr;
	int sockfd;
};
int main(int argc, char **argv)
{
	if (argc != 2){
		printf("usage: tcpcli <port>");
		exit(0);
	}
	struct myDefStruct *myDefPtr;
	int SERV_PORT = atoi(argv[1]);
	int listenfd, connfd;
	pthread_t tid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket (AF_INET, SOCK_STREAM, 0);
	id_index = 0;
	file_index = 0;
	char dst[100];
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY); // for any interface
	servaddr.sin_port = htons (SERV_PORT); //user define
	bind(listenfd, (__SOCKADDR_ARG) &servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	
	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (__SOCKADDR_ARG) &cliaddr, &clilen);
		printf("%d\n", connfd);
		myDefPtr= (struct myDefStruct*)malloc(sizeof(struct myDefStruct));
		myDefPtr->servaddr = cliaddr;
		myDefPtr->sockfd = connfd;
		pthread_create(&tid, NULL, &doit, (void*)myDefPtr);
		// printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
		// if ( (childpid = fork()) == 0) {  child process 
		// 	close(listenfd);
		// 	/* cloLse listening socket */
		// 	str_echo(connfd); /* process the request */
		// 	exit (0); //end child process
		// }
		// close(connfd);
		/* parent closes connected socket */
	 }
	
	// for ( ; ; ) {
	// 	clilen = sizeof(cliaddr);
	// 	connfd = accept(listenfd, (__SOCKADDR_ARG) &cliaddr, &clilen);
	// 	printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( cliaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(cliaddr.sin_port));
	// 	if ( (childpid = fork()) == 0) { /* child process */
	// 		close(listenfd);
	// 		/* close listening socket */
	// 		str_echo(connfd); /* process the request */
	// 		exit (0); //end child process
	// 	}
	// 	close(connfd);
	// 	/* parent closes connected socket */
	//  }
 }
 void str_echo(struct myDefStruct* arg)
{
	//printf("%d\n", sockfd);
	int sockfd = arg->sockfd;
	struct sockaddr_in servaddr = arg->servaddr;
	ssize_t n;
	char buf2[MAXLINE];
	char *buf = buf2+1;
	//char buf[MAXLINE]; //MAXLINE is defined by user
	char temp[MAXLINE];
	char dst[100];
	int file_index_in_files;
	char *ptr;
	std::string  temp1, temp2;
	int j, k;
	int flag;
	std::string sock;
again:
	while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
		// printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( servaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(servaddr.sin_port));
		strncpy(temp,(char*)&servaddr,sizeof(sockaddr_in));
		temp[sizeof(sockaddr_in)] = '\0';
		sock = std::string((char*)&servaddr, sizeof(sockaddr_in));	
		if(client_state.find(sock) == client_state.end()){
			buf2[0] = NORMAL;
			client_state[sock] = CONNECT;
			
			//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, CONNECT)) ;
			strcpy(buf, WELCOME);
			write(sockfd, buf2, strlen(WELCOME)+2);
			continue;
		}
		else if(client_state[sock] == CONNECT&&!strcmp(buf,"R")){
			buf2[0] = NORMAL;
			client_state[sock] = regist;
			//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, regist)) ;
			strcpy(buf , "Registration : Please type your username and password by <username>/<password> : \n");
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == regist){
			buf2[0] = NORMAL;
			if(strstr(buf,"/")==NULL)
				continue;
			
			
			//processing_account = new struct account();
			accounts[id_index].id = id_index;
			ptr = strtok(buf,"/");

			accounts[id_index].name = ptr;
			flag = 0;
			for(int i = 0;i<id_index;i++){
				if(accounts[i].use&&!accounts[i].name.compare(accounts[id_index].name)){
					strcpy(buf , "Username has been used, please choose a new name and type your username and password again by <username>/<password> : \n");
					write(sockfd, buf2, strlen(buf)+2);
					flag = 1;
					break;
				}
			}
			if(flag)
				continue;
			ptr = strtok(NULL,"/");
			if(ptr == NULL){
				strcpy(buf , "Registration : Please type your username and password by <username>/<password> : \n");
				write(sockfd, buf2, strlen(buf)+2);
				continue;
			}
			accounts[id_index].pw = ptr;
			accounts[id_index].use = 1;
			client_state[sock] = lobby;
			client_userid[sock] = id_index;
			client_account_index[sock] = id_index;
			//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, regist)) ;
			id_index++;
			strcpy(buf , "Registration complete \n");
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == CONNECT&&!strcmp(buf,"L")){
			buf2[0] = NORMAL;
			client_state[sock] = login;
			//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, login)) ;
			strcpy(buf, "Login : Please input your username and password by <username>/<password> : \n");
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == login){
			buf2[0] = NORMAL;
			if(strstr(buf,"/")==NULL)
				continue;
			ptr = strtok(buf,"/");
			temp1 = ptr;
			ptr = strtok(NULL,"/");
			if(ptr == NULL){
				strcpy(buf , "Login : Format error. Please type your username and password by <username>/<password> : \n");
				write(sockfd, buf2, strlen(buf)+2);
				continue;
			}
			temp2 = ptr;
			// std::cout<<temp1<<temp2<<'\n';
			for(k = 0,j=0;k<id_index;k++){
				// std::cout<<accounts[k].name<<accounts[k].pw<<'\n';
				if(!accounts[k].name.compare(temp1)&&!accounts[k].pw.compare(temp2)&&accounts[k].use==1){
					strcpy(buf , "Login succeed");
					client_state[sock] = lobby;
					client_userid[sock] = accounts[k].id;
					client_account_index[sock] = k;
					j=1;
					write(sockfd, buf2, strlen(buf)+2);
					break;
				}
			}
			if(j==0){
				strcpy(buf, "Login error : Please input your username and password by <username>/<password> : \n");
				write(sockfd, buf2, strlen(buf)+2);
			}				
			continue;
		}
		else if(client_state[sock] == lobby){
			//puts("QQ");
			buf2[0] = NORMAL;
			client_state[sock] = lobby2;
			sprintf(buf,"*************Hello %s*****************\n[SU]Show User [SA]Show Article [A]dd Article [E]nter Article\n[O]nline users [Y]ell [T]ell [L]ogout [D]elete this account [F]iles\n",(accounts[client_userid[sock]].name).c_str());
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == lobby2&&!strcmp(buf,"D")){
			//puts("QQ");
			buf2[0] = NORMAL;
			client_state[sock] = logout;
			client_state.erase(sock);
			client_userid.erase(sock);
			client_account_index.erase(sock);
			accounts[client_account_index[sock]].use = 0;
			sprintf(buf,"Account delete\n");
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == lobby2&&!strcmp(buf,"L")){
			//puts("QQ");
			buf2[0] = EXIT;
			client_state[sock] = logout;
			client_state.erase(sock);
			client_userid.erase(sock);
			client_account_index.erase(sock);
			sprintf(buf,"Bye\n");
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == lobby2&&!strcmp(buf,"F")){
			//puts("QQ");
			buf2[0] = NORMAL;
			client_state[sock] = file_c;
			sprintf(buf,"[U]pdate file information [D]ownload others' file\n");
			write(sockfd, buf2, strlen(buf)+2);
			continue;
		}
		else if(client_state[sock] == file_c&&!strcmp(buf,"U")){
			buf2[0] = FILE_UPDATE_DATA;
			client_state[sock] = FILE_UPDATE_DATA_PROCESS;
			write(sockfd, buf2, 1);
			
			//todo
			continue;
		}
		else if(client_state[sock] == FILE_UPDATE_DATA_PROCESS){
			if(buf[0]==FILE_UPDATE_DATA_PROCESS){
				//puts(buf+1);
				sscanf(buf+1,"%s %u",dst,&files[file_index].size);
				files[file_index].name = std::string(dst);
				files[file_index].servaddr = servaddr;
				files[file_index].belong_to_client_account_index = client_account_index[sock];
				std::cout<<files[file_index].name;
				printf(" belong to %s\n",accounts[client_account_index[sock]].name.c_str());
				file_index++;
				buf2[0] = ack_QQ;
				write(sockfd, buf2, 1);
				continue;
			}
			else if(buf[0]==FILE_UPDATE_DATA_PROCESS_DONE){
				client_state[sock] = lobby;
				buf2[0] = NORMAL;
				write(sockfd, buf2, 1);				
				//todo				
				continue;
			}
		}
		else if(client_state[sock] == file_c&&!strcmp(buf,"D")){
			buf2[0] = FILE_GET_FILES;
			for(int iii=0;iii<file_index;iii++){
				// printf("%s belong to %s\n",files[iii].name.c_str(),accounts[files[iii].belong_to_client_account_index].name.c_str());
				
				sprintf(buf,"%s belong to %s\n",files[iii].name.c_str(),accounts[files[iii].belong_to_client_account_index].name.c_str());
				write(sockfd, buf2, strlen(buf)+2);
				read(sockfd, buf, 1);
				if(buf[0]!=ack_QQ)
					break;
				
			}
			client_state[sock] = FILE_DOWNLOAD_1;
			buf2[0] = NORMAL;
			strcpy(buf,splitline);
			write(sockfd, buf2, strlen(buf)+2);
			read(sockfd, buf2, 1);
			buf2[0] = NORMAL;
			do{
				strcpy(buf,"Please enter the filename you want to download?\n");
				write(sockfd, buf2, strlen(buf)+2);
				read(sockfd, buf2, MAXLINE);
				flag = 0;
				printf("%s %lu\n",buf2,strlen(buf2));
				for(int iii=0;iii<file_index;iii++){
					if(!strcmp(files[iii].name.c_str(),buf2)){
						file_index_in_files = iii;
						flag = 1;	
						break;
					}
				}
			}
			while(!flag);
			//call the client to prepare
			buf2[0] = FILE_GO_DOWNLOAD;
			strncpy(buf, (char*)&(files[file_index_in_files].servaddr), sizeof(sockaddr_in));
			strcpy(buf+ sizeof(sockaddr_in), files[file_index_in_files].name.c_str());
			write(sockfd, buf2, MAXLINE);
			client_state[sock] = lobby;
			//todo
			continue;
		}
		else if(client_state[sock] == logout){
			buf2[0]=EXIT;
			write(sockfd, buf2, 1);
			return;
		}
		printf("get string %s\n",buf);
	}
	if (n < 0 && errno == EINTR) /* interrupted by a signal before any data was read*/
		goto again; //ignore EINTR
	else if (n < 0)
		printf("str_echo: read error");
}
static void *doit(void *arg)
{
	pthread_detach(pthread_self());
	str_echo((struct myDefStruct*)arg);
	close(((struct myDefStruct*)arg)->sockfd);
	return (NULL);
} 