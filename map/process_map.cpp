
#include "../common/utils.h"
#include "process_map.h"
#include "base64.h"
#include "dns.h"

map<int, MapClient> TFMap::g_accept_map;
map<int, MapClient> TFMap::g_connect_map;
mutex TFMap::g_map_lock;
mutex TFMap::g_map_del_lock;


static map<string, T_DOMAIN> g_dns_map;

void TFMap::sendto_client(int fd, char *msg, int len)
{
    int ret = 0;

    if (len > 0 && fd > 0 )
    {
        ret = tf_send(fd, msg, len, 0);

        if (ret <= 0)
        {         
            TF_LOG_ERROR("[send] ret=%d, errno=%d", ret, errno);
        }
    }
    TF_LOG_DEBUG("[ppp]send msg to client:[fd=%d, ret=%d, len=%d]", fd, ret, len);
}

void TFMap::sendto_remote(int fd, char *msg, int len)
{
    int ret = 0;

    if (len > 0 && fd > 0)
    {
        ret = tf_send(fd, msg, len, 0);
        if (ret <= 0)
        {         
            TF_LOG_ERROR("[send] ret=%d, errno=%d", ret, errno);
        }
    }
    TF_LOG_DEBUG("[ppp]send msg to remote:[%d-%d]", fd, ret);
}
  
int TFMap::listen_for_client()  
{  
    int fd = -1;  
  
    fd = tf_socket(AF_INET, SOCK_STREAM, 0);  
    if( fd == -1 )  
    {
        TF_LOG_ERROR("create socket  %s failed", listen_port.c_str());
        return -1;
    }

    if (socket_keepalive(fd, 1))
    {
        TF_LOG_ERROR("socket_keepalive(%d) error", fd);
        tf_close(fd);
        return -1;
    }
   
    struct sockaddr_in sin;  
    sin.sin_family = AF_INET; 
    sin.sin_addr.s_addr = inet_addr(listen_ip.c_str());
    sin.sin_port = htons(stoi(listen_port, nullptr));  

    if( bind(fd, (struct sockaddr*)&sin, sizeof(sin)) < 0 )  
    {
        TF_LOG_ERROR("[map]bind port  %s failed", listen_port.c_str());
        close(fd);
        return -1;
    }

    if( listen(fd, listen_num) < 0)  
    {
        TF_LOG_ERROR("[map]listen port  %s failed", listen_port.c_str());
        close(fd);
        return -1;
    }

    TF_LOG_INFO("[map]listen port %d success", stoi(listen_port, nullptr));
  
    return fd;
} 

int TFMap::set_noblock(int fd)
{
    int oldopt = fcntl(fd, F_GETFL);
    // int newopt = oldopt | O_NONBLOCK;
    int newopt = oldopt;
    fcntl(fd, F_SETFL, newopt);
    return oldopt;
}


void TFMap::add_epoll_fd(int epfd, int fd)
{
    struct epoll_event ev;

 //   ev.events = EPOLLIN | EPOLLET | EPOLLERR;
    ev.events = EPOLLIN | EPOLLERR;
    ev.data.fd = fd;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev))
    {
        TF_LOG_ERROR("epoll_ctl failed");
    }
    set_noblock(fd);
}

void del_epoll_fd(int epfd, int fd)
{
    struct epoll_event ev;

    ev.data.fd = fd;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev))
    {
        TF_LOG_ERROR("epoll_ctl failed");
    }
}

int TFMap::get_action(char *msg)
{
    int action = 0;

    if (!memcmp(msg, "GET", 3))
    {
        action = TF_GET;
    } 
    else if (!memcmp(msg, "POST", 4))
    {
        action = TF_POST;
    }
#if 1
    else if (!memcmp(msg, "CONNECT", 7))
    {
        action = TF_CONNECT;
    }
#endif

    return action;
}

