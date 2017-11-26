/*************************************************************************
	> File Name: core_code.c
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:17:38
 ************************************************************************/

#include "core_code.h"
#include <unistd.h>

static G_STATUS CheckPthreadArg(PthreadArg_t *pArg_t);
static inline void DeleteEncyptSuffix(char *pFileName);
static inline G_STATUS RemoveFile(const char *pFileName);

__IO FileList_t *g_pCurFilelist = NULL;
pthread_mutex_t g_FileLock = PTHREAD_MUTEX_INITIALIZER;



//Pthread
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: It's a thread for encrypting or decrypting
 *  @Return: Aways NULL
 *  @Note:   g_pCurFilelist must be initialized before invoked following function
 */
void *Pthread_EncryptDecrypt(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    G_STATUS status = STAT_OK;
    
#ifdef __DEBUG
    if(STAT_OK != CheckPthreadArg(pArg_t))
    {
        DISP_LOG("%s\n", STR_INVALID_PARAMTER);
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
        
        if(STAT_OK != CheckFileListArg(pArg_t->pCurFileList))
        {
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
        
        status = (*pArg_t->pFunc)(pArg_t->pCurFileList->pFileName, 
            pArg_t->pCurFileList->FileNameLength , pArg_t->pCurFileList->FileSize, 
            pArg_t->pRatioFactor);
        
        //File lock >>>
        pthread_mutex_lock(&g_FileLock);
        if(STAT_OK != status)
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



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Core codes of encryption algorithm
 *  @Return: STAT_ERR / STAT_FATAL_ERR / STAT_OK
 *  @Note:   
 */
G_STATUS encrypt(char *pFileName, int FileNameLength, int64_t FileSize, int *pRatioFactor)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_OPEN_FILE);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char *pNewFileName = (char *)malloc(FileNameLength+sizeof(ENCRYPT_FILE_SUFFIX_NAME));
    if(NULL == pNewFileName)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_ERR_FAIL_TO_MALLOC);
        fclose(fp);
        return STAT_FATAL_ERR;
    }
    
    snprintf(pNewFileName, FileNameLength+sizeof(ENCRYPT_FILE_SUFFIX_NAME), 
        "%s%s", pFileName, ENCRYPT_FILE_SUFFIX_NAME);
    FILE *NewFp = NULL;
    NewFp = fopen(pNewFileName, "wb+");
    if(NULL == NewFp)
    {
        DISP_LOG("%s: %s\n", pNewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        free(pNewFileName); //Must free after DISP_LOG
        fclose(fp);
        return STAT_ERR;
    }

    if(0 == FileSize)
    {
        free(pNewFileName);
        fclose(fp);
        fclose(NewFp);
        return RemoveFile(pFileName);
    }

    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * BASE_FILE_SIZE);
    if(NULL == pData)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_ERR_FAIL_TO_MALLOC);
        unlink(pNewFileName);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_FATAL_ERR;
    }
        
    int64_t size = 0;
    int EncyptFactor = 0;
    int PasswordLength = 0;
    const char *pPassword = g_password;

    //Get encrypt factor
    while('\0' != *pPassword)
    {
        EncyptFactor += (uint32_t)*pPassword;
        pPassword++;
        PasswordLength++;
    }
    if(0 == PasswordLength)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_PASSWORD_NULL);
        unlink(pNewFileName);
        free(pData);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
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
    int CycleIndex = (int)(FileSize / BASE_FILE_SIZE);

    int RestDataCount = BASE_FILE_SIZE - PasswordLength;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLength * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_ERR_FAIL_TO_MALLOC);
        unlink(pNewFileName);
        free(pData);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_FATAL_ERR;
    }

    for(index = 0; index < CycleIndex; index++)
    {
        *pRatioFactor = index*100;
        
        //Read data from original file
        memset(pData, 0, sizeof(uint8_t)*BASE_FILE_SIZE);
        size = fread(pData, sizeof(uint8_t), BASE_FILE_SIZE, fp);
        if(BASE_FILE_SIZE != size)
        {
            DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_READ_FILE);
            unlink(pNewFileName);
            free(pData);
            free(pBackupData);
            free(pNewFileName); //Must free after unlink
            fclose(fp);
            fclose(NewFp);
            return STAT_ERR;
        }

        *pRatioFactor += 10;
        
        //Proccess 1
        pTmp = pData;
        for(i = 0; i < BASE_FILE_SIZE; i++)
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
        pTmp2 = pData + BASE_FILE_SIZE - 1;
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp++ = *pTmp2--;
        }
        
        pTmp = pData + BASE_FILE_SIZE - 1;
        pTmp2 = pData + RestDataCount - 1;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp-- = *pTmp2--;
        }

        pTmp = pData;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp++ = *pTmp2++;
        }
        
        *pRatioFactor += 10;

        //Proccess 3
        pPassword = g_password;
        pTmp = pData + BASE_FILE_SIZE - 1;
        for(i = 0; i < PasswordLength; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp--;
            pPassword++;
        }

        *pRatioFactor += 10;

        //Write encyption data to new file
        size = fwrite(pData, sizeof(uint8_t), BASE_FILE_SIZE, NewFp);
        if(BASE_FILE_SIZE != size)
        {
            DISP_LOG("%s: %s\n", pNewFileName, STR_FAIL_TO_WRITE_FILE);
            unlink(pNewFileName);
            free(pData);
            free(pBackupData);
            free(pNewFileName); //Must free after DISP_LOG and unlink
            fclose(fp);
            fclose(NewFp);
            return STAT_ERR;
        }
    }

    //Encrypt rest data
    int RestDataSize = (int)(FileSize % BASE_FILE_SIZE);
    
    //Read data from original file
    memset(pData, 0, sizeof(uint8_t)*BASE_FILE_SIZE);
    size = fread(pData, sizeof(uint8_t), RestDataSize, fp);
    if(size != RestDataSize)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_READ_FILE);
        unlink(pNewFileName);
        free(pData);
        free(pBackupData);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
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
    if(RestDataSize <= PasswordLength)
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
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp++ = *pTmp2--;
        }

        RestDataCount = RestDataSize - PasswordLength;
        pTmp = pData + RestDataSize - 1;
        pTmp2 = pData + RestDataCount - 1;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp-- = *pTmp2--;
        }

        pTmp = pData;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp++ = *pTmp2++;
        }
    }

    //Proccess 3
    if(256 < RestDataSize)
    {
        pPassword = g_password;
        pTmp = pData + RestDataSize - 1;
        for(i = 0; i < PasswordLength; i++)
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
        DISP_LOG("%s: %s\n", pNewFileName, STR_FAIL_TO_WRITE_FILE);
        unlink(pNewFileName);
        free(pData);
        free(pBackupData);
        free(pNewFileName); //Must free after DISP_LOG and unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_ERR;
    }

    free(pData);
    free(pBackupData);
    fclose(fp);
    fclose(NewFp);

    if(STAT_OK != RemoveFile(pFileName))
    {
        unlink(pNewFileName);
        free(pNewFileName);  //Must free after unlink
        return STAT_ERR;
    }

    free(pNewFileName);  //Must free after unlink
    return STAT_OK;
}

