/*************************************************************************
	> File Name: common.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: March 6th,2017 Monday 13:42:52
 ************************************************************************/

#ifndef __COMMON_H
#define __COMMON_H

#ifdef __LINUX
#define _FILE_OFFSET_BITS 64 //Make sure st_size is 64bits instead of 32bits
#endif

#include <stdio.h>
#include <stdlib.h>

/******************************Modifiable*******************************/
#define     BUF_SIZE                        64

//#include <stdint.h>
//typedef     char                                int8_t;
typedef     short                               int16_t;
typedef     int                                 int32_t;
typedef     unsigned char                       uint8_t;
typedef     unsigned short                      uint16_t;
typedef     unsigned int                        uint32_t;
#ifdef __LINUX
#ifdef __32BIT
typedef     long long                           int64_t;
typedef     unsigned long long                  uint64_t;
#elif defined __64BIT
typedef     long                                int64_t;
typedef     unsigned long                       uint64_t;
#endif
#elif defined __WINDOWS
typedef     long long                           int64_t;
typedef     unsigned long long                  uint64_t;
#endif

#include <sys/stat.h>

#ifdef __LINUX
#define     EXECUTE_DELETE                      "rm -rf %s >/dev/null 2>&1"
#define     GetFileInfo(name, info)             stat(name, info)
typedef struct stat                             stat_t;

#elif defined __WINDOWS
#define     EXECUTE_DELETE                      "del /a /f /q %s >nul 2>nul"
#define     GetFileInfo(name, info)             _stati64(name, info)
typedef struct _stati64                         stat_t;
#endif

extern FILE *g_LogFile;
extern char g_ErrBuf[BUF_SIZE];
extern time_t g_ti;
extern struct tm *g_time;

#define     DISP(format, args...) \
            fprintf(stdout, format, ##args)
            
#define     DISP_ERR_DEBUG(str) \
            snprintf(g_ErrBuf, sizeof(g_ErrBuf), "[%s][%d]: %s\n", __func__, __LINE__, str)
            
#define     DISP_ERR(format, args...) \
            snprintf(g_ErrBuf, sizeof(g_ErrBuf), format, ##args)
            
#define     DISP_ERR_PLUS(format, args...) \
            snprintf(g_ErrBuf, sizeof(g_ErrBuf), format, ##args)
            
//#undef      NULL
//#define     NULL                                ((void *)0)
/************************************************************************/

#define     __I                                 volatile const
#define     __O                                 volatile
#define     __IO                                volatile
#define     ALIGN_4K                            __attribute__((aligned(4)))

#ifdef __LINUX
#define     LOG_FILE_NAME                       "./log"
#elif defined __WINDOWS
#define     LOG_FILE_NAME                       ".\\log"
#endif

typedef enum {
    false = 0,
    true = !false,
}_bool_; //Name as _bool_ to differ bool and _bool defined in curses.h on WINDOWS platform

#ifndef _bool
#define _bool                                   _bool_
#endif

#ifndef bool
#define bool                                    _bool_
#endif


typedef enum {
    STAT_OK = 0,
    STAT_ERR,
    
    STAT_FATAL_ERR,
}ALIGN_4K G_STATUS;

G_STATUS SafeExecute(char *pFormat, ...);

#if 0
#if (defined __LINUX) || (defined __WINDOWS)
    #ifdef __REDIRECTION
        extern FILE *g_pDispFile;
        #define     DISP(format, args...) \
                    fprintf(g_pDispFile, format, ##args)
        #ifdef __DEBUG
        #define     DISP_ERR(str) \
                    fprintf(g_pDispFile, "[%s][%d]: %s\n", __func__, __LINE__, str)
        #else //__DEBUG
        #define     DISP_ERR(str) \
                    fprintf(g_pDispFile, "%s\n", str)
        #endif //__DEBUG
        #define     DISP_ERR_PLUS(format, args...) \
                    fprintf(g_pDispFile, format, ##args)
    #else //REDIRECTION
        #define     DISP(format, args...) \
                    fprintf(stdout, format, ##args)
        #ifdef __DEBUG
        #define     DISP_ERR(str) \
                    fprintf(stderr, "[%s][%d]: %s\n", __func__, __LINE__, str)
        #else //__DEBUG
        #define     DISP_ERR(str) \
                    fprintf(stderr, "%s\n", str)
        #endif //__DEBUG
        #define     DISP_ERR_PLUS(format, args...) \
                    fprintf(stderr, format, ##args)
    #endif //__REDIRECTION
#else //(defined __LINUX) || (defined __WINDOWS)
    #define     DISP(format, args...)           ((void)0)
    #define     DISP_ERR(str)                   ((void)0)
    #define     DISP_ERR_PLUS(format, args...)  ((void)0)
#endif //(defined __LINUX) || (defined __WINDOWS)

#ifdef __LINUX
    #include <time.h>
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

#endif