int TFMap::process_msg(int fd, char *msg, int len)
{
    int action = get_action(msg);

    if (TFMap::g_accept_map.find(fd) != TFMap::g_accept_map.end())
    {
        TFMap::g_accept_map[fd].active_time  = get_boot_time();

        switch(TFMap::g_accept_map[fd].flag)
        {
            case MAP_CONNECT_SUCCESS: /* conenct success */
            {
                if (TF_CONNECT == action)
                {
                    char reply_https[] = "HTTP/1.1 200 Connection established\r\n\r\n";
                    TFMap::sendto_client(fd, reply_https, strlen(reply_https));
                }
                else
                {
                    /* forward */
                    TFMap::sendto_remote(TFMap::g_accept_map[fd].connect_fd, msg, len);
                }
                break;
            }
            case MAP_RECONNECT:  /* reconnect target */
            {
                usleep(1000);   /* wait thread exit */
                TFMap::g_accept_map[fd].len    = len;
                memcpy(TFMap::g_accept_map[fd].msg, msg, sizeof(TFMap::g_accept_map[fd].msg));

                TF_LOG_INFO("reconnect target success fd=%d,name=%s, ip=%d,port=%d",  fd, TFMap::g_accept_map[fd].connect_name.c_str(),
                            TFMap::g_accept_map[fd].connect_ip.c_str(), TFMap::g_accept_map[fd].connect_port);

                add_connect_map(fd);
                break;
            }
            case MAP_CONNECT_FAILED:   /*reconnect failed*/
            {
                return -1;
            }
            case MAP_CREATE_THREAD:    /* create thread success */
            default:
            {
                break;
            }
        }

        return 0;
    }

    if (0 == action)
    {
        TF_LOG_ERROR("[drop]not found action fd=%d,len=%d,msg=%s ", fd, len, msg);
        return 0;
    }
    
 //   TF_LOG_INFO("process http fd=%d,len=%d,msg=%s", fd, len, msg);

    char *bpos = NULL;
    char *epos = NULL;
    char hosts[MAX_PATH_LEN] = "Host:";

    bpos = strstr(msg, hosts);
    if (NULL == bpos)
    {
        if (TF_CONNECT == action)
        {
            char delim[] = "HTTP";
    
            memset(hosts, 0, sizeof(hosts));
            strcpy(hosts, "CONNECT");

            bpos = msg;
            epos = strstr(bpos, delim);
            if (NULL == epos)
            {
                TF_LOG_ERROR("not found end symbol, drop fd=%d,len=%d,msg=%s", fd, len, msg);
                return 0;
            }

            action = TF_CONNECT_SSH;
            epos -= 1;
        }
        else
        {
            TF_LOG_ERROR("not found host, drop fd=%d,len=%d,msg=%s", fd, len, msg);
            return 0;
        }
    }
    else
    {
        char delim[] = "\r\n";
    
        epos = strstr(bpos, delim);
        if (NULL == epos)
        {
            TF_LOG_ERROR("not found end symbol, drop fd=%d,len=%d,msg=%s", fd, len, msg);
            return 0;
        }
    }

    char *minpos = NULL;
    char connect_name[MAX_PATH_LEN] = {0};
    char connect_port[MAX_PORT_LEN] = {0};
    char valid_msg[MAX_MAP_MSG_LEN] = {0};

    memcpy(valid_msg, bpos+strlen(hosts)+1, epos-bpos-strlen(hosts)-1);

    TF_LOG_INFO("----------------valid_msg:%s--------", valid_msg);

    minpos = strstr(valid_msg, ":");
    if (NULL == minpos)
    {
        strcpy(connect_name, valid_msg);
        strcpy(connect_port, "80");
    }
    else
    {
        memcpy(connect_name, valid_msg, minpos-valid_msg);
        strcpy(connect_port, minpos+1);
    }

    /* Proxy-Authorization: Basic dXNlcjp1c2VyMTIzNDU2\r\n */
    if (username != "" || password != "")
    {
        char *bp_pos = NULL;
        char *ep_pos = NULL;
        char auth[MAX_PATH_LEN] = "Proxy-Authorization: Basic";

        bp_pos = strstr(msg, auth);
        if (NULL == bp_pos)
        {
            TF_LOG_WARN("client not set username and password");
            return -1;
        }

        char pdelim[] = "\r\n";
    
        ep_pos = strstr(bp_pos, pdelim);
        if (NULL == ep_pos)
        {
            TF_LOG_ERROR("not found end symbol");
            return -1;
        }

        char valid_auth_msg[MAX_MAP_MSG_LEN] = {0};

        memcpy(valid_auth_msg, bp_pos+strlen(auth)+1, ep_pos-bp_pos-strlen(auth)-1);

        TF_LOG_INFO("----------------valid_auth_msg:%s--------", valid_auth_msg);

        string auth_info = username + ":" + password;
        char encode_auth[MAX_MSG_LEN] = {0};
        int encode_len = 0;
        int encode_ret = 0;
        
        encode_ret = base64_encode(auth_info.c_str(), auth_info.length(), encode_auth, &encode_len);
        if (0 != encode_ret || encode_len >= (int)sizeof(encode_auth))
        {
            TF_LOG_ERROR("encode failed(ret=%d, encode_len=%d, len=%d)", encode_ret, encode_len, sizeof(encode_auth));
            return -1;
        }

        if (memcmp(encode_auth, valid_auth_msg, encode_len))
        {
            TF_LOG_ERROR("auth not match(valid_auth_msg=%s, encode_auth=%s, len=%d)", valid_auth_msg, encode_auth, encode_len);
            return -1;
        }
    }

 #if 0  
    if (whitelist_domain.find(connect_name) == whitelist_domain.end())
    {
        TF_LOG_ERROR("not match white list domain(%s:%s)", connect_name, connect_port);
        /* add untrust list */
	    return -1;
    }
    else
    {
        /* add trust list */
    }
#endif

    MapClient map_client;

    map_client.accept_fd = fd;
    map_client.connect_name = connect_name;
    map_client.connect_ip = "";
    map_client.connect_port = atoi(connect_port);
    map_client.active_time  = get_boot_time();

    map_client.action = action;
    map_client.len    = len;
    memcpy(map_client.msg, msg, sizeof(map_client.msg));

    TFMap::g_accept_map.insert(pair<int, MapClient>(fd,  map_client));

    add_connect_map(fd);

    return 0;
}

