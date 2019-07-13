#include "log.h"
#include "time.h"
#include <stdarg.h>

LOG_DATA log_file;

void log_init(void)
{
    /* get system time */
    time_t time_v;
    time(&time_v);
    struct tm *time_p = localtime(&time_v);

    sprintf(log_file.path, "/home/ftpuser/log/lora/%4d-%02d.log",
            time_p->tm_year + 1900, time_p->tm_mon + 1);

    log_file.fp = fopen(log_file.path, "ab+");
}

int log_write(const char * format, ...) {
    char string[256];
    va_list arg;

    /* get system time */
    time_t time_v;
    time(&time_v);
    struct tm *time_p = localtime(&time_v);

    va_start(arg, format);
    vsprintf(string, format, arg);
    printf("%s", string);

#if 1
    bool flag = false;

    if (log_file.date != time_p->tm_mday) {
        log_file.date = time_p->tm_mday;

        if (log_file.fp != NULL) {
            fclose(log_file.fp);
        }
        flag = true;
    } else {
        if (log_file.fp == NULL) {
            flag = true;
        }  
    }

    if (flag) {
        flag = false;
        sprintf(log_file.path, "/var/log/lora/%4d-%02d-%02d.log",
                time_p->tm_year + 1900, time_p->tm_mon + 1, time_p->tm_mday);

        log_file.fp = fopen(log_file.path, "ab+");
    }

    if (log_file.fp != NULL) {
        fprintf(log_file.fp, "[%02d:%02d:%02d] %s", 
               time_p->tm_hour, time_p->tm_min, time_p->tm_sec, 
               string);
        fflush(log_file.fp);
    }
#endif

    va_end(arg);
}

int log_buf(char *buf, uint16_t len) {
    char string[1024];
    char str_tmp[16];

    /* get system time */
    time_t time_v;
    time(&time_v);
    struct tm *time_p = localtime(&time_v);

    string[0] = '\0';
    for (uint16_t i = 0; i < len; i++) {
        sprintf(str_tmp, "%02x ", buf[i]);
        strcat(string, str_tmp);
    }
    strcat(string, "\r\n");

    printf("%s", string);

#if 1
    bool flag = false;

    if (log_file.date != time_p->tm_mday) {
        log_file.date = time_p->tm_mday;

        if (log_file.fp != NULL) {
            fclose(log_file.fp);
        }
        flag = true;
    } else {
        if (log_file.fp == NULL) {
            flag = true;
        }  
    }

    if (flag) {
        flag = false;
        sprintf(log_file.path, "/var/log/lora/%4d-%02d-%02d.log",
                time_p->tm_year + 1900, time_p->tm_mon + 1, time_p->tm_mday);

        log_file.fp = fopen(log_file.path, "ab+");
    }


    if (log_file.fp != NULL) {
        fprintf(log_file.fp, "[%02d:%02d:%02d] %s", 
               time_p->tm_hour, time_p->tm_min, time_p->tm_sec, 
               string);
        fflush(log_file.fp);
    }
#endif

}

#if 0
/* create log file */
void log_write(const char *format, ...)
{
    FILE *fp = NULL;
    char buf[256];
    /* get system time */
    time_t time_v;
    time(&time_v);
    struct tm *time_p = localtime(&time_v);

    sprintf(log_file.path, "/home/ftpuser/log/lora/%4d-%02d.log", time_p->tm_year + 1900,
           time_p->tm_mon + 1);

    sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d    %s \r\n", time_p->tm_year + 1900,
          time_p->tm_mon + 1, time_p->tm_mday, time_p->tm_hour, time_p->tm_min,
          time_p->tm_sec, str);

    fp = fopen(log_file.path, "ab+");
    if (NULL != fp) {
        /* write log file */
        fwrite(buf, strlen(buf), 1, fp);
        /* file will not saved before closed */
        // fflush(fp);
        fclose(fp);
    }
}
#endif