/*
 *  @Briefs: Core codes of encryption algorithm
 *  @Return: STAT_ERR / STAT_FATAL_ERR / STAT_OK
 *  @Note:   1. The pFileName can't be :
                "."                             -> after DeleteEncryptSuffix is '\0'
                ".."                            -> after DeleteEncryptSuffix is "."
                "..."                           -> after DeleteEncryptSuffix is ".."
                "ENCRYPT_FILE_SUFFIX_NAME"      -> after DeleteEncryptSuffix is '\0'
                ".ENCRYPT_FILE_SUFFIX_NAME"     -> after DeleteEncryptSuffix is "."
                "..ENCRYPT_FILE_SUFFIX_NAME"    -> after DeleteEncryptSuffix is ".."
             2. The pFileName must contain symbol '.'
 */
G_STATUS decrypt(char *pFileName, int FileNameLength, int64_t FileSize, int *pRatioFactor)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_OPEN_FILE);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char *pNewFileName = (char *)malloc(FileNameLength);
    if(NULL == pNewFileName)
    {
        fclose(fp);
        DISP_LOG("%s: %s\n", pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_FATAL_ERR;
    }
    
    snprintf(pNewFileName, FileNameLength, "%s", pFileName);
    DeleteEncyptSuffix(pNewFileName);
    
    FILE *NewFp = NULL;
    NewFp = fopen(pNewFileName, "wb+");
    if(NULL == NewFp)
    {
        DISP_LOG("%s: %s\n", pNewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        free(pNewFileName); //Must free after DISP_LOG
        fclose(fp);
        return STAT_ERR;
    }

    if(0 == FileSize)
    {
        free(pNewFileName);
        fclose(fp);
        fclose(NewFp);
        return RemoveFile(pFileName);
    }

    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * BASE_FILE_SIZE);
    if(NULL == pData)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_ERR_FAIL_TO_MALLOC);
        unlink(pNewFileName);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_FATAL_ERR;
    }
        
    int64_t size = 0;
    int EncyptFactor = 0;
    int PasswordLength = 0;
    const char *pPassword = g_password;

    //Get decrypt factor
    while('\0' != *pPassword)
    {
        EncyptFactor += (int)*pPassword;
        pPassword++;
        PasswordLength++;
    }
    if(0 == PasswordLength)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_PASSWORD_NULL);
        unlink(pNewFileName);
        free(pData);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
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
    int CycleIndex = (int)(FileSize / BASE_FILE_SIZE);

    int RestDataCount = BASE_FILE_SIZE - PasswordLength;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLength * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_ERR_FAIL_TO_MALLOC);
        unlink(pNewFileName);
        free(pData);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_FATAL_ERR;
    }

    for(index = 0; index < CycleIndex; index++)
    {
        *pRatioFactor = index*100;
        
        //Read data from original file
        memset(pData, 0, sizeof(uint8_t)*BASE_FILE_SIZE);
        size = fread(pData, sizeof(uint8_t), BASE_FILE_SIZE, fp);
        if(size != BASE_FILE_SIZE)
        {
            DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_READ_FILE);
            unlink(pNewFileName);
            free(pData);
            free(pBackupData);
            free(pNewFileName); //Must free after unlink
            fclose(fp);
            fclose(NewFp);
            return STAT_ERR;
        }

        *pRatioFactor += 10;
        
        //Proccess 3
        pPassword = g_password + PasswordLength - 1;
        pTmp = pData + BASE_FILE_SIZE - PasswordLength;
        for(i = 0; i < PasswordLength; i++)
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
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData;
        pTmp2 = pData + PasswordLength;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + BASE_FILE_SIZE - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp-- = *pTmp2++;
        }

        *pRatioFactor += 10;

        //Proccess 1
        pTmp = pData;
        for(i = 0; i < BASE_FILE_SIZE; i++)
        {
            TmpData = 0;
            TmpData = *pTmp & SubFactor1;
            TmpData <<= EncyptFactor;
            TmpData |= *pTmp >> ((uint8_t)SubFactor2);
            *pTmp++ = TmpData;
        }

        *pRatioFactor += 50;

        //Write encyption data to new file
        size = fwrite(pData, sizeof(uint8_t), BASE_FILE_SIZE, NewFp);
        if(BASE_FILE_SIZE != size)
        {
            DISP_LOG("%s: %s\n", pNewFileName, STR_FAIL_TO_WRITE_FILE);
            unlink(pNewFileName);
            free(pData);
            free(pBackupData);
            free(pNewFileName); //Must free after DISP_LOG and unlink
            fclose(fp);
            fclose(NewFp);
            return STAT_ERR;
        }
    }

    //Encrypt rest data
    int RestDataSize = (int)(FileSize % BASE_FILE_SIZE);
    
    //Read data from original file
    memset(pData, 0, sizeof(uint8_t)*BASE_FILE_SIZE);
    size = fread(pData, sizeof(uint8_t), RestDataSize, fp);
    if(size != RestDataSize)
    {
        DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_READ_FILE);
        unlink(pNewFileName);
        free(pData);
        free(pBackupData);
        free(pNewFileName); //Must free after unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_ERR;
    }
    
    //Proccess 3
    if(256 < RestDataSize)
    {
        pPassword = g_password + PasswordLength - 1;
        pTmp = pData + RestDataSize - PasswordLength;
        for(i = 0; i < PasswordLength; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp++;
            pPassword--;
        }
    }       

    //Proccess 2
    if(RestDataSize <= PasswordLength)
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
        for(i = 0; i < PasswordLength; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        RestDataCount = RestDataSize - PasswordLength;
        pTmp = pData;
        pTmp2 = pData + PasswordLength;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + RestDataSize - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLength; i++)
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
        DISP_LOG("%s: %s\n", pNewFileName, STR_FAIL_TO_WRITE_FILE);
        unlink(pNewFileName);
        free(pData);
        free(pBackupData);
        free(pNewFileName); //Must free after DISP_LOG and unlink
        fclose(fp);
        fclose(NewFp);
        return STAT_ERR;
    }

    free(pData);
    free(pBackupData);
    fclose(fp);
    fclose(NewFp);
    
    if(STAT_OK != RemoveFile(pFileName))
    {
        unlink(pNewFileName);
        free(pNewFileName);  //Must free after unlink
        return STAT_ERR;
    }

    free(pNewFileName);  //Must free after unlink
    return STAT_OK;
}



