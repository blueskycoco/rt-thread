#include <rtthread.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#define SERV_PORT 22345
#define SERVADDR "4006:e024:680:c6e:223:8bff:fe59:de90"
#define BUF_SIZE 1024
static const char send_data[] = "This is UDP Client from RT-Thread.";
long g_sent1_len=0;

void udpclient6(char *server_addr,int server_port)
{
	char *recv_data;
	int sockfd;
	struct sockaddr_in6 server_addr6, client_addr6;
	socklen_t clientlen;
	
	recv_data = rt_malloc(BUF_SIZE);
	if(recv_data == RT_NULL)
	{
		rt_kprintf("No memory\n");
		return ;
	}
	
	if((sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) == -1)
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
	while(1)
	{
		if(sendto(sockfd, send_data, sizeof(send_data), 0, (struct sockaddr *)&server_addr6, sizeof(server_addr6)) < 0)
		{
			rt_kprintf("Sendto error\n");
			closesocket(sockfd);
			rt_free(recv_data);
			return ;
		}
		g_sent1_len+=sizeof(send_data);
		rt_kprintf("\nSent length = %d ", g_sent1_len);
	}
	/*
	rt_kprintf("Waiting for a reply...\n");
	
	clientlen = sizeof(client_addr6);
	if(recvfrom(sockfd, recv_data, BUF_SIZE, 0, (struct sockaddr *)&client_addr6, &clientlen) < 0)
	{
		rt_kprintf("Recvfrom error\n");
		rt_free(recv_data);
		return ;
	}
	rt_kprintf("got '%s'\n", recv_data);*/
	closesocket(sockfd);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(udpclient6, start udp server via ipv6);
#endif
