#ifndef _PROCESS_MAP_H_
#define _PROCESS_MAP_H_

#define MAP_CONNECT_SUCCESS   10
#define MAP_CONNECT_FAILED    1
#define MAP_CREATE_THREAD     2
#define MAP_RECONNECT         3

class MapClient
{
public:
    MapClient():accept_fd(-1),connect_fd(-1),flag(0),connect_name(""),connect_ip(""),connect_port(0),active_time(0) {}

    ~MapClient() {}

    MapClient & operator=(const MapClient &ob )
    {
        accept_fd       = ob.accept_fd;
        connect_fd      = ob.connect_fd;
        flag            = ob.flag;
        connect_name    = ob.connect_name;
        connect_ip      = ob.connect_ip;
        connect_port    = ob.connect_port;

        action          = ob.action; 
        memcpy(msg, ob.msg, sizeof(ob.msg));
        len             = ob.len;

        active_time     = ob.active_time;

        thread_id       = ob.thread_id;

        return *this;
    }
    
public:
    int         accept_fd;                     /* map --- local fd */
    int         connect_fd;                    /* map --- remote fd */
    int         flag;                          /* 0 - unuse  1 - connect success  0!(2) create thread */

    string      connect_name;
    string      connect_ip;
    int         connect_port;

    int         action;
    char        msg[MAX_MAP_MSG_LEN+1];
    int         len;

    int64_t     active_time;

    pthread_t thread_id;
};



class TFMap
{
public:
    TFMap(string _ip, string _port, int _num, string _username, string _password) :
         listen_ip(_ip), listen_port(_port), listen_num(_num), username(_username), password(_password){}

    ~TFMap() {}

    static map<int, MapClient> g_accept_map;      /* local  --- map client */
    static map<int, MapClient> g_connect_map;     /* remote --- map client */

    static mutex g_map_lock;
    static mutex g_map_del_lock;

    int set_noblock(int fd);

    void add_epoll_fd(int epfd, int fd);

    // void del_epoll_fd(int epfd, int fd);

    int listen_for_client();
 
    int start();

    int process_msg(int fd, char *msg, int len);

    int add_connect_map(int fd);

    int get_action(char *msg);

    static void sendto_client(int fd, char *msg, int len);
    
    static void sendto_remote(int fd, char *msg, int len);

    static void release_accept(int accept_fd, int epoll_fd);

private:
    
    string listen_ip;

    string listen_port;

    int listen_num;

    string username;
    
    string password;

    unordered_set<string> whitelist_domain;   /* whitelist */
};

int del_connect_map(int fd);

typedef struct T_DOMAIN
{
    string ip;
    uint8_t pos;   /*0-cn 1-!cn*/
} T_DOMAIN;


#endif
