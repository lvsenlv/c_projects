/*************************************************************************
	> File Name: core_code.h
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:25:33
 ************************************************************************/

#ifndef __CORE_CODE_H
#define __CORE_CODE_H

#include "encryption.h"
//#include <stdarg.h> //It has been included in curses.h

#ifdef __WINDOWS
#include <windows.h>
#endif

#define BASE_FILE_SIZE                          ((int64_t)(1024*1024*100)) //100Mb
#define PTHREAD_WAIT_INTERVAL                   1 //Unit: ms, wait for parent process to refresh window

#ifdef __LINUX
#define delay(time)                             usleep(time*1000) //Unit: ms
#elif defined __WINDOWS
#define delay(time)                             Sleep(time) //Unit: ms
#endif

//Format should contain '\n'
#define     DISP_LOG(format, args...) \
            { \
                pthread_mutex_lock(&g_LogLock); \
                g_ti = time(NULL); \
                g_time = localtime(&g_ti); \
                fprintf(g_LogFile, "[%4d-%02d-%02d %02d:%02d:%02d]: ", \
                    g_time->tm_year+1900, g_time->tm_mon, g_time->tm_mday, \
                    g_time->tm_hour, g_time->tm_min, g_time->tm_sec); \
                fprintf(g_LogFile, format, ##args); \
                pthread_mutex_unlock(&g_LogLock); \
            }

typedef struct PthreadArgStruct
{
    G_STATUS (*pFunc)(char *, int, int64_t, int *);
    int *pRatioFactor;
    __IO FileList_t *pCurFileList;
    __IO PROCESS_STATUS ProcessStatus;

    //use in multi threads
    __IO int SuccessCount;
    __IO int FailCount;
    __IO REFRESH_FLAG RefreshFlag;
}PthreadArg_t;

extern __IO FileList_t *g_pCurFilelist;

void *Pthread_EncryptDecrypt(void *arg);
G_STATUS encrypt(char *pFileName, int FileNameLength, int64_t FileSize, int *pRatioFactor);
G_STATUS decrypt(char *pFileName, int FileNameLength, int64_t FileSize, int *pRatioFactor);



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Initialize the member of pArg_t
 *  @Return: None
 *  @Note:   pArg_t can't be NULL
 */
static inline void InitPthreadArg(PthreadArg_t *pArg_t)
{
    pArg_t->pFunc = NULL;
    pArg_t->pRatioFactor = NULL;
    pArg_t->pCurFileList = NULL;
    pArg_t->ProcessStatus = PROCESS_STATUS_BUSY;

    pArg_t->SuccessCount = 0;
    pArg_t->FailCount = 0;
    pArg_t->RefreshFlag = REFRESH_FLAG_FALSE;
}

#endif
