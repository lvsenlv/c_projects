/*************************************************************************
	> File Name: common.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年03月06日 星期一 13时42分52秒
 ************************************************************************/

#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>

/******************************modifiable*******************************/
#define     BUF_SIZE                        128
#define     CLEAR_IN_BUF                    {while(getchar() != '\n');}
#ifdef __LINUX
    #define CLEAR                           system("clear")
#elif defined __WINDOWS
    #define CLEAR                           system("cls")
#endif

//#include <stdint.h>
//typedef     char                            int8_t;
typedef     short                           int16_t;
typedef     int                             int32_t;
typedef     unsigned char                   uint8_t;
typedef     unsigned short                  uint16_t;
typedef     unsigned int                    uint32_t;
#ifdef __LINUX
    #ifdef __32BIT
        typedef     long long                       int64_t;
        typedef     unsigned long long              uint64_t;
    #elif defined __64BIT
        typedef     long                            int64_t;
        typedef     unsigned long                   uint64_t;
    #endif
#elif defined __WINDOWS
    typedef     long long                       int64_t;
    typedef     unsigned long long              uint64_t;
#endif

//#undef      NULL
//#define     NULL                            ((void *)0)
/************************************************************************/

#define     __I                             volatile const
#define     __O                             volatile
#define     __IO                            volatile

#define     ERR_MALLOC                      "Error in malloc"
#define     ERR_FATAL                       "Fatal error"

typedef enum {
    STAT_OK = 0,
    STAT_ERR,
    STAT_GO_BACK,
    STAT_RETRY,
    STAT_EXIT,          //exit project
}G_STATUS;

#if 0
#if (defined __LINUX) || (defined __WINDOWS)
    #ifdef __REDIRECTION
        extern FILE *g_pDispFile;
        #define     DISP(format, args...) \
                    fprintf(g_pDispFile, format, ##args)
        #ifdef __DEBUG
        #define     DISP_ERR(str) \
                    fprintf(g_pDispFile, "[%s][%d]: %s \n", __func__, __LINE__, str)
        #else //__DEBUG
        #define     DISP_ERR(str) \
                    fprintf(g_pDispFile, "%s \n", str)
        #endif //__DEBUG
        #define     DISP_ERR_PLUS(format, args...) \
                    fprintf(g_pDispFile, format, ##args)
    #else //REDIRECTION
        #define     DISP(format, args...) \
                    fprintf(stdout, format, ##args)
        #ifdef __DEBUG
        #define     DISP_ERR(str) \
                    fprintf(stderr, "[%s][%d]: %s \n", __func__, __LINE__, str)
        #else //__DEBUG
        #define     DISP_ERR(str) \
                    fprintf(stderr, "%s \n", str)
        #endif //__DEBUG
        #define     DISP_ERR_PLUS(format, args...) \
                    fprintf(stderr, format, ##args)
    #endif //__REDIRECTION
#else //(defined __LINUX) || (defined __WINDOWS)
    #define     DISP(format, args...)           ((void)0)
    #define     DISP_ERR(str)                   ((void)0)
    #define     DISP_ERR_PLUS(format, args...)  ((void)0)
#endif //(defined __LINUX) || (defined __WINDOWS)
#endif //#if 0

#ifdef __LINUX
    #include <sys/time.h>
    extern struct timeval g_StartTime, g_StopTime;
    #define     START_COUNT                     gettimeofday(&g_StartTime, NULL)
    #define     STOP_COUNT                      gettimeofday(&g_StopTime, NULL)
    #ifdef __REDIRECTION
        #define     GET_TIME \
                    fprintf(g_pDispFile, "projec cost about %ldus in total\n", \
                    (g_StopTime.tv_sec - g_StartTime.tv_sec) * 1000000 + \
                     g_StopTime.tv_usec - g_StartTime.tv_usec)
    #else //REDIRECTION
        #define     GET_TIME \
                    fprintf(stdout, "projec cost about %ldus in total\n", \
                    (g_StopTime.tv_sec - g_StartTime.tv_sec) * 1000000 + \
                     g_StopTime.tv_usec - g_StartTime.tv_usec)
    #endif //__REDIRECTION
#endif //__LINUX

#endif
