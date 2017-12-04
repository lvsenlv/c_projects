/*************************************************************************
	> File Name: common.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: March 9th,2017 Thursday 14:47:06
 ************************************************************************/

#include "common.h"
#include <stdarg.h>

#ifndef __DISABLE_COMPILE_INFO
    #ifdef __LINUX
        #pragma message("Activate __LINUX")
    #elif defined __WINDOWS
        #pragma message("Activate __WINDOWS")
    #else
        #error "You must assign the platform as __LINUX or __WINDOWS"
    #endif
    
    #ifdef __32BIT
        #pragma message("Activate __32BIT")
    #elif defined __64BIT
        #pragma message("Activate __64BIT")
    #else
        #error "You must assign the platform as __32BIT or __64BIT"
    #endif //__LINUX
    
    #ifdef __DEBUG
        #pragma message("Activate __DEBUG")
    #endif //__DEBUG
    
    #ifdef __REDIRECTION
        #pragma message("Activate __REDIRECTION")
    #endif //__REDIRECTION
#endif //__DISABLE_COMPILE_INFO

/******************************Modifiable*******************************/
FILE *g_LogFile = NULL;
char g_ErrBuf[BUF_SIZE];
time_t g_ti;
struct tm *g_time;

void __attribute__((destructor)) AfterMain(void)
{
    if(NULL != g_LogFile)
    {
        fclose(g_LogFile);
    }
}

G_STATUS SafeExecute(char *pFormat, ...)
{
    va_list varg;
    char buf[256];
    
    va_start(varg, pFormat);
    vsnprintf(buf, sizeof(buf), pFormat, varg);
    va_end(varg);
    
    if('\0' == *buf)
        return STAT_ERR;

    FILE *fp = popen(buf, "r");
    if(NULL == fp)
        return STAT_ERR;

    pclose(fp);
    return STAT_OK;
}

/**********************************************************************/

#if 0
#if (defined __LINUX ) || (defined __WINDOWS)
    #ifdef __REDIRECTION
        FILE *g_pDispFile = NULL;
    #endif //__REDIRECTION

    #ifdef __LINUX
        struct timeval g_StartTime, g_StopTime;
    #endif //__LINUX
#endif //__LINUX || __WINDOWS

void __attribute__((constructor)) BeforeMain(void)
{
#ifdef __LINUX
    //START_COUNT;
#endif //__LINUX

#ifdef __REDIRECTION
    g_pDispFile = fopen(LOG_FILE_NAME, "w+");
    if(g_pDispFile)
        fclose(g_pDispFile);

    g_pDispFile = fopen(LOG_FILE_NAME, "a+");
    if(!g_pDispFile)
        g_pDispFile = stderr;

#endif //__REDIRECTION
}

void __attribute__((destructor)) AfterMain(void)
{
#ifdef __REDIRECTION
    if(g_pDispFile)
        fclose(g_pDispFile);
#endif //__REDIRECTION

#ifdef __LINUX
    //STOP_COUNT;
    //GET_TIME;
#endif //__LINUX
}

#endif //#if 0
