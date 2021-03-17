#include "../common/utils.h" 

#define MAX_LOG_LEN     (10240)
#define MAX_LEVEL_LEN   (10)
#define MAX_TIME_LEN    (128)

static int g_min_prio =  TF_DEBUG;
static int g_output   =  TF_STDOUT;

static inline void get_log_sys_time(char *result)
{
    time_t  l;
    struct tm *t;

    l = time (NULL);
    t = localtime (&l);

    sprintf (result, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d", t->tm_year + 1900,
            t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return;
}

void set_log_para(const char *level, const char *output)
{
    if (level != NULL && level[0] != 0)
    {
        if (!strcmp(level, "debug"))
        {
            if (g_min_prio != TF_DEBUG)
            {
                g_min_prio = TF_DEBUG;
            }
            
        }
        else if (!strcmp(level, "info"))
        {
            if (g_min_prio != TF_INFO)
            {
                g_min_prio = TF_INFO;
            }
        }
        else if (!strcmp(level, "notice"))
        {
            if (g_min_prio != TF_NOTICE)
            {
                g_min_prio = TF_NOTICE;
            }
        }
        else if (!strcmp(level, "warn"))
        {
            if (g_min_prio != TF_WARN)
            {
                g_min_prio = TF_WARN;
            }
        }
        else if (!strcmp(level, "error"))
        {
            if (g_min_prio != TF_ERROR)
            {
                g_min_prio = TF_ERROR;
            }
        }
        else if (!strcmp(level, "crit"))
        {
            if (g_min_prio != TF_CRIT)
            {
                g_min_prio = TF_CRIT;
            }
        }
        else if (!strcmp(level, "alert"))
        {
            if (g_min_prio != TF_ALERT)
            {
                g_min_prio = TF_ALERT;
            }
        }
        else if (!strcmp(level, "emerg"))
        {
            if (g_min_prio != TF_EMERG)
            {
                g_min_prio = TF_EMERG;
            }
        }
        else
        {
            printf("not recognize level(%s)\n", level);
        }
    }

    if (output != NULL && output[0] != 0)
    {
        if (!strcmp(output, "syslog"))
        {
            if (g_output != TF_SYSLOG)
            {
                g_output = TF_SYSLOG;
            }   
        }
        else if (!strcmp(output, "stdout"))
        {
            if (g_output != TF_STDOUT)
            {
                g_output = TF_STDOUT;
            }      
        }
        else
        {
            printf("not recognize output(%s)\n", output);
        }
    }

    printf("log level=%d, output=%d\n", g_min_prio, g_output);

}

void tf_write(int prio, const char *fmt, ...)
{
    if (prio > g_min_prio)
    {
        return;
    }

    char msg[MAX_LOG_LEN] = { 0 };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf (msg, sizeof (msg), fmt, ap);
    va_end(ap);

    char level[MAX_LEVEL_LEN] = { 0 };
    char cur_time[MAX_TIME_LEN] = { 0 };

    switch (prio)
    {
        case TF_DEBUG:
            sprintf(level, "DEBUG");
            break;

        case TF_INFO:
            sprintf(level, "INFO");
            break;

        case TF_NOTICE:
            sprintf(level, "NOTICE");
            break;

        case TF_WARN:
            sprintf(level, "WARNING");
            break;

        case TF_ERROR:
            sprintf(level, "ERROR");
            break;

        case TF_CRIT:
            sprintf(level, "CRITICAL");
            break;

        case TF_ALERT:
            sprintf(level, "ALERT");
            break;

        case TF_EMERG:
            sprintf(level, "EMERGENCY");
            break;

        default:
            break;
    }

    get_log_sys_time(cur_time);

#if __linux__
    if (TF_SYSLOG == g_output)
    {
        openlog("tfcs", LOG_CONS | LOG_PID, 0);  
        syslog(prio,  "%s %s %s",  cur_time, level, msg);  
        closelog();

        return;
    }
    else
#endif
    {
        fprintf(stdout, "%s %s %s\n",cur_time, level, msg);
    }
}
