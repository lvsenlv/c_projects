/*************************************************************************
	> File Name: core_code.c
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:17:38
 ************************************************************************/

#include "core_code.h"
#include <string.h>
#include <unistd.h>

static G_STATUS CheckPthreadArg(PthreadArg_t *pArg_t);
static inline void DeleteEncyptSuffix(char *pFileName);

__IO FileList_t *g_pCurFilelist = NULL;
pthread_mutex_t g_FileLock = PTHREAD_MUTEX_INITIALIZER;



//Pthread
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//Note: g_pCurFilelist must be initialized before invoked following function
void *Pthread_EncryptDecrypt(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    G_STATUS status = STAT_OK;
    
#ifdef __DEBUG
    if(CheckPthreadArg(pArg_t) != STAT_OK)
    {
        DISP_LOG(STR_NULL, STR_ERR_INVALID_PTHREAD_ARG);
        pArg_t->ProcessStatus = PROCESS_STATUS_FATAL_ERR;
        return NULL;
    }
#endif

    while(1)
    {
        //File lock >>>
        pthread_mutex_lock(&g_FileLock);
        if(NULL == g_pCurFilelist)
        {
            pArg_t->ProcessStatus = (pArg_t->FailCount == 0) ? 
                PROCESS_STATUS_SUCCESS : PROCESS_STATUS_ERR;
            pthread_mutex_unlock(&g_FileLock);
            break;
        }
        
        pArg_t->pCurFileList = g_pCurFilelist;
        g_pCurFilelist = g_pCurFilelist->pNext;
        pArg_t->RefreshFlag = REFRESH_FLAG_TRUE;
        
        if(CheckFileListArg(pArg_t->pCurFileList) != STAT_OK)
        {
            DISP_LOG(STR_NULL, STR_ERR_INVALID_FILE_LIST_ARG);
            pArg_t->FailCount++;
            pthread_mutex_unlock(&g_FileLock);
            continue;
        }
        
        pthread_mutex_unlock(&g_FileLock);
        //File lock <<<
        
        //Wait for clear refresh flag <=> wait for dispalying curent file name
        while(REFRESH_FLAG_FALSE != pArg_t->RefreshFlag)
        {
            delay(PTHREAD_WAIT_INTERVAL);
        }
        
        status = (*pArg_t->pFunc)(pArg_t->pCurFileList->pFileName, pArg_t->pCurFileList->FileSize, 
            pArg_t->pRatioFactor);
        
        //File lock >>>
        pthread_mutex_lock(&g_FileLock);
        if(status != STAT_OK)
        {
            if(STAT_FATAL_ERR == status)
            {
                pArg_t->ProcessStatus = PROCESS_STATUS_FATAL_ERR;
                pthread_mutex_unlock(&g_FileLock);
                return NULL;
            }
            
            pArg_t->FailCount++;
            pArg_t->RefreshFlag = REFRESH_FLAG_FAIL;
        }
        else
        {
            pArg_t->SuccessCount++;
            pArg_t->RefreshFlag = REFRESH_FLAG_SUCCESS;
        }
        pthread_mutex_unlock(&g_FileLock);
        //File lock <<<
        
        //Wait for clear refresh flag <=> wait for dispalying process status
        while(REFRESH_FLAG_FALSE != pArg_t->RefreshFlag)
        {
            delay(PTHREAD_WAIT_INTERVAL);
        }
        
    }
    
    return NULL;
}



