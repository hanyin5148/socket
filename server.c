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
#define PORT 5000
using namespace std;

void SERVER_RECV();

struct sockaddr_in serv_addr;
char sendBuff[1025];
char buf[1025] = {0};
int listenfd = 0, connfd = 0;
int ret = 0;

int main(int argc, char *argv[])
{
	//execute thread
	thread mThread(SERVER_RECV);

	//doing something
	while(1)
	{
		//keyboard input
		char* retBuf = fgets(buf, sizeof(buf), stdin);
		printf("Send the message from the server: ");
		if ( retBuf[0] == '\n' )
		{
			close(connfd);
			exit(-1);
		}
		printf("\n");
		//send the string to client
		ret = write(connfd, buf, strlen(buf));
		if ( ret == -1)
		{
			close(connfd);
			exit(-1);
		}
	}

	//wait the thread stop
	mThread.join();

	return 0;
}
void SERVER_RECV() {
	
	/* creates an UN-named socket inside the kernel and returns
	 * an integer known as socket descriptor
	 * This function takes domain/family as its first argument.
	 * For Internet family of IPv4 addresses we use AF_INET
	 */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);

	/* The call to the function "bind()" assigns the details specified
	 * in the structure 『serv_addr' to the socket created in the step above
	 */
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	/* The call to the function "listen()" with second argument as 10 specifies
	 * maximum number of client connections that server will queue for this listening
	 * socket.
	 */
	listen(listenfd, 5);
	/* In the call to accept(), the server is put to sleep and when for an incoming
	 * client request, the three way TCP handshake* is complete, the function accept()
	 * wakes up and returns the socket descriptor representing the client socket.
	 */
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

	bool run = true;
	
	int ret = 0;

	while (run) {

		// Receive
		ret = read(connfd, buf, sizeof(buf) - 1);
		if (ret == -1) 
		{
			exit(-2);
		}
		else if (ret == 0) 
		{ 
			close(connfd);
			break;
		}

		// 把收到的資料當作字串 後面補0 並印出
		buf[ret] = '\0';
		printf("Recieved: %s\n", buf);
		
	}
	close(connfd);
}
