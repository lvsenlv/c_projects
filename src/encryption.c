/*************************************************************************
	> File Name: encryption.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:37:45
 ************************************************************************/

#ifdef __LINUX
#define _FILE_OFFSET_BITS 64 //make sure st_size is 64bits instead of 32bits
#endif

#include "encryption.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static G_STATUS EncryptFile(char *pFileName, int64_t FileSize);
static G_STATUS DecryptFile(char *pFileName, int64_t FileSize);
static G_STATUS EncryptSmallFile(const char *pFileName, int64_t FileSize);
static G_STATUS DecryptSmallFile(const char *pFileName, int64_t FileSize);
static G_STATUS EncryptLargeFile(const char *pFileName, int64_t FileSize);
static G_STATUS DecryptLargeFile(const char *pFileName, int64_t FileSize);
static inline void DeleteEncyptSuffix(char *pFileName);
static inline void ConvertFileFormat(char *pFileName);

char g_password[CTL_PASSWORD_LENGHT_MAX];

G_STATUS EncryptDecrypt(char func)
{
    G_STATUS status;
    char FileName[CYT_FILE_NAME_LENGTH];
#ifdef __LINUX
        struct stat FileInfo; 
#elif defined __WINDOWS
        struct _stati64 FileInfo;
#endif

    while(1)
    {
        status = CTL_GetFileName(FileName);
        if(status != STAT_OK)
            return status;

#ifdef __LINUX
        if(stat(FileName, &FileInfo) == 0)
#elif defined __WINDOWS
        if(_stati64(FileName, &FileInfo) == 0)
#endif
            break;

        status = CTL_MakeChoice("%s", STR_FAIL_TO_GET_FILE_INFO);
        if(STAT_RETRY == status)
            continue;
        else
           return status;
    }

    status = CTL_GetPassord(g_password);
    if(status != STAT_OK)
        return status;

    if(S_IFREG & FileInfo.st_mode)
    {
        if(CTL_MENU_ENCRYPT == func)
        {
            EncryptFile(FileName, FileInfo.st_size);
        }
        else if(CTL_MENU_DECRYPT == func)
        {
            DecryptFile(FileName, FileInfo.st_size);
        }
        
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        if(CTL_MENU_ENCRYPT == func)
        {
            //EncryptFolder();
        }
        else if(CTL_MENU_DECRYPT == func)
        {
            //DecryptFolder();
        }
    }

    return STAT_OK;
}

static G_STATUS EncryptFile(char *pFileName, int64_t FileSize)
{    
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    mvwprintw(win, 0, 0, "%s%s\n", STR_IN_ENCRYPTING, pFileName);
    //wprintw(win, "%s", STR_RATE);
    wrefresh(win);

    G_STATUS status;
    if(FileSize <= CYT_SMALL_FILE_SIZE)
    {
        status = EncryptSmallFile(pFileName, FileSize);
    }
    else
    {
        status = EncryptLargeFile(pFileName, FileSize);
    }

    if(STAT_OK == status)
        wprintw(win, "success");
    else
        wprintw(win, "failed");

    wgetch(win);

    delwin(win);
    touchwin(stdscr);
    refresh();
    return STAT_OK;
}

static G_STATUS DecryptFile(char *pFileName, int64_t FileSize)
{    
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    mvwprintw(win, 0, 0, "%s%s\n", STR_IN_DECRYPTING, pFileName);
    //wprintw(win, "%s", STR_RATE);
    wrefresh(win);

    G_STATUS status;
    if(FileSize <= CYT_SMALL_FILE_SIZE)
    {
        status = DecryptSmallFile(pFileName, FileSize);
    }
    {
        status = DecryptLargeFile(pFileName, FileSize);
    }

    if(STAT_OK == status)
        wprintw(win, "success");
    else
        wprintw(win, "failed");

    wgetch(win);

    delwin(win);
    touchwin(stdscr);
    refresh();
    return STAT_OK;
}


/*************************************************************************
                   Core codes of encryption algorithm
************************************************************************/

