/*************************************************************************
	> File Name: encryption.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年08月03日 星期四 08时37分45秒
 ************************************************************************/

#ifdef __LINUX
#define _FILE_OFFSET_BITS 64 //make sure st_size is 64bits instead of 32bits
#endif

#include "encryption.h"
#include "control.h"
#include "control_str.h"
#include <string.h>

static G_STATUS EncryptDecryptFile(char func);
static inline G_STATUS GetFileSize(const char *pFileName, int64_t *pFileSize);
static G_STATUS Encrypt_KB_File(const char *pFileName, int64_t FileSize);
static G_STATUS Encrypt_MB_File(const char *pFileName, int64_t FileSize);
static G_STATUS Encrypt_GB_File(const char *pFileName, int64_t FileSize);
static G_STATUS Decrypt_KB_File(const char *pFileName, int64_t FileSize);
static void ConvertFileFormat(char *pFileName);

char g_password[CYT_PASSWORD_LENGHT];
/*************************************************************************
                            Redirect DISP
 ************************************************************************/
static char g_buf[BUF_SIZE];
#define     DISP(format, args...) \
                snprintf(g_buf, sizeof(g_buf), format, ##args)
#define     DISP_ERR_PLUS(format, args...) \
                snprintf(g_buf, sizeof(g_buf), format, ##args)
#ifdef __DEBUG
#define     DISP_ERR(str) \
                snprintf(g_buf, sizeof(g_buf), "[%s][%d]: %s \n", __func__, __LINE__, str)    
#else //__DEBUG
#define     DISP_ERR(str) \
                snprintf(g_buf, sizeof(g_buf), "%s \n", str)
#endif //__DEBUG

G_STATUS encrypt(char func)
{
    G_STATUS status;

    switch(func)
    {
        case CTL_MENU_FUNC_ENCRYPT_FILE :                         
            status = EncryptDecryptFile(func);
            if(status != STAT_OK)
                return status;
            break;
        case CTL_MENU_FUNC_ENCRYPT_FOLDER :
            break;
        case CTL_MENU_FUNC_DECRYPT_FILE :
            status = EncryptDecryptFile(func);
            if(status != STAT_OK)
                return status;
            break;
            break;
        case CTL_MENU_FUNC_DECRYPT_FOLDER :
            break;
        default :
            break;
    }
    
    return STAT_OK;
}

static G_STATUS EncryptDecryptFile(char func)
{    
    G_STATUS status;
    char FileName[CYT_FILE_NAME_LENGTH];
    status = CTL_GetFileName(FileName);
    if(status != STAT_OK)
        return status; 

    status = CTL_GetPassord(g_password);
    if(status != STAT_OK)
        return status;
    
    int64_t FileSize = 0;
    status = GetFileSize(FileName, &FileSize);
    if(status != STAT_OK)
        return STAT_GO_BACK;

    WINDOW *win = newwin(LINES, COLS, 0, 0);
    if(CTL_MENU_FUNC_ENCRYPT_FILE == func)
        wprintw(win, "%s%s ", g_pStr[CTL_STR_IN_ENCRYPTING], FileName);
    else if(CTL_MENU_FUNC_DECRYPT_FILE == func)
        wprintw(win, "%s%s ", g_pStr[CTL_STR_IN_DECRYPTING], FileName);
    wrefresh(win);
    if(FileSize <= BUF_SIZE_SMALL)
    {
        if(CTL_MENU_FUNC_ENCRYPT_FILE == func)
            status = Encrypt_KB_File(FileName, FileSize);
        else if(CTL_MENU_FUNC_DECRYPT_FILE == func)
            status = Decrypt_KB_File(FileName, FileSize);
        if(status != STAT_OK)
        {
            wprintw(win, "%s \n", g_buf);
            wprintw(win, "%s", g_pStr[CTL_STR_ANY_KEY]);
            wgetch(win);
            delwin(win);
            touchwin(stdscr);
            refresh();
            return STAT_GO_BACK;
        }
    }
    wprintw(win, "%s \n", g_pStr[CTL_STR_SUCCESS]);
    wprintw(win, "%s", g_pStr[CTL_STR_ANY_KEY]);
    wgetch(win);
    delwin(win);
    touchwin(stdscr);
    refresh();
    return STAT_OK;
}

#ifdef __LINUX    
static inline G_STATUS GetFileSize(const char *pFileName, int64_t *pFileSize)
{
    struct stat FileInfo;
    int res = 0;

    res = stat(pFileName, &FileInfo);
   
    if(res != 0)
    {
        DISP_ERR(g_pStr[CTL_STR_ERR_GET_FILE_INFO_ERR]);
        return STAT_ERR;
    }

    *pFileSize = FileInfo.st_size;
    
    return STAT_OK;
}
#elif defined __WINDOWS
static inline G_STATUS GetFileSize(const char *pFileName, int64_t *pFileSize)
{
    struct _stati64 FileInfo;
    int res = 0;

    res = _stati64(pFileName, &FileInfo);
   
    if(res != 0)
    {
        DISP_ERR(g_pStr[CTL_STR_ERR_GET_FILE_INFO_ERR]);
        return STAT_ERR;
    }
    
    *pFileSize = FileInfo.st_size;
    
    return STAT_OK;
}
#endif

/*************************************************************************
                   Core codes of encryption algorithm
************************************************************************/

static G_STATUS Encrypt_KB_File(const char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_ERR(g_pStr[CTL_STR_ERR_FOPEN_ERR]);
        return STAT_ERR;
    }
    
    //read data from original file
    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * BUF_SIZE_SMALL);
    if(NULL == pData)
    {
        fclose(fp);
        DISP_ERR(g_pStr[CTL_STR_ERR_MALLOC_ERR]);
        return STAT_ERR;
    }
    
    memset(pData, 0, sizeof(uint8_t)*BUF_SIZE_SMALL);
    fseek(fp, 0, SEEK_SET);
    int64_t size = 0;
    size = fread(pData, sizeof(uint8_t), FileSize, fp);
    if(size != FileSize)
    {
        free(pData);
        fclose(fp);
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_READ_FILE]);        
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
        DISP_ERR(g_pStr[CTL_STR_ERR_PASSWORD_NULL]);
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
            DISP_ERR(g_pStr[CTL_STR_ERR_MALLOC_ERR]);
            return STAT_ERR;
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
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_CREATE_OPEN_FILE]);
        return STAT_ERR;
    }
    
    size = fwrite(pData, sizeof(uint8_t), FileSize, fp);
    if(size != FileSize)
    {        
        free(pData);
        fclose(fp);
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_WRITE_FILE]);
        return STAT_ERR;
    }

    free(pData);
    fclose(fp);
    fp = NULL;

