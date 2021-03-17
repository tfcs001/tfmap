#ifndef _UTILS_H_
#define _UTILS_H_



#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#if WIN32
#include <windows.h>
#include <pthread.h>
#include <io.h>
#include <process.h>

#pragma comment(lib,"x86/pthreadVC2.lib")
#pragma comment(lib, "ws2_32.lib")

#define sleep(x) Sleep(1000*x)
#define usleep(x) Sleep(x/1000)
#else
#include <unistd.h>

/* client.cpp */
#include <netdb.h>
#include <syslog.h>

#include <dirent.h>  

#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>

/* common.cpp set socket */
#include <netinet/tcp.h>
#include <arpa/inet.h>

#endif

/* log.cpp  */
#include <stdarg.h>

/* main open dir */
#include <signal.h>
#include <fcntl.h>


#include <string>
#include <iostream>

#include <vector>
#include <algorithm>
#include <thread>

#include <map>
#include <unordered_set>

#include <mutex>
#include <fstream>
#include <iostream>


using namespace std;


#include "common.h"
#include "../log/log.h"


#endif
