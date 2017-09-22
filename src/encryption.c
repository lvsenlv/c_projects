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
#include "core_code.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

static PROCESS_STATUS EncryptFile(FileList_t *pFileList);
static PROCESS_STATUS DecryptFile(FileList_t *pFileList);
static PROCESS_STATUS EncryptFolder(FileList_t *pFileList);
static PROCESS_STATUS DecryptFolder(FileList_t *pFileList);
static inline void InitFileListNode(FileList_t *pFileList);
static inline void FreeFileList(FileList_t *pHeadNode);

char g_password[CTL_PASSWORD_LENGHT_MAX];
int g_RatioFactor[4];
FileList_t g_FileList;      //use in single thread
FileList_t *g_pFileList;    //ues in multi threads



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
G_STATUS EncryptDecrypt(char func)
{
    if((func != CTL_MENU_ENCRYPT) && (func != CTL_MENU_DECRYPT))
    {
        DISP_ERR(STR_ERR_INVALID_FUNC);
        return STAT_ERR;
    }
    
    G_STATUS status;
    char FileName[CYT_FILE_NAME_LENGHT];
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
        if(lstat(FileName, &FileInfo) == 0)
#elif defined __WINDOWS
        if(_stati64(FileName, &FileInfo) == 0)
#endif
            break;

        status = CTL_MakeChoice("%s", STR_FAIL_TO_GET_FILE_FOLDER_INFO);
        if(STAT_RETRY == status)
            continue;
        else
           return status;
    }

    status = CTL_GetPassord(g_password);
    if(status != STAT_OK)
        return status;

    PROCESS_STATUS ProcessStatus = PROCESS_STATUS_SUCCESS;

    if(S_IFREG & FileInfo.st_mode)
    {
        g_FileList.FileName = FileName;
        g_FileList.FileNameLenght = strlen(g_FileList.FileName);
        g_FileList.FileSize = FileInfo.st_size;
        g_FileList.pNext = NULL;

        ProcessStatus = (CTL_MENU_ENCRYPT == func) ? 
            EncryptFile(&g_FileList) : DecryptFile(&g_FileList);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        g_pFileList = ScanDirectory(FileName);
        if(NULL == g_pFileList)
        {
            DISP_ERR_PLUS("%s: %s", STR_FAIL_TO_SCAN_DIRECTORY, FileName);
            return STAT_ERR;
        }
        
        ProcessStatus = (CTL_MENU_ENCRYPT == func) ? 
            EncryptFolder(&g_FileList) : DecryptFolder(&g_FileList);

        FreeFileList(g_pFileList);
    }

    WINDOW *win;
    if(ProcessStatus == PROCESS_STATUS_SUCCESS)
    {
        win = newwin(1, sizeof(STR_SUCCESS)-1, (LINES-1)/2, (COLS-sizeof(STR_SUCCESS)+1)/2);
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        waddstr(win, STR_SUCCESS);
        CTL_RESET_COLOR(win, CTL_PANEL_GREEN);
        wrefresh(win);
    }
    else
    {
        win = newwin(1, sizeof(STR_FAIL)-1, (LINES-1)/2, (COLS-sizeof(STR_FAIL)+1)/2);
        CTL_SET_COLOR(win, CTL_PANEL_RED);
        waddstr(win, STR_FAIL);
        CTL_RESET_COLOR(win, CTL_PANEL_RED);
        wrefresh(win);
    }
    
    wgetch(win);
    delwin(win);
    touchline(stdscr, (LINES-1)/2, 1);
    refresh();

    return STAT_OK;
}



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static PROCESS_STATUS EncryptFile(FileList_t *pFileList)
{
    int lines = 0, cols = 0;

    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght) <= CTL_ENCRYPT_FILE_WIN_COLS)
    {
        lines = 4;
        cols = pFileList->FileNameLenght + sizeof(STR_IN_ENCRYPTING) - 1;
    }
    else
    {
        lines = ((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)
            /CTL_ENCRYPT_FILE_WIN_COLS) + 4;
        cols = CTL_ENCRYPT_FILE_WIN_COLS;
    }

    CTL_HIDE_CONSOLE_END_LINE();
    
    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    mvwhline(win, 0, 0, '-', cols);
    mvwhline(win, lines-1, 0, '-', cols);
    CTL_RESET_COLOR(win, CTL_PANEL_CYAN);

    CTL_SET_COLOR(win, CTL_PANEL_MAGENTA);
    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)%cols != 0)
    {
        mvwprintw(win, 1, 0, "%s%s\n", STR_IN_ENCRYPTING, pFileList->FileName);
    }
    else
    {
        mvwprintw(win, 1, 0, "%s%s", STR_IN_ENCRYPTING, pFileList->FileName);
    }
    CTL_RESET_COLOR(win, CTL_PANEL_MAGENTA);

    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwaddstr(win, lines-2, 0, STR_RATE);
    wrefresh(win);

    g_RatioFactor[0] = 0;
    
    PthreadArg_t PthreadArg;
    InitPthreadArg(&PthreadArg);
    PthreadArg.pFunc = encrypt;
    PthreadArg.pRatioFactor = &g_RatioFactor[0];
    PthreadArg.pLock = &g_StatusLock[0];
    PthreadArg.pCurFileList = &g_FileList;
    PthreadArg.ProcessStatus = PROCESS_STATUS_BUSY;
    
    pthread_t PthreadID;
    int ret = 0;
    ret = pthread_create(&PthreadID, NULL, Pthread_ProcessFile, &PthreadArg);
	if(ret)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
		CTL_SHOW_CONSOLE_END_LINE();
        return PROCESS_STATUS_ELSE_ERR;
	}

    int denominator = (int)(pFileList->FileSize / CYT_SMALL_FILE_SIZE);
    if(0 == denominator)
        denominator = 100;
    
    while(PROCESS_STATUS_BUSY == PthreadArg.ProcessStatus)
    {
        mvwprintw(win, lines-2, (cols-3)/2, "%d%%", g_RatioFactor[0]/denominator);
        wrefresh(win);
        usleep(500*1000); //500ms
    }
    CTL_RESET_COLOR(win, CTL_PANEL_YELLOW);

    delwin(win);
    touchwin(stdscr);
    CTL_SHOW_CONSOLE_END_LINE();
    return PthreadArg.ProcessStatus;
}