//Core codes of encryption algorithm
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
G_STATUS encrypt(char *pFileName, int64_t FileSize, int *pRatioFactor)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGHT];
    snprintf(NewFileName, sizeof(NewFileName), "%s%s", pFileName, ENCRYPT_FILE_SUFFIX_NAME);
    FILE *NewFp = NULL;
    NewFp = fopen(NewFileName, "wb+");
    if(NULL == NewFp)
    {
        fclose(fp);
        DISP_LOG(NewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        return STAT_ERR;
    }

    if(0 == FileSize)
    {
        fclose(fp);
        fclose(NewFp);
        goto GOTO_DELETE_FILE;
    }

    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * CYT_SMALL_FILE_SIZE);
    if(NULL == pData)
    {
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_FATAL_ERR;
    }
        
    int64_t size = 0;
    int EncyptFactor = 0;
    int PasswordLenght = 0;
    const char *pPassword = g_password;

    //Get encrypt factor
    while(*pPassword != '\0')
    {
        EncyptFactor += (uint32_t)*pPassword;
        pPassword++;
        PasswordLenght++;
    }
    if(0 == PasswordLenght)
    {        
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_PASSWORD_NULL);
        return STAT_ERR;
    }
    EncyptFactor %= 8;
    if(0 == EncyptFactor)
        EncyptFactor = 1;

    uint8_t SubFactor1 = 0xFF >> (8-EncyptFactor);
    uint8_t SubFactor2 = 8 - EncyptFactor;
    uint8_t *pTmp = NULL, *pTmp2 = NULL;
    uint8_t TmpData = 0;
    int i = 0, index = 0;
    int CycleIndex = (int)(FileSize / CYT_SMALL_FILE_SIZE);

    int RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_FATAL_ERR;
    }

    for(index = 0; index < CycleIndex; index++)
    {
        *pRatioFactor = index*100;
        
        //Read data from original file
        memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
        size = fread(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, fp);
        if(size != CYT_SMALL_FILE_SIZE)
        {
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);
            return STAT_ERR;
        }

        *pRatioFactor += 10;
        
        //Proccess 1
        pTmp = pData;
        for(i = 0; i < CYT_SMALL_FILE_SIZE; i++)
        {
            TmpData = 0;
            TmpData = *pTmp & SubFactor1;
            TmpData <<= SubFactor2;
            TmpData |= *pTmp >> ((uint8_t)EncyptFactor);
            *pTmp++ = TmpData;
        }

        *pRatioFactor += 50;

        //Proccess 2
        pTmp = pBackupData;
        pTmp2 = pData + CYT_SMALL_FILE_SIZE - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2--;
        }
        
        pTmp = pData + CYT_SMALL_FILE_SIZE - 1;
        pTmp2 = pData + RestDataCount - 1;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp-- = *pTmp2--;
        }

        pTmp = pData;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }
        
        *pRatioFactor += 10;

        //Proccess 3
        pPassword = g_password;
        pTmp = pData + CYT_SMALL_FILE_SIZE - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp--;
            pPassword++;
        }

        *pRatioFactor += 10;

        //Write encyption data to new file
        size = fwrite(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, NewFp);
        if(size != CYT_SMALL_FILE_SIZE)
        {        
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
            return STAT_ERR;
        }
    }

    //Encrypt rest data
    int RestDataSize = (int)(FileSize % CYT_SMALL_FILE_SIZE);
    
    //Read data from original file
    memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
    size = fread(pData, sizeof(uint8_t), RestDataSize, fp);
    if(size != RestDataSize)
    {
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);
        return STAT_ERR;
    }
    
    //Proccess 1
    pTmp = pData;
    for(i = 0; i < RestDataSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= SubFactor2;
        TmpData |= *pTmp >> ((uint8_t)EncyptFactor);
        *pTmp++ = TmpData;
    }

    //Proccess 2
    if(RestDataSize <= PasswordLenght)
    {
        pTmp = pData;
        pPassword = g_password;
        for(i = 0; i < RestDataSize; i++)
        {
            *pTmp ^= *pPassword;
            pPassword++;
            pTmp++;
        }
    }
    else
    {
        pTmp = pBackupData;
        pTmp2 = pData + RestDataSize - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2--;
        }

        RestDataCount = RestDataSize - PasswordLenght;
        pTmp = pData + RestDataSize - 1;
        pTmp2 = pData + RestDataCount - 1;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp-- = *pTmp2--;
        }

        pTmp = pData;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }
    }

    //Proccess 3
    if(RestDataSize > 256)
    {
        pPassword = g_password;
        pTmp = pData + RestDataSize - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp--;
            pPassword++;
        }
    }
    
    //Write encyption data to new file
    size = fwrite(pData, sizeof(uint8_t), RestDataSize, NewFp);
    if(size != RestDataSize)
    {        
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
        return STAT_ERR;
    }

    free(pData);
    free(pBackupData);
    fclose(fp);
    fclose(NewFp);
    fp = NULL;
    NewFp = NULL;

GOTO_DELETE_FILE:

#ifdef __LINUX
    snprintf(NewFileName, sizeof(NewFileName), "rm -rf %s >/dev/null 2>&1", pFileName);
#elif defined __WINDOWS
    ConvertNameFormat(pFileName);
    snprintf(NewFileName, sizeof(NewFileName), "del /a /f /q %s >nul 2>nul", pFileName);
#endif
    fp = popen(NewFileName, "r");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FAIL_TO_DELETE_OLD_FILE);
        return STAT_ERR;
    }

    pclose(fp);    
    return STAT_OK;    
}

