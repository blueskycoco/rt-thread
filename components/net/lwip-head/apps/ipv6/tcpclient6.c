#include <rtthread.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#define SERV_PORT 1234
#define SERVADDR "fe80::5867:8730:e9e6:d5c5%11"	// Do not use link-local address, lwip-head did not implement it.
#define BUF_SIZE 1024
static const char send_data[] = "This is TCP Client from RT-Thread.";
long g_recv_len=0,g_sent_len=0;
void tcpclient6(char *server_addr,int server_port)
{
	char* recv_data;
	int sockfd, bytes_received;
	struct sockaddr_in6 server_addr6;
	int status = 0;
	g_recv_len=0;
	g_sent_len=0;
	recv_data = rt_malloc(BUF_SIZE);
	if(recv_data == RT_NULL)
	{
		rt_kprintf("No memory\n");
		return ;
	}
	
	if((sockfd = socket(PF_INET6, SOCK_STREAM, 0)) == -1)
	{
		rt_kprintf("Socket error\n");
		rt_free(recv_data);
		return ;
	}
	
	memset(&server_addr6, 0, sizeof(server_addr6));
	server_addr6.sin6_family = AF_INET6;
	server_addr6.sin6_port = htons(server_port);
	if(inet_pton(AF_INET6, server_addr, &server_addr6.sin6_addr.s6_addr) != 1)
	{
		rt_kprintf("inet_pton() error\n");
		rt_free(recv_data);
		return ;
	}
	
	status = connect(sockfd, (struct sockaddr *)&server_addr6, sizeof(server_addr6));
	if(status < 0)
	{
		rt_kprintf("Connect error:%d\n", status);
		rt_free(recv_data);
		return ;
	}
	
	while(1)
	{
		bytes_received = recv(sockfd, recv_data, BUF_SIZE -1, 0);
		if(bytes_received <= 0)
		{
			closesocket(sockfd);
			rt_free(recv_data);
			break;
		}
		recv_data[bytes_received] = '\0';
		if(strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
		{
			closesocket(sockfd);
			rt_free(recv_data);
			break;
		}
		else
		{
			g_recv_len+=bytes_received;
			g_sent_len+=strlen(send_data);
			rt_kprintf("\nReceived length = %d ,Sent length = %d ", g_recv_len,g_sent_len);
		}
		send(sockfd, send_data, strlen(send_data), 0);
	}

	return;		
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(tcpclient6, startup tcp client via ipv6);
#endif
