/*************************************************************************
	> File Name: core_code.h
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:25:33
 ************************************************************************/

#ifndef __CORE_CODE_H
#define __CORE_CODE_H

#include "encryption.h"

#define PTHREAD_WAIT_INTERVAL               (1*1000) //Unit: us

extern __IO FileList_t *g_pCurFilelist;

typedef struct PthreadArgStruct
{
    G_STATUS (*pFunc)(char *, int64_t, int *);
    int *pRatioFactor;
    __IO FileList_t *pCurFileList;
    __IO PROCESS_STATUS ProcessStatus;

    //use in multi threads
    __IO int SuccessCount;
    __IO int FailCount;
    __IO REFRESH_FLAG RefreshFlag;
}PthreadArg_t;

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

void *Pthread_EncryptDecrypt(void *arg);
G_STATUS encrypt(char *pFileName, int64_t FileSize, int *pRatioFactor);
G_STATUS decrypt(char *pFileName, int64_t FileSize, int *pRatioFactor);

#endif
