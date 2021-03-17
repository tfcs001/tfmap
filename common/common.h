#ifndef _COMMON_H_
#define _COMMON_H_

#define MAX_SERVER_TO_CLIENT_NUM 1000 /* client hash size */
#define CLIENT_SECRET  "ack"  

#define CLIENT       1
#define TERMINAL     2

#define INSERT      1
#define UPDATE      2
// #define DELETE      3


#define IPADDR     1001
#define MACADDR    1002
#define SN         1003
#define DEVICETYPE 1004
#define EMAIL      1005
#define PASSWORD   1006


#define NEW_PASSWORD     1007
#define ADMIN_EMAIL      1008
#define ADMIN_PASSWORD   1009

#define WEB_PORT      1010
#define FILE_PORT     1011
#define SSH_PORT      1012
#define WEB_ADDR      1013
#define FILE_ADDR     1014

#define TOTAL_DISK_SIZE    1015
#define USE_DISK_SIZE      1016
#define TOTAL_MEM_SIZE     1017
#define USE_MEM_SIZE       1018
#define TOTAL_CPU_SIZE     1019
#define USE_CPU_SIZE       1020
#define RUN_TIME           1021

#define EXPORT_IP          1022
#define EXPORT_MAC         1023

#define TARGET_PORT        2001
#define CONNECT_PORT       2002
#define CS_TOKEN          2003


#define  IP_LEN         16
#define  MAC_LEN        17
#define  SN_LEN         20
#define  DT_LEN         7
#define  EMAIL_LEN      128
#define  PASSWD_LEN     64
#define  VERSION_LEN    64
#define  PORT_LEN       6

#define  SYSNAME_LEN    32

#define MAX_KEY_LEN 128

#define MAX_PORT_LEN  6
#define MAX_IP_LEN    16

#define MAX_PATH_LEN 1024
#define MAX_STR_LEN  1024
#define MAX_MSG_LEN  2048    /* mtu default 1500 */
#define MAX_BUF_LEN  4096
#define MAX_EMAIL_LEN  128


#define CONNECT_SERVER_DELAY_TIME 10
#define BUFSIZE      4096

#define  SUCCESS  0
#define  FAILURE  1


#define  CLIENT_SERVER_LOCAL          'a'

#define  SERVER_REQUSET_DISCONNECT    'd'

#define  CLIENT_SERVER_HEARTBEAT      'h'

#define  CLIENT_PORT                  'p'

#define  CLIENT_CONNECT_SERVICE       's'

#define  REPLY_MSG                    'r'


#define  EPOLL_SIZE     1000

#define  MAX_PKT_LEN_SIZE    4
#define  MAX_INDEX_LEN_SIZE  4

#define  MAX_HEARD_LEN       (MAX_PKT_LEN_SIZE+MAX_INDEX_LEN_SIZE+1)


#define  MAX_MAP_MSG_LEN    (MAX_MSG_LEN-MAX_HEARD_LEN)

#define  MAX_LIMIT_COUNT     5

#define  TF_GET             1
#define  TF_POST            2
#define  TF_CONNECT         3
#define  TF_CONNECT_SSH     3


#define DNS_HOME_IP         "114.114.114.114"
#define DNS_FOREIGH_IP      "8.8.8.8"
#define EXPORT_FOREIGH_IP   "10.77.77.2"
#define EXPORT_HOME_IP      "192.168.226.133"

#define MIN_PORT            30000
#define MAX_PORT            65000 

#define MAX_MAP_TIMEOUT     300

int socket_keepalive(int sockfd, int flag = 0);

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp);
#endif

long long get_now_time();

void get_sys_time (char *result);

void trim_blank (char *str);

int is_digit(const string& content);

extern char g_pos[];

void signal_process();


int tf_socket(int domain, int type, int protocol);

int tf_close(int fd);

int tf_send(int fd, char *msg, int len, int flag);

int is_ip(const char str[]);

int is_port(const string& port);

int  is_port_use(const int port);

int get_valid_port(int begin_port, int end_port);

int64_t get_boot_time();

#endif