static PROCESS_STATUS DecryptFile(FileList_t *pFileList)
{
    int lines = 0, cols = 0;

    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght) <= CTL_ENCRYPT_FILE_WIN_COLS)
    {
        lines = 4;
        cols = pFileList->FileNameLenght + sizeof(STR_IN_ENCRYPTING) - 1;
    }
    else
    {
        lines = ((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)
            /CTL_ENCRYPT_FILE_WIN_COLS) + 4;
        cols = CTL_ENCRYPT_FILE_WIN_COLS;
    }

    CTL_HIDE_CONSOLE_END_LINE();
    
    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    mvwhline(win, 0, 0, '-', cols);
    mvwhline(win, lines-1, 0, '-', cols);
    CTL_RESET_COLOR(win, CTL_PANEL_CYAN);

    CTL_SET_COLOR(win, CTL_PANEL_MAGENTA);
    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)%cols != 0)
    {
        mvwprintw(win, 1, 0, "%s%s\n", STR_IN_DECRYPTING, pFileList->FileName);
    }
    else
    {
        mvwprintw(win, 1, 0, "%s%s", STR_IN_DECRYPTING, pFileList->FileName);
    }
    CTL_RESET_COLOR(win, CTL_PANEL_MAGENTA);

    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwaddstr(win, lines-2, 0, STR_RATE);
    wrefresh(win);

    g_RatioFactor[0] = 0;
    
    PthreadArg_t PthreadArg;
    InitPthreadArg(&PthreadArg);
    PthreadArg.pFunc = decrypt;
    PthreadArg.pRatioFactor = &g_RatioFactor[0];
    PthreadArg.pLock = &g_StatusLock[0];
    PthreadArg.pCurFileList = &g_FileList;
    PthreadArg.ProcessStatus = PROCESS_STATUS_BUSY;
    
    pthread_t PthreadID;
    int ret = 0;
    ret = pthread_create(&PthreadID, NULL, Pthread_ProcessFile, &PthreadArg);
	if(ret)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
		CTL_SHOW_CONSOLE_END_LINE();
        return PROCESS_STATUS_ELSE_ERR;
	}

    int denominator = (int)(pFileList->FileSize / CYT_SMALL_FILE_SIZE);
    if(0 == denominator)
        denominator = 100;
    
    while(PROCESS_STATUS_BUSY == PthreadArg.ProcessStatus)
    {
        mvwprintw(win, lines-2, (cols-3)/2, "%d%%", g_RatioFactor[0]/denominator);
        wrefresh(win);
        usleep(500*1000); //500ms
    }
    CTL_RESET_COLOR(win, CTL_PANEL_YELLOW);

    delwin(win);
    touchwin(stdscr);
    CTL_SHOW_CONSOLE_END_LINE();
    return PthreadArg.ProcessStatus;
}