static G_STATUS EncryptSmallFile(const char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    
    //read data from original file
    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * CYT_SMALL_FILE_SIZE);
    if(NULL == pData)
    {
        fclose(fp);
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }
    
    memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
    fseek(fp, 0, SEEK_SET);
    int64_t size = 0;
    size = fread(pData, sizeof(uint8_t), FileSize, fp);
    if(size != FileSize)
    {
        free(pData);
        fclose(fp);
        DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);        
        return STAT_ERR;
    }

    fclose(fp);
    fp = NULL;

    //get encrypt factor
    uint32_t EncyptFactor = 0;
    uint32_t PasswordLenght = 0;
    const char *pPassword = g_password;
    while(*pPassword != '\0')
    {
        EncyptFactor += (uint32_t)*pPassword;
        pPassword++;
        PasswordLenght++;
    }
    if(0 == PasswordLenght)
    {
        free(pData);
        DISP_LOG(pFileName, STR_PASSWORD_NULL);
        return STAT_ERR;
    }
    EncyptFactor %= 8;
    if(0 == EncyptFactor)
        EncyptFactor = 1;

    uint8_t SubFactor1 = 0xFF >> (8-EncyptFactor);
    uint8_t SubFactor2 = 8 - EncyptFactor;
    uint8_t *pTmp, *pTmp2;
    uint8_t TmpData;
    int32_t i = 0;

    //proccess 1
    pTmp = pData;
    for(i = 0; i < FileSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= SubFactor2;
        TmpData |= *pTmp >> ((uint8_t)EncyptFactor);
        *pTmp++ = TmpData;
    }    

    //proccess 2
    if(FileSize <= PasswordLenght)
    {
        pTmp = pData;
        pPassword = g_password;
        for(i = 0; i < FileSize; i++)
        {
            *pTmp ^= *pPassword;
            pPassword++;
            pTmp++;
        }
    }
    else
    {
        uint8_t *pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
        if(NULL == pBackupData)
        {
            free(pData);
            fclose(fp);
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_EXIT;
        }
        
        pTmp = pBackupData;
        pTmp2 = pData + FileSize - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2--;
        }

        uint32_t RestDataCount = FileSize - PasswordLenght;
        pTmp = pData + FileSize - 1;
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

        free(pBackupData);
    }

    //proccess 3
    if(FileSize > 256)
    {
        pPassword = g_password;
        pTmp = pData + FileSize - 1;
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
    char NewFileName[CYT_FILE_NAME_LENGTH];
    snprintf(NewFileName, sizeof(NewFileName), "%s%s", pFileName, ENCRYPT_FILE_SUFFIX_NAME);
    fp = fopen(NewFileName, "wb+");
    if(NULL == fp)
    {
        free(pData);
        DISP_LOG(NewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        return STAT_ERR;
    }
    
    size = fwrite(pData, sizeof(uint8_t), FileSize, fp);
    if(size != FileSize)
    {        
        free(pData);
        fclose(fp);
        DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
        return STAT_ERR;
    }

    free(pData);
    fclose(fp);
    fp = NULL;

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

static G_STATUS DecryptSmallFile(const char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    
    //read data from original file
    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * CYT_SMALL_FILE_SIZE);
    if(NULL == pData)
    {
        fclose(fp);
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }
    
    memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
    fseek(fp, 0, SEEK_SET);
    int64_t size = 0;
    size = fread(pData, sizeof(uint8_t), FileSize, fp);
    if(size != FileSize)
    {
        free(pData);
        fclose(fp);
        DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);
        return STAT_ERR;
    }

    //get encrypt factor
    uint32_t EncyptFactor = 0;
    uint32_t PasswordLenght = 0;
    const char *pPassword = g_password;
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
        DISP_LOG(pFileName, STR_PASSWORD_NULL);
        return STAT_ERR;
    }
    EncyptFactor %= 8;
    if(0 == EncyptFactor)
        EncyptFactor = 1;

    uint8_t SubFactor1 = 0xFF >> EncyptFactor;
    uint8_t SubFactor2 = 8 - EncyptFactor;
    uint8_t *pTmp, *pTmp2;
    uint8_t TmpData;
    int32_t i = 0;

    //proccess 3
    if(FileSize > 256)
    {
        pPassword = g_password + PasswordLenght - 1;
        pTmp = pData + FileSize - PasswordLenght;
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
    if(FileSize <= PasswordLenght)
    {
        pTmp = pData;
        pPassword = g_password;
        for(i = 0; i < FileSize; i++)
        {
            *pTmp ^= *pPassword;
            pPassword++;
            pTmp++;
        }
    }
    else
    {
        uint8_t *pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
        if(NULL == pBackupData)
        {
            free(pData);
            fclose(fp);
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_EXIT;
        }
        
        pTmp = pBackupData;
        pTmp2 = pData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        uint32_t RestDataCount = FileSize - PasswordLenght;
        pTmp = pData;
        pTmp2 = pData + PasswordLenght;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + FileSize - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp-- = *pTmp2++;
        }

        free(pBackupData);
    }    

    //proccess 1
    pTmp = pData;
    for(i = 0; i < FileSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= EncyptFactor;
        TmpData |= *pTmp >> ((uint8_t)SubFactor2);
        *pTmp++ = TmpData;
    }     

    //write encyption data to new file
    char NewFileName[CYT_FILE_NAME_LENGTH];
    snprintf(NewFileName, sizeof(NewFileName), "%s", pFileName);
    DeleteEncyptSuffix(NewFileName);
    FILE *NewFp = fopen(NewFileName, "wb+");
    if(NULL == NewFp)
    {
        free(pData);
        fclose(fp);
        DISP_LOG(pFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        return STAT_ERR;
    }
    
    size = fwrite(pData, sizeof(uint8_t), FileSize, NewFp);
    if(size != FileSize)
    {        
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_FAIL_TO_WRITE_FILE);
        return STAT_ERR;
    }

    free(pData);
    fclose(fp);    
    fclose(NewFp);
    
    return STAT_OK;
}

static G_STATUS EncryptLargeFile(const char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGTH];
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
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }
        
    int64_t size = 0;
    int32_t EncyptFactor = 0;
    int32_t PasswordLenght = 0;
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
    int32_t i = 0, index = 0;
    int32_t CycleIndex = (int32_t)(FileSize / CYT_SMALL_FILE_SIZE);

    int32_t RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }    
    
    for(index = 0; index < CycleIndex; index++)
    {
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
    int32_t RestDataSize = (int32_t)(FileSize % CYT_SMALL_FILE_SIZE);
    
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
    if(size != CYT_SMALL_FILE_SIZE)
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

static G_STATUS DecryptLargeFile(const char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGTH];
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
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }
        
    int64_t size = 0;
    int32_t EncyptFactor = 0;
    int32_t PasswordLenght = 0;
    const char *pPassword = g_password;

    //get decrypt factor
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
    int32_t i = 0, index = 0;
    int32_t CycleIndex = (int32_t)(FileSize / CYT_SMALL_FILE_SIZE);

    int32_t RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }    
    
    for(index = 0; index < CycleIndex; index++)
    {
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
    int32_t RestDataSize = (int32_t)(FileSize % CYT_SMALL_FILE_SIZE);
    
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
