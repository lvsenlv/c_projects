/*************************************************************************
	> File Name: common.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年03月09日 星期四 14时47分06秒
 ************************************************************************/

#include "common.h"

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
#else //__REDIRECTION
    #pragma message("Deactivate __REDIRECTION")
#endif //__REDIRECTION

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
    g_pDispFile = fopen("./log.txt", "w+");
    if(g_pDispFile)
        fclose(g_pDispFile);

    g_pDispFile = fopen("./log.txt", "a+");
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

