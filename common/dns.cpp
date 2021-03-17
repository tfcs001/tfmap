#include "utils.h"


int ip_to_hostname(const char* ip)
{
    int ret = 0;
 
    if(!ip)
    {
        printf("invalid params\n");
        return -1;
    }
 
    struct addrinfo hints;
    struct addrinfo *res, *res_p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME | AI_NUMERICHOST;
    hints.ai_protocol = 0;
 
    ret = getaddrinfo(ip, NULL, &hints, &res);
    if(ret != 0)
    {
        printf("getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }
 
    for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
    {
        char host[1024] = {0};
        ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NAMEREQD);
        if(ret != 0)
        {
            printf("getnameinfo: %s\n", gai_strerror(ret));
        }
        else
        {
            printf("hostname: %s\n", host);
        }
    }
 
    freeaddrinfo(res);
    return ret;
}

int hostname_to_ip(const char* hostname, char *ip)
{
    int ret = 0;
 
    if(!hostname)
    {
        TF_LOG_ERROR("invalid params");
        return -1;
    }
 
    struct addrinfo hints;
    struct addrinfo *res, *res_p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;
 
    ret = getaddrinfo(hostname, NULL, &hints, &res);
    if(ret != 0)
    {
        TF_LOG_ERROR("getaddrinfo(hostname=%s): %s", hostname, gai_strerror(ret));
        return -1;
    }
 
    for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
    {
        char host[1024] = {0};
        ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        if(ret != 0)
        {
            TF_LOG_ERROR("getnameinfo(hostname=%s): %s", hostname, gai_strerror(ret));
        }
        else
        {
            TF_LOG_INFO("hostname:%s, ip:%s", hostname, host);
            memcpy(ip, host, MAX_IP_LEN);
            break;
        }
    }
 
    freeaddrinfo(res);

    return 0;
}
