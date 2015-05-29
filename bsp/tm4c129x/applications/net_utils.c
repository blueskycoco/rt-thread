/*
 * netutils: ping implementation
 */

#include "lwip/opt.h"

#include "lwip/mem.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip.h"

/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

/** ping receive timeout - in milliseconds */
#define PING_RCV_TIMEO 1000
/** ping delay - in milliseconds */
#define PING_DELAY     100

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif
#define IPV6_PING 0
/* ping variables */
static u16_t ping_seq_num;
struct _ip_addr
{
    rt_uint8_t addr0, addr1, addr2, addr3;
};
/** Prepare a echo ICMP request */

static void ping_prepare_echo6( struct icmp6_echo_hdr *iecho, u16_t len)
{
	size_t i;
	size_t data_len = len - sizeof(struct icmp6_echo_hdr);
	iecho->type=ICMP6_TYPE_EREQ;
	iecho->code=0;	
	iecho->chksum = 0;
	iecho->id	  = PING_ID;
	iecho->seqno  = htons(++ping_seq_num);

	/* fill the additional data buffer with some data */
	for(i = 0; i < data_len; i++)
	{
		((char*)iecho)[sizeof(struct icmp6_echo_hdr) + i] = (char)i;
	}
	/*remove chksum ,tm4c129x can hw set chksum*/
	//iecho->chksum = inet_chksum(iecho, len);
}


static void ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);	
    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);	
    iecho->chksum = 0;
    iecho->id     = PING_ID;
    iecho->seqno  = htons(++ping_seq_num);

    /* fill the additional data buffer with some data */
    for(i = 0; i < data_len; i++)
    {
        ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }
	/*remove chksum ,tm4c129x can hw set chksum*/
    //iecho->chksum = inet_chksum(iecho, len);
}

/* Ping using the socket ip */
static err_t ping_send6(int s, struct ip6_addr *addr, int size)
{
	int err;
	struct sockaddr_in6 to;
	struct icmp6_echo_hdr *iecho;
	size_t ping_size = sizeof(struct icmp6_echo_hdr) + size;
	
	LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

	iecho = rt_malloc(ping_size);
	if (iecho == RT_NULL)
	{
		return ERR_MEM;
	}

	ping_prepare_echo6(iecho, (u16_t)ping_size);
	
	to.sin6_len = sizeof(to);
	to.sin6_family = AF_INET6;
	to.sin6_flowinfo = 0;
	memcpy((void *)to.sin6_addr.s6_addr,(unsigned char *)(addr->addr),16);	
	

	err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));
	rt_free(iecho);

	return (err ? ERR_OK : ERR_VAL);
}

static err_t ping_send(int s, struct ip_addr *addr, int size)
{
    int err;
	struct sockaddr_in to;
    struct icmp_echo_hdr *iecho;
	size_t ping_size = sizeof(struct icmp_echo_hdr) + size;
    
    LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

    iecho = rt_malloc(ping_size);
    if (iecho == RT_NULL)
    {
        return ERR_MEM;
    }

    ping_prepare_echo(iecho, (u16_t)ping_size);
	
    to.sin_len = sizeof(to);
    to.sin_family = AF_INET;
	to.sin_addr.s_addr = addr->addr;
    

    err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));
    rt_free(iecho);

    return (err ? ERR_OK : ERR_VAL);
}
static int ping_recv6(int s)
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in6 from;	
    struct ip6_hdr *iphdr;
    struct icmp6_echo_hdr *iecho;
	struct ip6_addr *addr;
	
    while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
    {
        if (len >= (sizeof(struct ip6_hdr)+sizeof(struct icmp6_echo_hdr)))
        {
			addr = (struct ip6_addr *)&(from.sin6_addr.s6_addr); 
            iphdr = (struct ip6_hdr *)buf;
            iecho = (struct icmp6_echo_hdr *)(buf+(IP6H_PLEN(iphdr)));
            if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)))
            {
            	rt_kprintf("ping: recv %4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x\n",
                    IP6_ADDR_BLOCK1(addr),
                    IP6_ADDR_BLOCK2(addr),
                    IP6_ADDR_BLOCK3(addr),
                    IP6_ADDR_BLOCK4(addr),
                    IP6_ADDR_BLOCK5(addr),
                    IP6_ADDR_BLOCK6(addr),
                    IP6_ADDR_BLOCK7(addr),
                    IP6_ADDR_BLOCK8(addr));
                return 0;
            }
            else
            {
                rt_kprintf("ping: drop\n");
            }
        }
    }

    if (len <= 0)
    {
        rt_kprintf("ping: timeout\n");
    }
	return -1;
}

