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

#include"MQTTClient.h"

#define PORT 5000
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void SERVER_RECV(int);

char *address="tcp://localhost:1883";
char *client_id="publish_client";
char *topic="mqtt_examples";
char buf[1024];
const int time_out=10000;
int  rv;
int  QOS=1;
MQTTClient   client;
MQTTClient_connectOptions conn_opts=MQTTClient_connectOptions_initializer;
MQTTClient_message publish_msg=MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

float hexToDec(char *source);
int getIndexOfSigns(char ch);

int main(void)
{
    //
    conn_opts.keepAliveInterval=60;
    conn_opts.cleansession=1;

    MQTTClient_create(&client,address,client_id,MQTTCLIENT_PERSISTENCE_NONE,NULL);
    if((rv=MQTTClient_connect(client,&conn_opts))!=MQTTCLIENT_SUCCESS)
    {
    printf("MQTTClient_connect failure:%s\n",strerror(errno));
        return 0;
    }
    publish_msg.qos=QOS;
    publish_msg.retained=0;

    //
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
    char recvold[1024];
    char recvall[1024];
    char temp[5];
    float temp_trans[4]={0};
    char slice_temp[4];
    char temp_str1[20]={0};
    char temp_str2[20]={0};
    char temp_str3[20]={0};
    int i =0;
    int j = 0;
    int k = 0;
    int start = 0;
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
		//printf("Recieved: %s\n", recvbuf);	

		if (recvbuf[1] == 'E')  //If '$E' temperature message
		{
		    //printf("hex temp ");
		    for ( i = 27; i < 39; i++)   //store temperature i=27~38 to temp array[12]
		    {
			temp[i-27] = recvbuf[i]; 
			//printf("%c", temp[i-27]);
		    }
		    //printf("\n");
		    memset(temp_trans, 0, sizeof(temp_trans));
		    start = 0;
		    for ( i = 0; i < 3; i++)    //transfrom temp array[3] for three times (tree temperature) save to slice_temp[4]
		    {
			memset(slice_temp, 0, sizeof(slice_temp));
			for(j = 0;j < 4; j++)
			{
			     
			     slice_temp[j] = temp[j + start];
			     //printf("%c", slice_temp[j]);
			} 
			printf("\n");
			temp_trans[i] = hexToDec(slice_temp);  //save three dec temperature to temp_trans[]
			//printf("dec temp: %5.1f\n", temp_trans[i]);
			start += 4;
		    }

		    sprintf(temp_str1, "%5.2f", temp_trans[0]);
		    sprintf(temp_str2, "%5.2f", temp_trans[1]);
		    sprintf(temp_str3, "%5.2f", temp_trans[2]);

		    for( i = 0; i < sizeof(recvbuf); i++)   
		    {
			j = i;
			if (recvold[i] == ';')
			{   
			    for(k = 0; k < strlen(temp_str1); k++)
			    {
				recvall[j + k]=temp_str1[k];           //recvall[0-3]
			    }	
			    recvall[j + strlen(temp_str1)] = ';';      //recvall[4]
			    j += strlen(temp_str1)+1;                  //j=4+1
			    for(k = 0; k < strlen(temp_str2); k++)
			    {
				recvall[j + k]=temp_str2[k];           //recvall[5-8]
			    }
			    recvall[j + strlen(temp_str1)] = ';';      //recvall[9]
			    j += strlen(temp_str1)+1;                   //j=10
			    for(k = 0; k < strlen(temp_str3); k++)
			    {
				recvall[j + k]=temp_str3[k];           //recvall[10-15]
			    }
                            i += 2;
			    
			}
			
			if(recvold[i] == '*')    //*53
			{
			    recvall[i + 14] = recvold[i];  //14=4+4+6
			    recvall[i + 15] = recvold[i+1];  //14=4+4+6
			    recvall[i + 16] = recvold[i+2];  //14=4+4+6
			    break;
			}
			else
			{
			    recvall[j] = recvold[i];
			}
		    }
		    printf("Recieve message: %s\n", recvall);
		    memset(recvold, 0, sizeof(recvold));
		    memset(recvall, 0, sizeof(recvall));
		    //printf("after clear old : %s\n", recvold);
		    //printf("after clear all : %s\n", recvall);
		}
                else
		{
		    memcpy(recvold , recvbuf, sizeof recvold);  //store HTP0
		    //printf("Store message: %s\n", recvold);
	 	}
        printf("\n");
        publish_msg.payload=(void *)recvbuf;
        publish_msg.payloadlen=strlen(recvbuf);
        MQTTClient_publishMessage(client,topic,&publish_msg,&token);
        //printf("publish %s to %s\n",recvbuf,topic);

        sleep(3);
    }
}

float hexToDec(char *source)
{
    float sum = 0;
    long t = 1;
    int i, len;
    len = strlen(source);
    for(i = len-1; i >= 0; i--)
    {
	sum += t * getIndexOfSigns(*(source + i));
	t *= 16;
    }
    //printf("temperature hexToDec: %f\n", sum);
    return sum / 10;
}


int getIndexOfSigns(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
	return ch - '0';
    }
    if(ch >= 'A' && ch <= 'F')
    {
	return ch - 'A' + 10;
    }
    if(ch >= 'a' && ch <= 'f')
    {
	return ch - 'a' + 10;
    }
    return -1;
}