//Static function
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Check the member of pArg_t
 *  @Return: STAT_ERR / STAT_OK
 *  @Note:   None
 */
static G_STATUS CheckPthreadArg(PthreadArg_t *pArg_t)
{
#ifdef __DEBUG
    if(NULL == pArg_t)
        return STAT_ERR;
#endif
    
    if((NULL == pArg_t->pFunc) || (NULL == pArg_t->pRatioFactor) || 
       (NULL == pArg_t->pCurFileList))
        return STAT_ERR;

    if(PROCESS_STATUS_BUSY != pArg_t->ProcessStatus)
        pArg_t->ProcessStatus = PROCESS_STATUS_BUSY;
    if(0 != pArg_t->SuccessCount)
        pArg_t->SuccessCount = 0;
    if(0 != pArg_t->FailCount)
        pArg_t->FailCount = 0;
    if(REFRESH_FLAG_FALSE != pArg_t->RefreshFlag)
        pArg_t->RefreshFlag = REFRESH_FLAG_FALSE;

    return STAT_OK;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Delete the last suffix in pFileName
 *  @Return: None
 *  @Note:   None
 */
static inline void DeleteEncyptSuffix(char *pFileName)
{
    int len = strlen(pFileName);
    char *ptr = pFileName + len - 1;

    while(('.' != *ptr) && (0 < len))
    {
        ptr--;
        len--;
    }

    if(0 != len)
        *ptr = '\0';
}

/*
 *  @Briefs: Delete the pFileName
 *  @Return: STAT_ERR / STAT_OK
 *  @Note:   None
 */
static inline G_STATUS RemoveFile(const char *pFileName)
{
    if(0 != unlink(pFileName))
    {
        DISP_LOG("%s: %s\n", pFileName, STR_FAIL_TO_DELETE_OLD_FILE);
        return STAT_ERR;
    }

    return STAT_OK;
}