static int ping_recv(int s)
{
    char buf[64];
    int fromlen, len;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;
	struct sockaddr_in from;
    struct _ip_addr *addr;
	
    while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
    {
		if (len >= (sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr)))
        {
            addr = (struct _ip_addr *)&(from.sin_addr);			
            rt_kprintf("ping: recv %d.%d.%d.%d\n", addr->addr0, addr->addr1, addr->addr2, addr->addr3);
            iphdr = (struct ip_hdr *)buf;
            iecho = (struct icmp_echo_hdr *)(buf+(IPH_HL(iphdr) * 4));
            if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)))
            {
                return 0;
            }
            else
            {
                rt_kprintf("ping: drop\n");
            }
        }
    }

    if (len <= 0)
    {
        rt_kprintf("ping: timeout\n");
    }
	return -1;
}

rt_err_t ping_test6(char* target, rt_uint32_t time, rt_size_t size)
{
    int s;
	int result=-1;
    int timeout = PING_RCV_TIMEO;
    struct ip6_addr ping_target;
    rt_uint32_t send_time;
	char *addr;
    send_time = 0;

    if(size == 0)
        size = PING_DATA_SIZE;
	memset(&ping_target, 0, sizeof(ping_target));
	//ping_target.sin6_family = AF_INET6;
	if(inet_pton(AF_INET6, target, (struct ip6_addr*)&ping_target) != 1)
	{
		rt_kprintf("inet_pton() error\n");
		return result;
	}	
	addr = target;
	if ((s = lwip_socket(AF_INET6, SOCK_RAW, IP6_NEXTH_ICMP6)) < 0)
    {
        rt_kprintf("create socket failled\n");
        return -RT_ERROR;
    }
    

    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (1)
    {
        if (ping_send6(s, &ping_target, size) == ERR_OK)
        {
			rt_kprintf("ping: send %s\n", addr);
            result=ping_recv6(s);
        }
        else
        {
			rt_kprintf("ping: send %s error\n", addr);
        }

        send_time ++;
        if (send_time >= time || result==0) break; /* send ping times reached, stop */

        rt_thread_delay(PING_DELAY); /* take a delay */
    }

    lwip_close(s);
	
    return result;
}
rt_err_t ping_test(char* target, rt_uint32_t time, rt_size_t size)
{
    int s;
	int result=-1;
    int timeout = PING_RCV_TIMEO;
    struct ip_addr ping_target;
    rt_uint32_t send_time;
    struct _ip_addr
    {
        rt_uint8_t addr0, addr1, addr2, addr3;
    } *addr;
    send_time = 0;

    if(size == 0)
        size = PING_DATA_SIZE;
    if (inet_aton(target, (struct in_addr*)&ping_target) == 0) return -RT_ERROR;
    addr = (struct _ip_addr*)&ping_target;
	if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
    {
        rt_kprintf("create socket failled\n");
        return -RT_ERROR;
    }
    

    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (1)
    {
        if (ping_send(s, &ping_target, size) == ERR_OK)
        {
            rt_kprintf("ping: send %d.%d.%d.%d\n", addr->addr0, addr->addr1, addr->addr2, addr->addr3);
            result=ping_recv(s);
        }
        else
        {
            rt_kprintf("ping: send %d.%d.%d.%d - error\n", addr->addr0, addr->addr1, addr->addr2, addr->addr3);
        }

        send_time ++;
        if (send_time >= time || result==0) break; /* send ping times reached, stop */

        rt_thread_delay(PING_DELAY); /* take a delay */
    }

    lwip_close(s);
	rt_kprintf("ping return %d\n",result);
    return result;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(ping_test, ping ipv4 network host);
FINSH_FUNCTION_EXPORT(ping_test6, ping ipv6 network host);
#endif