#ifdef __LINUX
    snprintf(NewFileName, sizeof(NewFileName), "rm -f %s", pFileName);
#elif defined __WINDOWS    
    snprintf(NewFileName, sizeof(NewFileName), "del %s", pFileName);
    ConvertFileFormat(NewFileName);
#endif
    fp = popen(NewFileName, "r");
    if(NULL == fp)
    {
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_DELETE_OLD_FILE]);
        return STAT_ERR;
    }

    pclose(fp);    
    return STAT_OK;
}

static G_STATUS Encrypt_MB_File(const char *pFileName, int64_t FileSize)
{
    return STAT_OK;
}

static G_STATUS Encrypt_GB_File(const char *pFileName, int64_t FileSize)
{
    return STAT_OK;
}

static G_STATUS Decrypt_KB_File(const char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_ERR(g_pStr[CTL_STR_ERR_FOPEN_ERR]);
        return STAT_ERR;
    }
    
    //read data from original file
    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * BUF_SIZE_SMALL);
    if(NULL == pData)
    {
        fclose(fp);
        DISP_ERR(g_pStr[CTL_STR_ERR_MALLOC_ERR]);
        return STAT_ERR;
    }
    
    memset(pData, 0, sizeof(uint8_t)*BUF_SIZE_SMALL);
    fseek(fp, 0, SEEK_SET);
    int64_t size = 0;
    size = fread(pData, sizeof(uint8_t), FileSize, fp);
    if(size != FileSize)
    {
        free(pData);
        fclose(fp);
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_READ_FILE]);        
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
        DISP_ERR(g_pStr[CTL_STR_ERR_PASSWORD_NULL]);
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
            DISP_ERR(g_pStr[CTL_STR_ERR_MALLOC_ERR]);
            return STAT_ERR;
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
    snprintf(NewFileName, sizeof(NewFileName), "%s%s", pFileName, DECRYPT_FILE_SUFFIX_NAME);
    FILE *NewFp = fopen(NewFileName, "wb+");
    if(NULL == NewFp)
    {
        free(pData);
        fclose(fp);
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_CREATE_OPEN_FILE]);
        return STAT_ERR;
    }
    
    size = fwrite(pData, sizeof(uint8_t), FileSize, NewFp);
    if(size != FileSize)
    {        
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_ERR(g_pStr[CTL_STR_ERR_FAIL_TO_WRITE_FILE]);
        return STAT_ERR;
    }

    free(pData);
    fclose(fp);    
    fclose(NewFp);
    
    return STAT_OK;
}

static void ConvertFileFormat(char *pFileName)
{
    while(*pFileName != '\0')
    {
        if('/' == *pFileName)
            *pFileName = 92;    //'\' ASCII is 92 
        pFileName++;
    }
}