void TFMap::release_accept(int accept_fd, int epoll_fd)
{
    TFMap::g_map_del_lock.lock();

    /* avoid delete */
    if (TFMap::g_accept_map.find(accept_fd) != TFMap::g_accept_map.end())
    {
        del_epoll_fd(epoll_fd, accept_fd);
        tf_close(accept_fd);
        del_connect_map(accept_fd);
        TFMap::g_accept_map.erase(accept_fd);
    }

    TFMap::g_map_del_lock.unlock();             
}

static void *timer_process(void *arg)
{
    return NULL;
    int epoll_fd = *(int *)arg;
    while (1)
    {
        int64_t cur_boot_time = get_boot_time();

        /* > 1000 */
        for(auto &it : TFMap::g_accept_map)
        {
            int accept_fd = it.first;
            MapClient map_client;
            if (TFMap::g_accept_map.find(accept_fd) != TFMap::g_accept_map.end())
            {
                map_client = TFMap::g_accept_map[accept_fd];
            }
            else
            {
                break;
            }

            if (cur_boot_time-map_client.active_time > MAX_MAP_TIMEOUT)
            {
                /* release session */
                TF_LOG_INFO("timeout release fd(name:%s, ip:%s, port:%d, accept_fd:%d, connect_fd:%d)",
                            map_client.connect_name.c_str(), map_client.connect_ip.c_str(), map_client.connect_port, 
                            map_client.accept_fd, map_client.connect_fd);

                TFMap::release_accept(map_client.accept_fd, epoll_fd);
            }
        }

        sleep(60);
    }
    return NULL;
}

