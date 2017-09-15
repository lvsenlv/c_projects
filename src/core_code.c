/*************************************************************************
	> File Name: core_code.c
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:17:38
 ************************************************************************/

#include "core_code.h"
#include <string.h>

static G_STATUS encrypt(char *pFileName, int64_t FileSize, int *pRatioFactor);
static G_STATUS decrypt(char *pFileName, int64_t FileSize, int *pRatioFactor);
static inline void DeleteEncyptSuffix(char *pFileName);
static inline void ConvertFileFormat(char *pFileName);

pthread_mutex_t g_FileLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_StatusLock[4] = {
    PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 
    PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
};

//Pthread
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void *Pthread_EncryptFile(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    G_STATUS status;
    status = encrypt(pArg_t->pFileList->FileName, pArg_t->pFileList->FileSize, 
        pArg_t->pRatioFactor);

    pthread_mutex_lock(pArg_t->pLock);
    if(status != STAT_OK)
    {
        pArg_t->ProcessStatus = (STAT_FATAL_ERR == status) ? 
            PROCESS_STATUS_FATAL_ERR : PROCESS_STATUS_ERR;
    }
    else
    {
        pArg_t->ProcessStatus = PROCESS_STATUS_SUCCESS;
    }
    pthread_mutex_unlock(pArg_t->pLock);
    
    return NULL;
}

void *Pthread_DecryptFile(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    G_STATUS status;

    status = decrypt(pArg_t->pFileList->FileName, pArg_t->pFileList->FileSize, 
        pArg_t->pRatioFactor);
    pthread_mutex_lock(pArg_t->pLock);
    if(status != STAT_OK)
    {
        pArg_t->ProcessStatus = (STAT_FATAL_ERR == status) ? 
            PROCESS_STATUS_FATAL_ERR : PROCESS_STATUS_ERR;
    }
    else
    {
        pArg_t->ProcessStatus = PROCESS_STATUS_SUCCESS;
    }
    pthread_mutex_unlock(pArg_t->pLock);
    
    return NULL;
}

void *Pthread_EncryptFolder(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    FileList_t *CurFile;
    G_STATUS status = STAT_OK;

    pthread_mutex_lock(&g_FileLock);
    CurFile = pArg_t->pCurFileList;
    pArg_t->pCurFileList = pArg_t->pCurFileList->pNext;
    pthread_mutex_unlock(&g_FileLock);

    while(CurFile != NULL);
    {
        
        pthread_mutex_lock(pArg_t->pLock);
        pArg_t->pCurFileName = CurFile->FileName;
        pthread_mutex_unlock(pArg_t->pLock);
        
        status = encrypt(CurFile->FileName, CurFile->FileSize, pArg_t->pRatioFactor);
        pthread_mutex_lock(pArg_t->pLock);
        if(status != STAT_OK)
        {
            if(STAT_FATAL_ERR == status)
            {
                *pArg_t->pRatioFactor = PROCESS_STATUS_FATAL_ERR;
                pthread_mutex_unlock(pArg_t->pLock);
                return NULL;
            }
            
            *pArg_t->pRatioFactor = PROCESS_STATUS_ERR;
            
        }
        else
        {
            *pArg_t->pRatioFactor = PROCESS_STATUS_SUCCESS;
        }
        pthread_mutex_unlock(pArg_t->pLock);
        
        pthread_mutex_lock(&g_FileLock);
        CurFile = pArg_t->pCurFileList;
        pArg_t->pCurFileList = pArg_t->pCurFileList->pNext;
        pthread_mutex_unlock(&g_FileLock);
    }
    
    return NULL;
}

void *Pthread_DecryptFolder(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    FileList_t *CurFile;
    G_STATUS status = STAT_OK;

    pthread_mutex_lock(&g_FileLock);
    CurFile = pArg_t->pCurFileList;
    pArg_t->pCurFileList = pArg_t->pCurFileList->pNext;
    pthread_mutex_unlock(&g_FileLock);

    while(CurFile != NULL);
    {
        
        pthread_mutex_lock(pArg_t->pLock);
        pArg_t->pCurFileName = CurFile->FileName;
        pthread_mutex_unlock(pArg_t->pLock);
        
        status = decrypt(CurFile->FileName, CurFile->FileSize, pArg_t->pRatioFactor);
        pthread_mutex_lock(pArg_t->pLock);
        if(status != STAT_OK)
        {
            if(STAT_FATAL_ERR == status)
            {
                *pArg_t->pRatioFactor = PROCESS_STATUS_FATAL_ERR;
                pthread_mutex_unlock(pArg_t->pLock);
                return NULL;
            }
            
            *pArg_t->pRatioFactor = PROCESS_STATUS_ERR;            
            
        }
        else
        {
            *pArg_t->pRatioFactor = PROCESS_STATUS_SUCCESS;
        }
        pthread_mutex_unlock(pArg_t->pLock);
        
        pthread_mutex_lock(&g_FileLock);
        CurFile = pArg_t->pCurFileList;
        pArg_t->pCurFileList = pArg_t->pCurFileList->pNext;
        pthread_mutex_unlock(&g_FileLock);
    }
    
    return NULL;

}



//Core codes of encryption algorithm
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static G_STATUS encrypt(char *pFileName, int64_t FileSize, int *pRatioFactor)
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

    //get encrypt factor
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
        
        //read data from original file
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
        
        //proccess 1
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

        //proccess 2
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

        //proccess 3
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

        //write encyption data to new file
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

    //encrypt rest data
    int RestDataSize = (int)(FileSize % CYT_SMALL_FILE_SIZE);
    
    //read data from original file
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
    
    //proccess 1
    pTmp = pData;
    for(i = 0; i < RestDataSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= SubFactor2;
        TmpData |= *pTmp >> ((uint8_t)EncyptFactor);
        *pTmp++ = TmpData;
    }

    //proccess 2
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

    //proccess 3
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
    
    //write encyption data to new file
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

#ifdef __LINUX
    snprintf(NewFileName, sizeof(NewFileName), "rm -rf %s", pFileName);
#elif defined __WINDOWS
    ConvertFileFormat(pFileName);
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

static G_STATUS decrypt(char *pFileName, int64_t FileSize, int *pRatioFactor)
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

    //get decrypt factor
    while(*pPassword != '\0')
    {
        EncyptFactor += (uint)*pPassword;
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
        
        //read data from original file
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
        
        //proccess 3
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

        //proccess 2
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

        //proccess 1
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

        //write encyption data to new file
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

    //encrypt rest data
    int RestDataSize = (int)(FileSize % CYT_SMALL_FILE_SIZE);
    
    //read data from original file
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
    
    //proccess 3
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

    //proccess 2
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

    //proccess 1
    pTmp = pData;
    for(i = 0; i < RestDataSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= EncyptFactor;
        TmpData |= *pTmp >> ((uint8_t)SubFactor2);
        *pTmp++ = TmpData;
    }

    
    //write encyption data to new file
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



//static inline functions
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

static inline void ConvertFileFormat(char *pFileName)
{
    while(*pFileName != '\0')
    {
        if('/' == *pFileName)
            *pFileName = 92;    //'\' ASCII is 92 
        pFileName++;
    }
}

