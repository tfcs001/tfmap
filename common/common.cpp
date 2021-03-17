#include "utils.h"

char g_pos[3] = {'&',0};

/*
Program received signal SIGPIPE, Broken pipe. 0x00007ffff7bcdc5b in send ()
*/
void signal_process()
{
#if __linux__
    signal(SIGPIPE, SIG_IGN);
#endif
	return;
}

/* flag 1=listen fd  0-connnec fd */
int socket_keepalive(int sockfd, int flag)
{
    int keepAlive = 1;    /* 开启keepalive属性 */
    int keepIdle = 1;     /* 如该连接在1+2秒内没有任何数据往来，则进行探测 */
    int keepInterval = 2; /* 探测时发包的时间间隔为2秒 */
    int keepCount = 3;    /* 探测尝试的次数。如果第1次探测包就收到响应了，则后2次的不再发送 */
    int tcp_nodelay = 1;

    /* Solve the problem of incorrect message receiving sequence */
    
    int ret = 0;
    
    if (1 == flag)
    {
        /* avoid occur Address already in use  */
        int reuseaddr = 1;
        if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)))
        {
            TF_LOG_ERROR("SO_REUSEADDR WRONG!(ret=%d, errno=%d, %s)", ret, errno, strerror(errno));
            return -1;
        }
    }

    ret = setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY, (char *)&tcp_nodelay, sizeof(int)); 
    if (ret)
    {
        TF_LOG_ERROR("TCP_NODELAY WRONG!(ret=%d, errno=%d, %s)", ret, errno, strerror(errno));
		return 1;
    }

    ret = setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(char *)&keepAlive,sizeof(keepAlive));
    if(ret) 
    {
        TF_LOG_ERROR("It is SO_KEEPALIVE WRONG!(ret=%d, errno=%d, %s)", ret, errno, strerror(errno));
        return 1;
    }
#if __linux__
    ret = setsockopt(sockfd,SOL_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle));
    if(ret)
    {
        TF_LOG_ERROR("It is TCP_KEEPIDLE WRONG!(ret=%d, errno=%d, %s)", ret, errno, strerror(errno));
        return 1;
    }

    ret = setsockopt(sockfd,SOL_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval));
    if(ret)
    {
        TF_LOG_ERROR("It is TCP_KEEPINTVL WRONG!(ret=%d, errno=%d, %s)", ret, errno, strerror(errno));
        return 1;
    }

    ret = setsockopt(sockfd,SOL_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount));
    if(ret)
    {
        TF_LOG_ERROR("It is SO_KEEPALIVE WRONG!(ret=%d, errno=%d, %s)", ret, errno, strerror(errno));
        return 1;
    }
#endif

	return 0;
}

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
  GetLocalTime(&wtm);
  tm.tm_year   = wtm.wYear - 1900;
  tm.tm_mon   = wtm.wMonth - 1;
  tm.tm_mday   = wtm.wDay;
  tm.tm_hour   = wtm.wHour;
  tm.tm_min   = wtm.wMinute;
  tm.tm_sec   = wtm.wSecond;
  tm. tm_isdst  = -1;
  clock = mktime(&tm);
  tp->tv_sec = clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}
#endif

long long get_now_time()
{
    struct timeval now;
    long long time = 0;

    gettimeofday(&now, NULL);

    time = now.tv_sec;
//    time = time*1000000 + now.tv_usec;
    
    return time;
}


void get_sys_time (char *result)
{
    time_t  l;
    struct tm *t;

    l = time (NULL);
    t = localtime (&l);

    sprintf (result, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d", t->tm_year + 1900,
        t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return;
}

void trim_blank (char *str)
{
    size_t  len = strlen (str);
    size_t  lws;

    while (len && isspace (str[len - 1]))
        --len;
    if (len)
    {
        lws = strspn (str, " \n\r\t\v");
        memmove (str, str + lws, len -= lws);
    }
    str[len] = 0;
}

/* 0 - num !0- error */
int is_digit(const string& content)
{
    int i = 0;
    for(i=0; i< (int)content.length(); i++)
    {
        if(!(isdigit(content.at(i))))
        {
            return 1;
        }
    }

    return 0;
}

/* 0 - port */
int is_port(const string& port)
{
    if (is_digit(port))
    {
        return 1;
    }

    if (atoi(port.c_str()) < 0 || atoi(port.c_str()) > 65535)
    {
        return 2;
    }

    return 0;
}

int tf_socket(int domain, int type, int protocol)
{
#if _WIN32
	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err;
	wVersionRequested = MAKEWORD(2, 2);
	
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		TF_LOG_ERROR("Load WinSock Failed!");
		return -1;
	}
#endif

	return socket(domain, type, protocol);
}

int tf_close(int fd)
{
    if (-1 == fd)
    {
        return 0;
    }

#if _WIN32
    int ret = closesocket(fd);
    WSACleanup();
	return ret;
#else
	return close(fd);
#endif
}

/* 1 ms  --- 10 ms  --- 100ms -- 1s */
int tf_send(int fd, char *msg, int len, int flag)
{
    int ret = 0;
    ret = send(fd, msg, len, 0);
    if (ret == -1 && errno == 11)
    {         
        usleep(1000);
        ret = send(fd, msg, len, 0);
        if (ret == -1 && errno == 11)
        {         
            usleep(10*1000);
            ret = send(fd, msg, len, 0);
            if (ret == -1 && errno == 11)
            {         
                usleep(100*1000);
                ret = send(fd, msg, len, 0);
                if (ret == -1 && errno == 11)
                {         
                    sleep(1);
                    ret = send(fd, msg, len, 0);
                }
            }
        }
    }

    return ret;
}

/* 0 - yes  1 - no */
int is_ip(const char str[])
{
    int a,b,c,d;
    char temp[100] = {0};

    if((sscanf(str,"%d.%d.%d.%d",&a,&b,&c,&d)) != 4)
    {
        return 1;
    }
        
    sprintf(temp,"%d.%d.%d.%d",a,b,c,d);
    if(strcmp(temp,str) != 0)
    {
        return 1;
    }
        
    if((a <= 255 && a >= 0) && (b <= 255 && b >= 0) && (c <= 255 && c >= 0) && (d <= 255 && d >= 0))
    {
        return 0;
    }

    return 1;
}

/* 0 -use  1- no use */
int  is_port_use(const int port)
{
    return 1;
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = 0;
    
    if(bind(fd, (struct sockaddr *)(&addr), sizeof(sockaddr_in)) == 0) /* success */
    {
        close(fd);
        return 0;
    }
    close(fd);

    return 1;
}

/* -1 failed >0 success */
int get_valid_port(int begin_port, int end_port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    int i = 0;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0;
    
    if (begin_port == -1 || end_port == -1 )
    {
        begin_port = MIN_PORT;
        end_port   = MAX_PORT;
    }

    for (i = begin_port; i <= end_port; i++)
    {
        addr.sin_port = htons(i);
        if(bind(fd, (struct sockaddr *)(&addr), sizeof(sockaddr_in)) == 0)  /* success */
        {
            close(fd);
            return i;
        }
    }

    close(fd);

    return -1;
}

int64_t get_boot_time()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ts.tv_sec;
}