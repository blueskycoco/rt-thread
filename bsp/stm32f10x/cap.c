#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include <signal.h>
#include <fnmatch.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <iconv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <linux/rtc.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/epoll.h>
#define MAXEVENTS 64
#define MAXLOG_SIZE	40*1024*1024
#define BUF_LEN	10*1024*1024
pthread_mutex_t log_mutex	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  log_signal 	= PTHREAD_COND_INITIALIZER;
uint32_t log_w_ptr, log_r_ptr;
uint8_t *log_buffer = NULL;
FILE *g_file_fd;
uint8_t new_file_flag = 1;
void new_log_file(void)
{
	struct timeval    tv;
	struct timezone   tz;
	struct tm         *p;
	char file_name[256] = {0};
	if (new_file_flag) {
		fclose(g_file_fd);
		gettimeofday(&tv, &tz);
		p = localtime(&tv.tv_sec);
		sprintf(file_name, "/home/pi/glass-%04d-%02d-%02d_%02d:%02d:%02d.log", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
		g_file_fd = fopen(file_name,"wb");
		new_file_flag = 0;
	}
}
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0)
	{
		printf("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E': 
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':  
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		printf("com set error");
		return -1;
	}
	//printf("set done!\n");
	return 0;
}
int open_com_port(char *dev)
{
	int fd;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		printf("Can't Open Serial ttySAC3");
		return(-1);
	}
	//else 
	//	printf("open tts/0 .....\n");

	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printf("fcntl failed!\n");
	else
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,FNDELAY));
	if(isatty(STDIN_FILENO)==0)
		printf("standard input is not a terminal device\n");
	//else
	//	printf("isatty success!\n");
	//printf("fd-open=%d\n",fd);
	return fd;
}
int cap_serial(int fd)
{
	int efd,i;
	int index = 0;
	char buff[1024] = {0};
	struct epoll_event event;
	struct epoll_event *events;
	efd = epoll_create1 (0);
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event);
	events = calloc (MAXEVENTS, sizeof(event));
	for(;;) {
		int n;
		n = epoll_wait (efd, events, MAXEVENTS, 5000);
		if(n > 0) {
			for (i=0; i<n; i++) {
				if (events[i].data.fd == fd &&
						(events[i].events & EPOLLIN)) {
					int length = read(events[i].data.fd, buff, 1024);
					if(length > 0) {
						pthread_mutex_lock(&log_mutex);
						if (log_w_ptr + length > BUF_LEN) {
							memcpy(log_buffer + log_w_ptr, buff, BUF_LEN - log_w_ptr);
							memcpy(log_buffer, buff+BUF_LEN-log_w_ptr, length - (BUF_LEN-log_w_ptr));
							log_w_ptr = length - (BUF_LEN-log_w_ptr);
						} else {
							memcpy(log_buffer + log_w_ptr, buff, length);
							log_w_ptr += length;
						}
						pthread_cond_signal(&log_signal);
						pthread_mutex_unlock(&log_mutex);
					}
					break;
				}
			}
		}/* else {
			printf("No data whthin 5 seconds.\n");
			}*/
	}
	free (events);
	close (fd);
	fclose(g_file_fd);
}
void log_write_file(uint32_t index, uint32_t len)
{
	struct timeval    tv;
	struct timezone   tz;
	struct tm         *p;
	uint32_t 			i;
	char str_time[32] = {0};
	for (i=index;i<len; i++) {
		fwrite(log_buffer+i, 1, sizeof(uint8_t), g_file_fd);
		if (log_buffer[i] == '\n') {
			gettimeofday(&tv, &tz);
			p = localtime(&tv.tv_sec);
			sprintf(str_time, "[%02d:%02d:%02d.%06ld]", p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
			fwrite(str_time, 1, strlen(str_time), g_file_fd);
			if (p->tm_hour == 0) {
				if (p->tm_min == 0) {
					new_log_file();
				} else if (p->tm_min > 10) {
					new_file_flag = 1;
				}
			}
		}
	}
}

void *log_write(void *arg)
{
	while (1) {
		pthread_mutex_lock(&log_mutex);
		pthread_cond_wait(&log_signal, &log_mutex);
		if (log_w_ptr != log_r_ptr) {
			if (log_r_ptr < log_w_ptr) {
					log_write_file(log_r_ptr, log_w_ptr);
					log_r_ptr = log_w_ptr;
			} else {
					log_write_file(log_r_ptr, BUF_LEN);
					log_r_ptr = 0;
					log_write_file(log_r_ptr, log_w_ptr);
					log_r_ptr = log_w_ptr;				
			}
			fflush(g_file_fd);
		}
		pthread_mutex_unlock(&log_mutex);

	}
}
int main(int argc, void *argv[])
{
	int fd_com = 0;
	char file_name[256] = {0};
	struct timeval    tv;
	struct timezone   tz;
	struct tm         *p;
	//sleep(10);	
	gettimeofday(&tv, &tz);
	p = localtime(&tv.tv_sec);
	sprintf(file_name, "/home/pi/glass-%04d-%02d-%02d_%02d:%02d:%02d.log", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	if((fd_com=open_com_port("/dev/ttyAMA0"))<0)
	{
		printf("open_port error");
		return -1;
	}
	if(set_opt(fd_com,115200,8,'N',1)<0)
	{
		printf("set_opt error");
		close(fd_com);
		return -1;
	}
	g_file_fd = fopen(file_name,"wb");  
	if(g_file_fd == NULL)  
	{  
		perror("errno");  
	}  
	else   
	{  
		printf("File Open successed!\n");  
	}  
	log_buffer = (uint8_t *)calloc(BUF_LEN, sizeof(uint8_t));
	if (log_buffer == NULL) {
		perror("no enough log buffer\r\n");
		fclose(g_file_fd);
		close(fd_com);
	} else {
		pthread_t tid;
		if (pthread_create(&tid, NULL, log_write, NULL)) {
			printf("error creating thread.");
			fclose(g_file_fd);
			close(fd_com);
		} else
			cap_serial(fd_com);
	}
	return 0;
}
