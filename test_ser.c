#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define PORT 5000
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void SERVER_RECV(int);

int main(void)
{
    signal(SIGCHLD, SIG_IGN);
    int listenfd; //only for accept
	/* creates an UN-named socket inside the kernel and returns
	 * an integer known as socket descriptor
	 * This function takes domain/family as its first argument.
	 * For Internet family of IPv4 addresses we use AF_INET
	 */
    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        //  listenfd = socket(AF_INET, SOCK_STREAM, 0)
        ERR_EXIT("socket error");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt error");

	/* The call to the function "bind()" assigns the details specified
	 * in the structure 'servaddr' to the socket created in the step above
	 */
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

	/* The call to the function "listen()" with second argument as 10 specifies
	 * maximum number of client connections that server will queue for this listening
	 * socket.
	 */
    if (listen(listenfd, SOMAXCONN) < 0) //listen should be after socket and bind, but before accept
        ERR_EXIT("listen error");

    struct sockaddr_in peeraddr; //output parameters
    socklen_t peerlen = sizeof(peeraddr); //input/output parameters, must have an initial value
    int conn; // can actively connect

    pid_t pid;

    while (1)
    {
		/* In the call to accept(), the server is put to sleep and when for an incoming
		 * client request, the three way TCP handshake* is complete, the function accept()
		 * wakes up and returns the socket descriptor representing the client socket.
		 */
        if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0) 
            ERR_EXIT("accept error");
        printf("recv connect ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr),
               ntohs(peeraddr.sin_port));

        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork error");
        if (pid == 0)
        {
            // Child process
            close(listenfd);
            SERVER_RECV(conn);
            exit(EXIT_SUCCESS);
        }
        else
            close(conn); //Parent process
    }

    return 0;
}

void SERVER_RECV(int conn)
{
    char recvbuf[1024];
    while (1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        // Receive
		int ret = read(conn, recvbuf, sizeof(recvbuf) - 1);
		
		if (ret == -1) 
		{
			exit(-2);
		}
		else if (ret == 0) 
		{ 
			close(conn);
			break;
		}

		// Treat the received data as a string, add '\0' at the end and print it out
		recvbuf[ret] = '\0';
		printf("Recieved: %s\n", recvbuf);
		
    }
}