int TFMap::start()
{
    int listen_fd = 0;
    int epoll_fd = -1;

    listen_fd = listen_for_client();  
    if( listen_fd == -1 )  
    {  
        TF_LOG_ERROR("client:tcp_server_init error ");  
        return 1;  
    }

    if (-1 == (epoll_fd = epoll_create(EPOLL_SIZE)))
    {
        TF_LOG_ERROR("epoll_create failed");
        close(listen_fd);
        return 1;
    }

    whitelist_domain.insert("10.222.2.122");
    whitelist_domain.insert("10.222.2.177");
    whitelist_domain.insert("10.230.2.163");
    whitelist_domain.insert("10.222.2.126");
    whitelist_domain.insert("10.216.2.162");

    add_epoll_fd(epoll_fd, listen_fd);
    struct epoll_event evs[EPOLL_SIZE] = {0};

    /* create timer thread */
    pthread_t timer_thread_id;
    if (0 != pthread_create(&timer_thread_id, NULL, timer_process, &epoll_fd))
    {
        TF_LOG_ERROR("create timer thread failed");
        close(listen_fd);
        return 1;
    }

    TF_LOG_INFO("create timer thread success");

    while (1)
    {
        int num = epoll_wait(epoll_fd, evs, EPOLL_SIZE,  -1);
        for (int i = 0; i < num; i++)
        {
            if (evs[i].data.fd == listen_fd && (evs[i].events & EPOLLIN))
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int cfd = accept(listen_fd, (struct sockaddr*)&client, &len);
                if (cfd == -1)
                {
                    TF_LOG_ERROR("accept failed(fd:%d) ", evs[i].data.fd);
                    continue;
                }
                if(socket_keepalive(cfd))
                {
                    TF_LOG_ERROR("socket_keepalive(%d) error", cfd);
                    tf_close(cfd);
                    continue;
                }

                add_epoll_fd(epoll_fd, cfd);
                TF_LOG_INFO("accept client connection: %d ", cfd);
            }
            else if (evs[i].events & EPOLLIN)
            {              
                char msg[MAX_MAP_MSG_LEN+1] = {0};
                int len = recv(evs[i].data.fd, msg, MAX_MAP_MSG_LEN, 0);
                if (len > 0)
                {
                    /* parse http header */
                    if (-1 == process_msg(evs[i].data.fd, msg, len))
                    {
                        TF_LOG_WARN("remote connect failed， local fd=%d, ret=%d", evs[i].data.fd, len);                        
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    TF_LOG_WARN("local recv failed， fd=%d, ret=%d, errno=%d", evs[i].data.fd, len, errno);
                }

                /* disconnect local client */
                del_epoll_fd(epoll_fd, evs[i].data.fd);
                tf_close(evs[i].data.fd);
                
                del_connect_map(evs[i].data.fd);

                TFMap::g_accept_map.erase(evs[i].data.fd);
            }
            else if (evs[i].events & EPOLLERR)
            {
                TF_LOG_WARN("client accept fd=%d close ", evs[i].data.fd);
                del_epoll_fd(epoll_fd, evs[i].data.fd);
                tf_close(evs[i].data.fd);
                
                del_connect_map(evs[i].data.fd);     

                TFMap::g_accept_map.erase(evs[i].data.fd);       
            }
            else if (evs[i].events & EPOLLOUT)
            {
                TF_LOG_WARN("EPOLLOUT %d ", evs[i].data.fd);;  /* send */
            }
        }
    }

    TF_LOG_ERROR("exit local process");

    close(listen_fd);
    close(epoll_fd);

    return 0; 
}

#if 0
#define MAX_BUFF_LINE_SIZE 1024
int get_cmd_content(const char *cmd_line, char* buff, int buff_len) 
{
    FILE *fp = NULL;
    char line_buf[MAX_BUFF_LINE_SIZE] = { 0 };
    int copy_len = 0;

    if (NULL == cmd_line)    
    {
        TF_LOG_ERROR("cmd is NULL");
        return 1;
    }

    if (NULL != buff)
    {
        memset(buff, 0, buff_len);
    }
 
    fp = popen(cmd_line, "r");
    if(NULL == fp)
    {
        TF_LOG_ERROR("popen: %s faild!", cmd_line);    
        return 1;
    }
    while(NULL != fgets(line_buf, sizeof(line_buf), fp))
    {
        int str_len = strlen(line_buf)-1;
        copy_len = str_len < (buff_len - 1 ) ? str_len : (buff_len - 1);
        strncpy(buff, line_buf, copy_len);

        break;
    }
    pclose(fp);
 
    return 0;
}
#endif

