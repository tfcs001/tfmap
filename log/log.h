#ifndef _TF_LOG_H_
#define _TF_LOG_H_

enum
{
    TF_EMERG,
    TF_ALERT,
    TF_CRIT,
    TF_ERROR,
    TF_WARN,
    TF_NOTICE,
    TF_INFO,
    TF_DEBUG
};

enum
{
    TF_STDOUT,
    TF_SYSLOG
};

#ifdef __cplusplus
extern "C" {
#endif

void set_log_para(const char *level, const char *output);

void tf_write(int prio, const char *fmt, ...);

#define TF_LOG_EMERG(MSG, ...)         tf_write(TF_EMERG,  "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_ALERT(MSG, ...)         tf_write(TF_ALERT,  "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_CRIT(MSG, ...)          tf_write(TF_CRIT,   "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_ERROR(MSG, ...)         tf_write(TF_ERROR,  "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_WARN(MSG, ...)          tf_write(TF_WARN,   "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_NOTICE(MSG, ...)        tf_write(TF_NOTICE, "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_INFO(MSG, ...)          tf_write(TF_INFO,   "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)
#define TF_LOG_DEBUG(MSG, ...)         tf_write(TF_DEBUG,  "%s: " MSG, __FUNCTION__, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
