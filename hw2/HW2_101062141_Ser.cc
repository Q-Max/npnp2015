#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* bzero()*/
#include <string.h> /* strcpy*/
#include <sys/socket.h> /* socket*/
#include <sys/select.h>
#include <time.h>
#include <arpa/inet.h> /* ipv4*/
#include <netinet/in.h> /* htonl ntohl htons ntohs*/
#include <errno.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <map>
#include <pthread.h>

#define LISTENQ 256
#define MAXLINE 4096
#define WELCOME "*************Welcome*****************\n[R]egister\t[L]ogin\n"

#define connect 0
#define regist 1
#define login 2
#define lobby 3
#define lobby2 4
#define logout 5
#define ack 6
using namespace std;
struct account{
	unsigned int id;
	std::string name;
	std::string pw;
	int use;
};
account accounts [100000];
std::vector<account> account_list;

int max(int num1, int num2) ;
int main(int argc, char **argv)
{
	int i, maxi, maxfd, listenfd, connfd, sockfd,udpfd;
	const int on=1;
	unsigned int id = 0;
	int state = 0;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	socklen_t len;
	int j,k;
	fd_set rset, allset;
	char mesg[MAXLINE];
	char temp[sizeof(sockaddr_in)+1];
	char dst[100];
	char *ptr;
	std::string sock2;
	std::string sock;
	if (argc != 2){
		printf("usage: serv <Port>\n");
		exit(0);
	}
	int SERV_PORT=atoi(argv[1]);
	char line[MAXLINE];
	account processing_account;
	//socklen_t clilen;
	// struct sockaddr_in cliaddr, servaddr;
	// listenfd = socket(AF_INET, SOCK_STREAM, 0);
	// bzero(&servaddr, sizeof(servaddr));
	// servaddr.sin_family= AF_INET;
	// servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// servaddr.sin_port= htons(SERV_PORT);
	//setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	// bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//listen(listenfd, LISTENQ);
	
	/* initialize */
	maxi = -1;
	/* index into client[] array */
	/* for create UDP socket*/
	struct sockaddr_in servaddr;
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	// if(setsockopt(udpfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	// 	perror("Error");
	// }
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//typedef std::map<struct sockaddr_in, int> TEST_MAP;
	//std::map<struct sockaddr_in, int> *client_state = new std::map<struct sockaddr_in, int>();
	std::map<std::string, int> client_state;
	std::map<std::string, int> client_userid;
	std::map<std::string, int> client_account_index;
	std::map<std::string, int>::iterator client_iter;
	string temp1;
	string temp2;
	maxfd = udpfd + 1;
	for (i = 0; i<100000; i++)accounts[i].use=0;
	for (i = 0; i < FD_SETSIZE; i++)client[i] = -1;
	/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	for ( ; ; ) {
		rset = allset;
		FD_SET(udpfd, &rset);
		 /*# of ready descriptors */
		
		if (FD_ISSET(udpfd, &rset)) {
			len = sizeof(servaddr);
			if((n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0 ){
				printf("timeout\n");
			}
			// mesg[0] = ACK:
			// sendto(udpfd, mesg, 1, 0, (struct sockaddr *) &servaddr, len);
			printf("connection from %s, port %hu\n",inet_ntop(AF_INET, &( servaddr.sin_addr), dst, INET_ADDRSTRLEN),ntohs(servaddr.sin_port));
			strncpy(temp,(char*)&servaddr,sizeof(sockaddr_in));
			temp[sizeof(sockaddr_in)] = '\0';
			sock = std::string((char*)&servaddr, sizeof(sockaddr_in));
			sock2 = temp;
			if(client_state.find(sock) == client_state.end()){
				client_state[sock] = connect;
				if(client_state[sock2] == connect){
					puts("QQQQ");
				}
				//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, connect)) ;
				strcpy(mesg, WELCOME);
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == connect&&!strcmp(mesg,"R")){
				client_state[sock] = regist;
				//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, regist)) ;
				strcpy(mesg , "Registration : Please type your username and password by <username>/<password> : \n");
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == regist){
				if(strstr(mesg,"/")==NULL)
					continue;
				
				
				//processing_account = new struct account();
				accounts[id].id = id;
				ptr = strtok(mesg,"/");

				accounts[id].name = ptr;
				for(int i = 0;i<id;i++){
					if(!accounts[i].name.compare(accounts[id].name)){
						strcpy(mesg , "Username has been used, please choose a new name and type your username and password again by <username>/<password> : \n");
						sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
						continue;
					}
				}
				ptr = strtok(NULL,"/");
				if(ptr == NULL){
					strcpy(mesg , "Registration : Please type your username and password by <username>/<password> : \n");
					sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
					continue;
				}
				accounts[id].pw = ptr;
				accounts[id].use = 1;
				client_state[sock] = lobby;
				client_userid[sock] = id;
				client_account_index[sock] = id;
				//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, regist)) ;
				id++;
				strcpy(mesg , "Registration complete \n");
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == connect&&!strcmp(mesg,"L")){
				client_state[sock] = login;
				//client_state->insert(std::pair<struct sockaddr_in, int>(servaddr, login)) ;
				strcpy(mesg, "Login : Please input your username and password by <username>/<password> : \n");
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == login){
				if(strstr(mesg,"/")==NULL)
					continue;
				ptr = strtok(mesg,"/");
				temp1 = ptr;
				ptr = strtok(NULL,"/");
				if(ptr == NULL){
					strcpy(mesg , "Login : Format error. Please type your username and password by <username>/<password> : \n");
					sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
					continue;
				}
				temp2 = ptr;
				std::cout<<temp1<<temp2<<'\n';
				for(k = 0,j=0;k<id;k++){
					std::cout<<accounts[k].name<<accounts[k].pw<<'\n';
					if(!accounts[k].name.compare(temp1)&&!accounts[k].pw.compare(temp2)&&accounts[k].use==1){
						strcpy(mesg , "Login succeed");
						client_state[sock] = lobby;
						client_userid[sock] = accounts[k].id;
						client_account_index[sock] = k;
						j=1;
						sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
						break;
					}
				}
				if(j==0){
					strcpy(mesg, "Login error : Please input your username and password by <username>/<password> : \n");
					sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				}				
				continue;
			}
			/*else if(client_state[sock] == lobby2&&!strcmp(mesg,"O")){
				//puts("QQ");
				client_state[sock] = lobby;
				ptr = mesg;
				for(client_iter = client_account_index.begin(),client_iter!=client_account_index.end();client_iter++){
					sprintf(ptr,"%s\n",accounts[(*client_iter)].name.c_str());
					ptr = ptr + strlen(ptr);
					*ptr = ' ';
					ptr++;
					*(ptr) = '\0';
				}				
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}*/
			else if(client_state[sock] == lobby2&&!strcmp(mesg,"D")){
				//puts("QQ");
				client_state[sock] = logout;
				client_state.erase(sock);
				client_userid.erase(sock);
				client_account_index.erase(sock);
				accounts[client_account_index[sock]].use = 0;
				sprintf(mesg,"Account delete\n");
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == lobby2&&!strcmp(mesg,"L")){
				//puts("QQ");
				client_state[sock] = logout;
				client_state.erase(sock);
				client_userid.erase(sock);
				client_account_index.erase(sock);
				sprintf(mesg,"Bye\n");
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == lobby){
				//puts("QQ");
				client_state[sock] = lobby2;
				sprintf(mesg,"*************Hello %s*****************\n[SU]Show User [SA]Show Article [A]dd Article [E]nter Article\n[O]nline users [Y]ell [T]ell [L]ogout [D]elete this account\n",(accounts[client_userid[sock]].name).c_str());
				sendto(udpfd, mesg, strlen(mesg), 0, (struct sockaddr *) &servaddr, len);
				continue;
			}
			else if(client_state[sock] == logout){
				
				continue;
			}
			sendto(udpfd, mesg, n, 0, (struct sockaddr *) &servaddr, len);
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