static int get_ip_addr(const char *name,  char *ip, int *flag)
{
    if (!is_ip(name))
    {
        strcpy(ip, name);
        return 0;
    }

    /* Determine whether it is a foreign domain name */
    char dns_ip[MAX_IP_LEN] = {0};
    int ret = 0;

    if (g_dns_map.find(name) != g_dns_map.end())
    {
        strcpy(ip, g_dns_map[name].ip.c_str());
        return 0;
    }    

    if (NULL != strstr(name, "google") || NULL != strstr(name, "gstatic"))
    {
        strcpy(dns_ip, DNS_FOREIGH_IP);
        *flag = 1;
    }
    else
    {
        strcpy(dns_ip, DNS_HOME_IP);
    }

    ret = hostname_to_ip(name, ip);
    if (!ret)
    {
        T_DOMAIN domain_info;

        domain_info.ip = ip;
        domain_info.pos = *flag;
        g_dns_map.insert(pair<string, T_DOMAIN>(name,  domain_info));
    }

    return ret;

#if 0
    char cmd_line[MAX_BUFF_LINE_SIZE] = {0};
    sprintf(cmd_line, "%s %s %s %s", "nslookup", name, dns_ip, " | grep Address | grep -v '#' | head -n 1 | awk '{print $2}'");
    TF_LOG_INFO("cmd:%s", cmd_line);
    return get_cmd_content(cmd_line, ip, MAX_IP_LEN);
#endif

}