//Adapt 4 threads to process encrypting or decrypting
static PROCESS_STATUS EncryptFolder(FileList_t *pFileList)
{
    return STAT_OK;
}

static PROCESS_STATUS DecryptFolder(FileList_t *pFileList)
{
    return STAT_OK;
}



//file list related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FileList_t *ScanDirectory(char *pFolderName)
{
    DIR *pDir;
    pDir = opendir(pFolderName);
    if(NULL == pDir)
        return NULL;
    
    FileList_t *pHeadNode = (FileList_t *)malloc(sizeof(FileList_t));
    if(NULL == pHeadNode)
    {
        closedir(pDir);
        return NULL;
    }
    
    InitFileListNode(pHeadNode);
    
    FileList_t *pCurFileList = pHeadNode, *pNewFileList;
    struct dirent *pEntry;
    int FolderNameLenght = strlen(pFolderName);
    int FileNameLenght;
    char *pFileName;
    
    while(1)
    {
        pEntry = readdir(pDir);
        if(NULL == pEntry)
            break;

        if('.' == pEntry->d_name[0])
        {
            if('\0' == pEntry->d_name[1])
                continue;
            else if(('.' == pEntry->d_name[1]) && ('\0' == pEntry->d_name[2]))
                continue;
        }

        FileNameLenght = FolderNameLenght + strlen(pEntry->d_name) + 2; //contains '/' or '\' and '\0'
        pFileName = (char *)malloc(FileNameLenght);
        if(NULL == pFileName)
        {
            FreeFileList(pHeadNode);
            closedir(pDir);
            return NULL;
        }

        snprintf(pFileName, FileNameLenght, "%s/%s", pFolderName, pEntry->d_name);
        
        if(DT_DIR == pEntry->d_type)
        {
            pNewFileList = ScanDirectory(pFileName);
            if(NULL == pNewFileList)
            {
                free(pFileName);
                continue;
            }
            
            pHeadNode->FileSize += pNewFileList->FileSize;
            pCurFileList->pNext = pNewFileList->pNext;
            pCurFileList = pNewFileList;

            while(pCurFileList->pNext != NULL)
            {
                pCurFileList = pCurFileList->pNext;
            }
            
            if(pNewFileList->FileName != NULL)
                free(pNewFileList->FileName);
            free(pNewFileList);
        }
        else if(DT_REG == pEntry->d_type)
        {
            pNewFileList = (FileList_t *)malloc(sizeof(FileList_t));
            if(NULL == pNewFileList)
            {
                FreeFileList(pHeadNode);
                closedir(pDir);
                return NULL;
            }
            
            pNewFileList->FileName = pFileName;
            pNewFileList->FileNameLenght = FileNameLenght;
            pNewFileList->FileSize = 0;
            pNewFileList->pNext = NULL;
            
            pCurFileList->pNext = pNewFileList;
            pCurFileList = pNewFileList;

            pHeadNode->FileSize++;
        }
        else
        {
            free(pFileName);
            continue;
        }
        
    }

    if(NULL == pHeadNode->pNext)
    {
        FreeFileList(pHeadNode);
        pHeadNode = NULL;
    }

    closedir(pDir);
    return pHeadNode;
}

void DispFileList(FileList_t *pHeadNode)
{
    FileList_t *pFileList = pHeadNode->pNext;
    
    while(pFileList != NULL)
    {
        printf("%s\n", pFileList->FileName);
        pFileList = pFileList->pNext;
    }
}



//static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void InitFileListNode(FileList_t *pFileList)
{
    pFileList->FileName = NULL;
    pFileList->FileNameLenght = 0;
    pFileList->FileSize = 0;
    pFileList->pNext = NULL;
}

static inline void FreeFileList(FileList_t *pHeadNode)
{
    FileList_t *CurNode = pHeadNode;
    FileList_t *TmpNode;
    while(CurNode != NULL)
    {
        TmpNode = CurNode->pNext;
        if(CurNode->FileName != NULL)
            free(CurNode->FileName);
        free(CurNode);
        CurNode = TmpNode;
    }
}

