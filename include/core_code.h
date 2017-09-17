/*************************************************************************
	> File Name: core_code.h
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:25:33
 ************************************************************************/

#ifndef __CORE_CODE_H
#define __CORE_CODE_H

#include "encryption.h"
#include <pthread.h>

extern FileList_t g_FileList;
extern __IO FileList_t *g_pCurFilelist;
extern pthread_mutex_t g_StatusLock[4];

typedef struct PthreadArg
{
    G_STATUS (*pFunc)(char *, int64_t, int *);
    int *pRatioFactor;
    pthread_mutex_t *pLock;
    __IO FileList_t *pCurFileList;
    PROCESS_STATUS ProcessStatus;

    //use in multi threads
    int SuccessCount;
    int FailCount;
}PthreadArg_t;

static inline void InitPthreadArg(PthreadArg_t *pArg_t)
{
    pArg_t->pFunc = NULL;
    pArg_t->pRatioFactor = NULL;
    pArg_t->pLock = NULL;
    pArg_t->pCurFileList = NULL;
    pArg_t->ProcessStatus = PROCESS_STATUS_BUSY;

    pArg_t->SuccessCount = 0;
    pArg_t->FailCount = 0;
}

void *Pthread_ProcessFile(void *arg);
void *Pthread_ProcessFolder(void *arg);
G_STATUS encrypt(char *pFileName, int64_t FileSize, int *pRatioFactor);
G_STATUS decrypt(char *pFileName, int64_t FileSize, int *pRatioFactor);

#endif
