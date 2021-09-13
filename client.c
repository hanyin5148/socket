#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <thread>
#define ADDRESS "127.0.0.1"
#define PORT 5000
using namespace std;

void CLIENT_RECV();

struct sockaddr_in serv_addr;
char recvBuff[1024];
char buf[1024] = {0};
int sockfd = 0;
int ret = 0;

int main(int argc, char *argv[])
{
	//execute thread
	thread mThread(CLIENT_RECV);
	
	//doing something
	while(1){
		//keyboard input
		char* retBuf = fgets(buf, sizeof(buf), stdin);
		//printf("Send some message :");
		if (retBuf[0] == '\n')
		{
			printf("Close and exit \n");
			exit(-1);
		}
		printf("\n");
		//send the string to server
		ret = write(sockfd, buf, strlen(buf));
		if ( ret == -1)
		{
			exit(-1);
		}
	}
	
	//wait the thread stop
	mThread.join();

	return 0;
}

void CLIENT_RECV(){
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		exit(-1);
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		exit(-1);
	}

	/* Information like IP address of the remote host and its port is
	 * bundled up in a structure and a call to function connect() is made
	 * which tries to connect this socket with the socket (IP address and port)
	 * of the remote host
	 */
	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		exit(-1);
	}
	
	/* Once the sockets are connected, the server sends the data (date+time)
	 * on clients socket through clients socket descriptor and client can read it
	 * through normal read call on the its socket descriptor.
	 */
	bool run = true;
	int ret = 0;
	while (run) {
		char buf[255] = { 0 };
		// Receive
		ret = read(sockfd, buf, sizeof(buf) - 1);
		if (ret == -1) 
		{
			exit(-1);
		}
		else if (ret == 0) 
		{ 
			break;
		}
		// 把資料當作字串 自己在最後補0 印出來
		buf[ret] = '\0';
		printf("Recieved: %s\n", buf);
	}
	close(sockfd);
}