/*
    Note: the pFileName can't be "." or ".." or "..." or ".ept" or "..ept" or "...ept".
        And the pFileName must contain symbol '.'
*/
G_STATUS decrypt(char *pFileName, int64_t FileSize, int *pRatioFactor)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGHT];
    snprintf(NewFileName, sizeof(NewFileName), "%s", pFileName);
    DeleteEncyptSuffix(NewFileName);
    
    FILE *NewFp = NULL;
    NewFp = fopen(NewFileName, "wb+");
    if(NULL == NewFp)
    {
        fclose(fp);
        DISP_LOG(NewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        return STAT_ERR;
    }

    if(0 == FileSize)
    {
        fclose(fp);
        fclose(NewFp);
        return STAT_OK;
    }

    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * CYT_SMALL_FILE_SIZE);
    if(NULL == pData)
    {
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_FATAL_ERR;
    }
        
    int64_t size = 0;
    int EncyptFactor = 0;
    int PasswordLenght = 0;
    const char *pPassword = g_password;

    //Get decrypt factor
    while(*pPassword != '\0')
    {
        EncyptFactor += (int)*pPassword;
        pPassword++;
        PasswordLenght++;
    }
    if(0 == PasswordLenght)
    {        
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_PASSWORD_NULL);
        return STAT_ERR;
    }
    EncyptFactor %= 8;
    if(0 == EncyptFactor)
        EncyptFactor = 1;

    uint8_t SubFactor1 = 0xFF >> EncyptFactor;
    uint8_t SubFactor2 = 8 - EncyptFactor;
    uint8_t *pTmp = NULL, *pTmp2 = NULL;
    uint8_t TmpData = 0;
    int i = 0, index = 0;
    int CycleIndex = (int)(FileSize / CYT_SMALL_FILE_SIZE);

    int RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_FATAL_ERR;
    }

    for(index = 0; index < CycleIndex; index++)
    {
        *pRatioFactor = index*100;
        
        //Read data from original file
        memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
        size = fread(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, fp);
        if(size != CYT_SMALL_FILE_SIZE)
        {
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);
            return STAT_ERR;
        }

        *pRatioFactor += 10;
        
        //Proccess 3
        pPassword = g_password + PasswordLenght - 1;
        pTmp = pData + CYT_SMALL_FILE_SIZE - PasswordLenght;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp++;
            pPassword--;
        }

        *pRatioFactor += 10;

        //Proccess 2
        pTmp = pBackupData;
        pTmp2 = pData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData;
        pTmp2 = pData + PasswordLenght;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + CYT_SMALL_FILE_SIZE - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp-- = *pTmp2++;
        }

        *pRatioFactor += 10;

        //Proccess 1
        pTmp = pData;
        for(i = 0; i < CYT_SMALL_FILE_SIZE; i++)
        {
            TmpData = 0;
            TmpData = *pTmp & SubFactor1;
            TmpData <<= EncyptFactor;
            TmpData |= *pTmp >> ((uint8_t)SubFactor2);
            *pTmp++ = TmpData;
        }

        *pRatioFactor += 50;

        //Write encyption data to new file
        size = fwrite(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, NewFp);
        if(size != CYT_SMALL_FILE_SIZE)
        {        
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
            return STAT_ERR;
        }
    }

    //Encrypt rest data
    int RestDataSize = (int)(FileSize % CYT_SMALL_FILE_SIZE);
    
    //Read data from original file
    memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
    size = fread(pData, sizeof(uint8_t), RestDataSize, fp);
    if(size != RestDataSize)
    {
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);
        return STAT_ERR;
    }
    
    //Proccess 3
    if(RestDataSize > 256)
    {
        pPassword = g_password + PasswordLenght - 1;
        pTmp = pData + RestDataSize - PasswordLenght;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp++;
            pPassword--;
        }
    }       

    //Proccess 2
    if(RestDataSize <= PasswordLenght)
    {
        pTmp = pData;
        pPassword = g_password;
        for(i = 0; i < RestDataSize; i++)
        {
            *pTmp ^= *pPassword;
            pPassword++;
            pTmp++;
        }
    }
    else
    {
        pTmp = pBackupData;
        pTmp2 = pData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        RestDataCount = RestDataSize - PasswordLenght;
        pTmp = pData;
        pTmp2 = pData + PasswordLenght;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + RestDataSize - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp-- = *pTmp2++;
        }
    }    

    //Proccess 1
    pTmp = pData;
    for(i = 0; i < RestDataSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= EncyptFactor;
        TmpData |= *pTmp >> ((uint8_t)SubFactor2);
        *pTmp++ = TmpData;
    }

    
    //Write encyption data to new file
    size = fwrite(pData, sizeof(uint8_t), RestDataSize, NewFp);
    if(size != RestDataSize)
    {        
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
        return STAT_ERR;
    }

    free(pData);
    free(pBackupData);
    fclose(fp);
    fclose(NewFp);
   
    return STAT_OK;    
}



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static G_STATUS CheckPthreadArg(PthreadArg_t *pArg_t)
{
#ifdef __DEBUG
    if(NULL == pArg_t)
        return STAT_ERR;
#endif
    
    if((NULL == pArg_t->pFunc) || (NULL == pArg_t->pRatioFactor) || 
       (NULL == pArg_t->pCurFileList))
        return STAT_ERR;

    if(pArg_t->ProcessStatus != PROCESS_STATUS_BUSY)
        pArg_t->ProcessStatus = PROCESS_STATUS_BUSY;
    if(pArg_t->SuccessCount != 0)
        pArg_t->SuccessCount = 0;
    if(pArg_t->FailCount != 0)
        pArg_t->FailCount = 0;
    if(pArg_t->RefreshFlag != REFRESH_FLAG_FALSE)
        pArg_t->RefreshFlag = REFRESH_FLAG_FALSE;

    return STAT_OK;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void DeleteEncyptSuffix(char *pFileName)
{
    int len = strlen(pFileName);
    char *ptr = pFileName + len - 1;

    while((*ptr != '.') && (len > 0))
    {
        ptr--;
        len--;
    }

    if(len != 0)
        *ptr = '\0';
}
