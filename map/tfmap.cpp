#include "../common/utils.h"
#include "../config/config.h"
#include "process_map.h"
#include <sys/resource.h>

#define COMPILE_DATE __DATE__  /* compile date */
#define COMPILE_TIME __TIME__  /* compile time */

#define VERSION_NUM          "v1.0.0"
#define PROC_NAME            "tfmap"

typedef struct S_TF_MAP_CONFIG
{
    string ip;
    string port;
    string username;
    string password;
} S_TF_MAP_CONFIG;


#ifdef SINGLE  

static void show_copyright()
{
    struct tm when;
    time_t now;

    char stars[] = {"***************************************************************"};
    char spaces[] = {"*                                                             *"};
    char ver[81], info[81], author[81], company[81], copyright[81];

    unsigned int len = strlen(stars);

    time(&now);
    localtime_r(&now, &when);


    sprintf(ver, "*                    %10s %5s                        *",
                 PROC_NAME, VERSION_NUM);

    if (strlen(ver) != len)
    {
        ver[len - 1] = '*';
        ver[len]   = '\0';
    }

    sprintf(info, "*            Compiled at %8s, on %12s            *",
            COMPILE_TIME, COMPILE_DATE);
    sprintf(author,
            "*                Author:     tfcs                              *");
    sprintf(company,
            "*                  Time:   2019-01 ~~ %4d-%02d                 *",
            when.tm_year + 1900, when.tm_mon + 1);
    sprintf(copyright,
            "*                  (c) All Rights Reserved.                   *");

    printf("\n");
    printf("%s\n", stars);
    printf("%s\n", spaces);
    printf("%s\n", ver);
    printf("%s\n", info);

    printf("%s\n", spaces);
    printf("%s\n", spaces);
    printf("%s\n", spaces);
    printf("%s\n", author);
    printf("%s\n", spaces);
    printf("%s\n", company);
    printf("%s\n", copyright);
    printf("%s\n", stars);
}

static int is_prog_run()  
{  
    long pid = 0;  
    char command[1024] = {0};  
    char proc_name[1024] = {0};  
    int fd = 0;  
  
    pid = getpid();  
    
    DIR *dir;  
    struct dirent *result = NULL;  
    dir = opendir("/proc");  
    while((result = readdir(dir)) != NULL)  
    {  
        if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, "..") 
            || !strcmp(result->d_name, "self") || atol(result->d_name) == pid)  
        {
            continue;  
        }
        memset(command, 0, sizeof(command));  
        memset(command, 0 ,sizeof(command));  
        sprintf(command, "/proc/%s/cmdline", result->d_name);  
        if (access(command, F_OK) == 0) 
        {  
            fd = open(command,O_RDONLY);  
            if (fd == -1) 
            {
                continue;  
            }
            read(fd, proc_name, 1024);  
            close(fd);  

            if (!strcmp(proc_name, PROC_NAME)) 
            {
                return 0;
            }
        }  
    }  
    return 1;  
} 
#endif

static void usage()
{
    printf("usage:  [-c file][-d]\n\
    where:\n\
        -c - config file\n\
        -d - daemon mode\n");
}

static S_TF_MAP_CONFIG tf_map_config;
void *map_thread_process(void *arg)
{
    TFMap tf_map(tf_map_config.ip, tf_map_config.port, 5, tf_map_config.username, tf_map_config.password);
    tf_map.start();

    return NULL;
}

#ifdef SINGLE
int main(int argc, char **argv)
{
#else
int start_tf_map(const char *file_path, int flag_daemon)
{
#endif

#ifdef SINGLE  
    char *file_path = NULL;
    int flag_daemon = 0;
    int op = 0; 
    while ((op = getopt(argc, argv, "vVdhHc:")) != -1)
    {
        switch(op)
        {
            case 'v':
            case 'V':
                show_copyright();
                return 0;
            case 'd':
                flag_daemon = 1;
                break;
            case 'c':
                file_path = optarg;
                break;
            case 'h':
            case 'H':
            default:
                usage();
                return 1;
        }
    }
        
    show_copyright();

    if (!is_prog_run())
    {
        printf("\nprogram(%s) is running, please kill first.\n", PROC_NAME);
        return 1;
    }

    if (flag_daemon)
    {
        daemon(0, 1);
    }

#endif

    if (NULL == file_path)
    {
         printf("not found config file\n");
         usage();
         return 1;
    }

    if (0 != access(file_path, F_OK))
    {
         printf("not found config file(%s)\n", file_path);
         usage();
         return 1;
    }

#if __linux__      
    signal_process();
#endif

    CIni ini;
    ini.OpenFile(file_path, "r");

    string map_switch = ini.GetStr("module", "tfmap");
    if(map_switch != "on")
    {
        return -1;
    }

    string level  = ini.GetStr("log", "level");
	TF_LOG_INFO("log level:%s", level.c_str());

#if _WIN32
	string output = "stdout";
#else
    string output = ini.GetStr("log", "ouput");
#endif

    TF_LOG_INFO("log output:%s", output.c_str());
    set_log_para(level.c_str(), output.c_str());

    tf_map_config.ip  = ini.GetStr("tfmap", "listen_ip");
    if ("" == tf_map_config.ip)
    {
        TF_LOG_ERROR("please set tfmap listen_ip");
        return 1;
    }
    TF_LOG_INFO("tfmap listen_ip:%s", tf_map_config.ip.c_str());

    tf_map_config.port  = ini.GetStr("tfmap", "listen_port");
    if ("" == tf_map_config.port)
    {
        TF_LOG_ERROR("please set tfmap listen_port");
        return 1;
    }
    TF_LOG_INFO("tfmap listen_port:%s", tf_map_config.port.c_str());

    tf_map_config.username  = ini.GetStr("tfmap", "username");
    tf_map_config.password  = ini.GetStr("tfmap", "password");
    if ("" == tf_map_config.username && "" == tf_map_config.password )
    {
        TF_LOG_INFO("not set tfmap username and password");
    }
    else if ("" == tf_map_config.username && "" != tf_map_config.password )
    {
        TF_LOG_INFO("please set tfmap username");
        return 1;
    }
    else if ("" != tf_map_config.username && "" == tf_map_config.password )
    {
        TF_LOG_INFO("please set tfmap password");
        return 1;
    }
    else
    {
        TF_LOG_INFO("tfmap username=%s, password=%s", tf_map_config.username.c_str(), tf_map_config.password.c_str());
    }

#ifdef SINGLE 

	rlimit fdLimit;

	fdLimit.rlim_cur = 10000;
	fdLimit.rlim_max = 10000;
 	if (-1 == setrlimit (RLIMIT_NOFILE, &fdLimit))
 	{
  		TF_LOG_ERROR("Set max fd open count fail");
		return 1;
	}
	fdLimit.rlim_cur = 0;
	fdLimit.rlim_max = 0;
  	if (-1 == getrlimit (RLIMIT_NOFILE, &fdLimit))
  	{
   		TF_LOG_ERROR("get ulimit fd number failed");
		return 1;
  	}
	TF_LOG_NOTICE("ulimit hard rlim_cur=%d, soft rlim_max=%d", fdLimit.rlim_cur, fdLimit.rlim_max);

    pthread_t map_thread_id;
    if (0 != pthread_create(&map_thread_id, NULL, map_thread_process, NULL))
    {
        TF_LOG_ERROR("create map thread failed");
        return 1;
    }
    else
    {
        TF_LOG_INFO("create map thread success");
    }
      
    while (1)
    {
        sleep(10);
    }
#endif

    return 0;
}