int connect_target(int fd, int flag)
{
    int   connect_fd = -1;
    int   foreign_flag = 0;
    int   target_port = TFMap::g_accept_map[fd].connect_port;
    char  target_ip[MAX_IP_LEN]     = {0};

    if (TFMap::g_accept_map[fd].connect_ip == "")
    {
        if (0 != get_ip_addr(TFMap::g_accept_map[fd].connect_name.c_str(), target_ip, &foreign_flag))
        { 
            TF_LOG_ERROR("get ip addr error(fd=%d, %s:%d)",  fd, TFMap::g_accept_map[fd].connect_name.c_str(), 
                            TFMap::g_accept_map[fd].connect_port);

            TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;
            return 1;
        }
        TFMap::g_accept_map[fd].connect_ip = target_ip;
    }
    else
    {
        strcpy(target_ip, TFMap::g_accept_map[fd].connect_ip.c_str());
    }

    TF_LOG_INFO("connect target begin(fd=%d, name=%s, %s:%d)",  fd, TFMap::g_accept_map[fd].connect_name.c_str(), 
                        target_ip,  TFMap::g_accept_map[fd].connect_port);

    connect_fd = tf_socket(AF_INET, SOCK_STREAM, 0);
    if (connect_fd < 0)
    {
        TF_LOG_ERROR("socket error (name=%s:%d fd=%d, errno=%d)", 
               TFMap::g_accept_map[fd].connect_name.c_str(), TFMap::g_accept_map[fd].connect_port, connect_fd, errno);

        /* 后续可尝试不同ip连接 ??? */

        TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;

        return 1;
    }

#if 0
    if (1 == foreign_flag)
    {
        struct sockaddr_in local_addr = {0};
        local_addr.sin_family = AF_INET;  
        local_addr.sin_addr.s_addr = inet_addr(EXPORT_FOREIGH_IP);
        int ret = bind(connect_fd, (struct sockaddr *) &local_addr, sizeof(local_addr) );
        if(ret == -1)
        {
            TF_LOG_ERROR("bind socket error. fd=%d, errno=%d", connect_fd, errno);
            tf_close(connect_fd);

            TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;
            return 1;
        }
    }
    else
    {
        struct sockaddr_in local_addr = {0};
        local_addr.sin_family = AF_INET;  
        local_addr.sin_addr.s_addr = inet_addr(EXPORT_FOREIGH_IP);
        int ret = bind(connect_fd, (struct sockaddr *) &local_addr, sizeof(local_addr) );
        if(ret == -1)
        {
            TF_LOG_ERROR("bind socket error. fd=%d, errno=%d", connect_fd, errno);
            tf_close(connect_fd);

            TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;
            return 1;
        }
    }
#endif

#if 0
    www.bing.com
0.0.0.0:13128           0.0.0.0:*            
192.168.226.133:55576   39.108.221.136:16818 
10.77.77.2:48554        202.89.233.100:443   
192.168.12.250:13128    192.168.12.107:58750 
10.77.77.2:60012        172.217.25.14:443    
192.168.12.250:13128    192.168.12.107:58734 
#endif

    if(socket_keepalive(connect_fd))
    {
        TF_LOG_ERROR("socket_keepalive(%d) error", connect_fd);
        tf_close(connect_fd);
        TFMap::g_accept_map[fd].connect_fd = -1;
        TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;
        return 1;
    }
    TFMap::g_accept_map[fd].connect_fd = connect_fd;

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(target_port);

#if 0
    TFMap::g_map_lock.lock(); 
    /* add connect valid */
    struct hostent *connect_host = gethostbyname( TFMap::g_accept_map[fd].connect_name.c_str() );
	TFMap::g_map_lock.unlock(); 
	
    if (NULL == connect_host)
    {
        TF_LOG_ERROR("gethostbyname failed remote(%s-%d) ", target_ip, target_port);
        tf_close(connect_fd);

        TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;
        return 1;
    }
    memcpy( (void *) &servaddr.sin_addr, (void *) connect_host->h_addr, connect_host->h_length );
#endif

    servaddr.sin_addr.s_addr = inet_addr(target_ip);

    /* timeout process */
    if (connect(connect_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        TF_LOG_ERROR("connect target failed(%s:%d) ", target_ip, target_port);
        tf_close(connect_fd);

        if (TFMap::g_accept_map.find(fd) != TFMap::g_accept_map.end())
        {
            TFMap::g_accept_map[fd].flag = MAP_CONNECT_FAILED;
            TFMap::g_accept_map[fd].connect_fd = -1;
        }

        return 1;
    }

    TF_LOG_INFO("connect target success fd=%d, connect fd=%d ",  fd, connect_fd);

    if (TFMap::g_accept_map.find(fd) != TFMap::g_accept_map.end())
    {
        if (0 == flag)
        {
            TF_LOG_INFO("accept fd=%d, ip=%s, port=%d, connect fd=%d success",  fd, 
                TFMap::g_accept_map[fd].connect_ip.c_str(), TFMap::g_accept_map[fd].connect_port, connect_fd);

            TFMap::g_accept_map[fd].flag = MAP_CONNECT_SUCCESS;
            if (TFMap::g_accept_map[fd].action == TF_CONNECT)
            {
                char reply_https[] = "HTTP/1.1 200 Connection established\r\n\r\n";
                TFMap::sendto_client(fd, reply_https, strlen(reply_https));
            }
            else
            {
                TFMap::sendto_remote(connect_fd, TFMap::g_accept_map[fd].msg, TFMap::g_accept_map[fd].len);
            }
        }
    }  
    else
    {
        TF_LOG_ERROR("g_accept_map(fd:%d, connect fd:%d) not exist", fd, connect_fd);
        tf_close(connect_fd);
        return 1;
    }

    char msg[MAX_MAP_MSG_LEN+1] = {0};
    int  len = 0;
    for (;;) 
    {
        memset(msg, 0, sizeof(msg));
        len = recv(connect_fd, msg, MAX_MAP_MSG_LEN, 0);
        if (len > 0)
        {
            TF_LOG_DEBUG("[ppp]recv target msg:[fd=%d, connect fd=%d, len=%d]", fd, connect_fd, len); 
            TFMap::sendto_client(fd, msg, len);
        }
        else
        {
            TF_LOG_ERROR("[ppp]recv target msg:[fd=%d, connect fd=%d, ip=%s, port=%d, len=%d, errno=%d]", 
                       fd, connect_fd, target_ip, target_port, len, errno);
            tf_close(connect_fd);

            if (TFMap::g_accept_map.find(fd) != TFMap::g_accept_map.end())
            {
                TFMap::g_accept_map[fd].flag = MAP_RECONNECT;
                TFMap::g_accept_map[fd].connect_fd = -1;
            } 
            break;
        }  
    }
    TF_LOG_INFO("close map(ip=%s, port=%d) success", target_ip, target_port);

    return 1;
}

static int g_flag_thread = 0;

static void *thread_process(void *arg)
{
    int fd = *(int *)arg;
    g_flag_thread = 1;

    if (TFMap::g_accept_map.find(fd) != TFMap::g_accept_map.end())
    {
        TFMap::g_accept_map[fd].flag = MAP_CREATE_THREAD;

        connect_target(fd, 0);
    }

    TF_LOG_ERROR("thread exit(accept fd=%d)", fd);

    return NULL;
}

int TFMap::add_connect_map(int fd)
{
    g_flag_thread = 0;
    TFMap::g_map_lock.lock();

    if (0 != pthread_create(&TFMap::g_accept_map[fd].thread_id, NULL, thread_process, &fd))
    {
        TF_LOG_ERROR("create thread(accept fd=%d, thread_id=%d) failed", fd, TFMap::g_accept_map[fd].thread_id);
    }
    else
    {
        TF_LOG_INFO("create thread(accept fd=%d, thread_id=%d) success", fd, TFMap::g_accept_map[fd].thread_id);
    }
    
    TFMap::g_map_lock.unlock();

    while (0 == g_flag_thread)
    {
        usleep(1000);
    }
 
    return 0;
}

static void thread_quit(pthread_t thread_id)
{
    int kill_rc = pthread_kill(thread_id, 0);

    if(kill_rc == ESRCH)
    {
        TF_LOG_DEBUG("the specified thread did not exists or already quit(thread_id=%u)", thread_id);
    }
    else if(kill_rc == EINVAL)
    {
        TF_LOG_DEBUG("signal is invalid(thread_id=%u)", thread_id);
    }
    else
    {
        TF_LOG_DEBUG("the specified thread is alive(thread_id=%u)", thread_id);

        if (0 == pthread_cancel(thread_id))
        {
            TF_LOG_DEBUG("send pthread_cancel(thread_id=%u) begin", thread_id);

            /* 避免取消的线程再拿锁，导致线程不能退出 pthread_cancel wait cancel point */
            TFMap::g_map_lock.unlock(); 
            usleep(10000);
#if __linux__
            pthread_join(thread_id, NULL);
#endif
            TFMap::g_map_lock.lock(); 
            TF_LOG_DEBUG("send pthread_cancel(thread_id=%u) end", thread_id);
        }
        else
        {
            TF_LOG_ERROR("send pthread_cancel(thread_id=%u) failed", thread_id);
        }

    }
}

int del_connect_map(int fd)
{
    TFMap::g_map_lock.lock();
    
    if (TFMap::g_accept_map.find(fd) == TFMap::g_accept_map.end())
    {
        TF_LOG_ERROR("connect not found accept fd:%d", fd);
        TFMap::g_map_lock.unlock();
        return 1;
    }

    int socketfd = TFMap::g_accept_map[fd].connect_fd;
    if(0 == tf_close(socketfd))
    {
        TF_LOG_NOTICE("close connect accept fd=%d, connect fd=%d success", fd, socketfd);
    }
    else
    {
        TF_LOG_ERROR("close connect  accept fd=%d, connect fd=%d success", fd, socketfd);
    }

    if (0 != TFMap::g_accept_map[fd].flag)
    { 
        thread_quit(TFMap::g_accept_map[fd].thread_id);
    }

//    TFMap::g_accept_map.erase(fd);  /* tmp reserve */

    TFMap::g_map_lock.unlock();


    return 0;
}
