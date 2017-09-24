/*************************************************************************
	> File Name: encryption.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:37:45
 ************************************************************************/

#ifdef __LINUX
#define _FILE_OFFSET_BITS 64 //Make sure st_size is 64bits instead of 32bits
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

    PROCESS_STATUS ProcessStatus;

    if(S_IFREG & FileInfo.st_mode)
    {
        FileList_t FileList;
        
        FileList.FileName = FileName;
        FileList.FileNameLenght = strlen(FileList.FileName);
        FileList.FileSize = FileInfo.st_size;
        FileList.pNext = NULL;

        ProcessStatus = (CTL_MENU_ENCRYPT == func) ? 
            EncryptFile(&FileList) : DecryptFile(&FileList);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        FileList_t *pFileList = NULL;
        pFileList = ScanDirectory(FileName);
        if(NULL == pFileList)
        {
            DISP_ERR_PLUS("%s: %s", STR_FAIL_TO_SCAN_DIRECTORY, FileName);
            return STAT_ERR;
        }

        ProcessStatus = (CTL_MENU_ENCRYPT == func) ? 
            EncryptFolder(pFileList) : DecryptFolder(pFileList);

        FreeFileList(pFileList);
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
/*
    Return: PROCESS_STATUS_ERR, PROCESS_STATUS_FATAL_ERR, 
            PROCESS_STATUS_ELSE_ERR, PROCESS_STATUS_SUCCESS
*/
static PROCESS_STATUS EncryptFile(FileList_t *pFileList)
{
    if(STAT_OK != CheckFileListArg(pFileList))
    {
        DISP_ERR(STR_ERR_INVALID_FILE_LIST_ARG);
        return PROCESS_STATUS_ELSE_ERR;
    }
    
    //Create the window >>>
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

    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)%cols != 0)
    {
        mvwprintw(win, 1, 0, "%s%s\n", STR_IN_ENCRYPTING, pFileList->FileName);
    }
    else
    {
        mvwprintw(win, 1, 0, "%s%s", STR_IN_ENCRYPTING, pFileList->FileName);
    }
    CTL_RESET_COLOR(win, CTL_PANEL_CYAN);

    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwaddstr(win, lines-2, 0, STR_RATE);
    wrefresh(win);
    //Create the window <<<

    //Core code >>>
    int RatioFactor = 0;
    PthreadArg_t PthreadArg;
    InitPthreadArg(&PthreadArg);
    PthreadArg.pFunc = encrypt;
    PthreadArg.pRatioFactor = &RatioFactor;
    PthreadArg.pLock = &g_StatusLock[0];
    PthreadArg.pCurFileList = pFileList;
    PthreadArg.ProcessStatus = PROCESS_STATUS_BUSY;
    
    pthread_t PthreadID;
    int ret = 0;
    ret = pthread_create(&PthreadID, NULL, Pthread_ProcessFile, &PthreadArg);
	if(ret != 0)
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
        mvwprintw(win, lines-2, (cols-3)/2, "%d%%", RatioFactor/denominator);
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

}

//Adapt 4 threads to process encrypting or decrypting
static PROCESS_STATUS EncryptFolder(FileList_t *pFileList)
{
    //Create the window >>>
    WINDOW *win1 = newwin(LINES/4, COLS, 0, 0);
    WINDOW *win2 = newwin(LINES/4, COLS, LINES/4, 0);
    WINDOW *win3 = newwin(LINES/4, COLS, LINES/2, 0);
    WINDOW *win4 = newwin(LINES/4, COLS, LINES*3/4, 0);

    CTL_SET_COLOR(win1, CTL_PANEL_YELLOW);
    CTL_SET_COLOR(win2, CTL_PANEL_YELLOW);
    CTL_SET_COLOR(win3, CTL_PANEL_YELLOW);
    CTL_SET_COLOR(win4, CTL_PANEL_YELLOW);

    //wattron(win1, A_REVERSE);
    mvwaddstr(win1, 0, 0, STR_TASK1);
    mvwaddstr(win1, 0, COLS/4, STR_SUCCESS_COUNT);
    mvwaddstr(win1, 0, COLS/2, STR_FAIL_COUNT);
    mvwaddstr(win1, 0, COLS*3/4, STR_RATE);
    //wattroff(win1, A_REVERSE);
    
    //wattron(win2, A_REVERSE);
    mvwaddstr(win2, 0, 0, STR_TASK2);
    mvwaddstr(win2, 0, COLS/4, STR_SUCCESS_COUNT);
    mvwaddstr(win2, 0, COLS/2, STR_FAIL_COUNT);
    mvwaddstr(win2, 0, COLS*3/4, STR_RATE);
    //wattroff(win2, A_REVERSE);
    
    //wattron(win3, A_REVERSE);
    mvwaddstr(win3, 0, 0, STR_TASK3);
    mvwaddstr(win3, 0, COLS/4, STR_SUCCESS_COUNT);
    mvwaddstr(win3, 0, COLS/2, STR_FAIL_COUNT);
    mvwaddstr(win3, 0, COLS*3/4, STR_RATE);
    //wattroff(win3, A_REVERSE);
    
    //wattron(win4, A_REVERSE);
    mvwaddstr(win4, 0, 0, STR_TASK4);
    mvwaddstr(win4, 0, COLS/4, STR_SUCCESS_COUNT);
    mvwaddstr(win4, 0, COLS/2, STR_FAIL_COUNT);
    mvwaddstr(win4, 0, COLS*3/4, STR_RATE);
    //wattroff(win4, A_REVERSE);

    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
    wrefresh(win4);

    g_pCurFilelist = pFileList;
    int RatioFactor1 = 0, RatioFactor2 = 0, RatioFactor3 = 0, RatioFactor4 = 0;

    PthreadArg_t PthreadArg1, PthreadArg2, PthreadArg3, PthreadArg4;
    InitPthreadArg(&PthreadArg1);
    InitPthreadArg(&PthreadArg2);
    InitPthreadArg(&PthreadArg3);
    InitPthreadArg(&PthreadArg4);

    PthreadArg1.pFunc = decrypt;
    PthreadArg1.pRatioFactor = &RatioFactor1;
    PthreadArg1.pLock = &g_StatusLock[0];
    PthreadArg1.pCurFileList = g_pCurFilelist;
    PthreadArg1.ProcessStatus = PROCESS_STATUS_BUSY;
    PthreadArg1.SuccessCount = 0;
    PthreadArg1.FailCount = 0;
    PthreadArg1.RefreshFlag = 1;
    
    PthreadArg2.pFunc = decrypt;
    PthreadArg2.pRatioFactor = &RatioFactor2;
    PthreadArg2.pLock = &g_StatusLock[1];
    PthreadArg2.pCurFileList = g_pCurFilelist;
    PthreadArg2.ProcessStatus = PROCESS_STATUS_BUSY;
    PthreadArg2.SuccessCount = 0;
    PthreadArg2.FailCount = 0;
    PthreadArg2.RefreshFlag = 1;
    
    PthreadArg3.pFunc = decrypt;
    PthreadArg3.pRatioFactor = &RatioFactor3;
    PthreadArg3.pLock = &g_StatusLock[2];
    PthreadArg3.pCurFileList = g_pCurFilelist;
    PthreadArg3.ProcessStatus = PROCESS_STATUS_BUSY;
    PthreadArg3.SuccessCount = 0;
    PthreadArg3.FailCount = 0;
    PthreadArg3.RefreshFlag = 1;
    
    PthreadArg4.pFunc = decrypt;
    PthreadArg4.pRatioFactor = &RatioFactor4;
    PthreadArg4.pLock = &g_StatusLock[3];
    PthreadArg4.pCurFileList = g_pCurFilelist;
    PthreadArg4.ProcessStatus = PROCESS_STATUS_BUSY;
    PthreadArg4.SuccessCount = 0;
    PthreadArg4.FailCount = 0;
    PthreadArg4.RefreshFlag = 1;
    
    int ret = 0;
    pthread_t PthreadID1, PthreadID2, PthreadID3, PthreadID4;
    
    ret = pthread_create(&PthreadID1, NULL, Pthread_ProcessFolder, &PthreadArg1);
	if(ret != 0)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
		CTL_SHOW_CONSOLE_END_LINE();
        return PROCESS_STATUS_ELSE_ERR;
	}
	
	ret = pthread_create(&PthreadID2, NULL, Pthread_ProcessFolder, &PthreadArg2);
	if(ret != 0)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
		CTL_SHOW_CONSOLE_END_LINE();
        return PROCESS_STATUS_ELSE_ERR;
	}
	
	ret = pthread_create(&PthreadID3, NULL, Pthread_ProcessFolder, &PthreadArg3);
	if(ret != 0)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
		CTL_SHOW_CONSOLE_END_LINE();
        return PROCESS_STATUS_ELSE_ERR;
	}
	
	ret = pthread_create(&PthreadID4, NULL, Pthread_ProcessFolder, &PthreadArg4);
	if(ret != 0)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
		CTL_SHOW_CONSOLE_END_LINE();
        return PROCESS_STATUS_ELSE_ERR;
	}
	
	int denominator1 = 100, denominator2 = 100, denominator3 = 100, denominator4 = 100;
        
    while(1)
    {
        if((PROCESS_STATUS_BUSY != PthreadArg1.ProcessStatus) && 
           (PROCESS_STATUS_BUSY != PthreadArg2.ProcessStatus) && 
           (PROCESS_STATUS_BUSY != PthreadArg3.ProcessStatus) && 
           (PROCESS_STATUS_BUSY != PthreadArg4.ProcessStatus))
           break;

        mvwprintw(win1, 0, (COLS*3/4 + sizeof(STR_RATE)-1), 
            "%d%%", *PthreadArg1.pRatioFactor/denominator1);
        mvwprintw(win2, 0, (COLS*3/4 + sizeof(STR_RATE)-1), 
            "%d%%", *PthreadArg2.pRatioFactor/denominator2);
        mvwprintw(win3, 0, (COLS*3/4 + sizeof(STR_RATE)-1), 
            "%d%%", *PthreadArg3.pRatioFactor/denominator3);
        mvwprintw(win4, 0, (COLS*3/4 + sizeof(STR_RATE)-1), 
            "%d%%", *PthreadArg4.pRatioFactor/denominator4);

        if(1 == PthreadArg1.RefreshFlag)
        {
            PthreadArg1.RefreshFlag = 0;
            denominator1 = (int)(PthreadArg1.pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
            if(0 == denominator1)
                denominator1 = 100;
            
            mvwprintw(win1, 0, (COLS/4 + sizeof(STR_SUCCESS_COUNT)-1), 
                "%d", PthreadArg1.SuccessCount);
            mvwprintw(win1, 0, (COLS/2 + sizeof(STR_FAIL_COUNT)-1), 
                "%d", PthreadArg1.FailCount);
            mvwaddstr(win1, 1, 0, PthreadArg1.pCurFileList->FileName);
        }

        if(1 == PthreadArg2.RefreshFlag)
        {
            PthreadArg2.RefreshFlag = 0;
            denominator2 = (int)(PthreadArg2.pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
            if(0 == denominator2)
                denominator2 = 100;
                
            mvwprintw(win2, 0, (COLS/4 + sizeof(STR_SUCCESS_COUNT)-1), 
                "%d", PthreadArg2.SuccessCount);
            mvwprintw(win2, 0, (COLS/2 + sizeof(STR_FAIL_COUNT)-1), 
                "%d", PthreadArg2.FailCount);
            mvwaddstr(win2, 1, 0, PthreadArg2.pCurFileList->FileName);
        }

        if(1 == PthreadArg3.RefreshFlag)
        {
            PthreadArg3.RefreshFlag = 0;
            denominator3 = (int)(PthreadArg3.pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
            if(0 == denominator3)
                denominator3 = 100;
                
            mvwprintw(win3, 0, (COLS/4 + sizeof(STR_SUCCESS_COUNT)-1), 
                "%d", PthreadArg3.SuccessCount);
            mvwprintw(win3, 0, (COLS/2 + sizeof(STR_FAIL_COUNT)-1), 
                "%d", PthreadArg3.FailCount);
            mvwaddstr(win3, 1, 0, PthreadArg3.pCurFileList->FileName);
        }
        
        if(1 == PthreadArg4.RefreshFlag)
        {    
            PthreadArg4.RefreshFlag = 0;
            denominator4 = (int)(PthreadArg4.pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
            if(0 == denominator4)
                denominator4 = 100;
                
            mvwprintw(win4, 0, (COLS/4 + sizeof(STR_SUCCESS_COUNT)-1), 
                "%d", PthreadArg4.SuccessCount);
            mvwprintw(win4, 0, (COLS/2 + sizeof(STR_FAIL_COUNT)-1), 
                "%d", PthreadArg4.FailCount);
            mvwaddstr(win4, 1, 0, PthreadArg4.pCurFileList->FileName);
        }

        wrefresh(win1);
        wrefresh(win2);
        wrefresh(win3);
        wrefresh(win4);

        sleep(1);
    }
    
    CTL_RESET_COLOR(win1, CTL_PANEL_YELLOW);
    CTL_RESET_COLOR(win2, CTL_PANEL_YELLOW);
    CTL_RESET_COLOR(win3, CTL_PANEL_YELLOW);
    CTL_RESET_COLOR(win4, CTL_PANEL_YELLOW);
    
    return STAT_OK;
}

static PROCESS_STATUS DecryptFolder(FileList_t *pFileList)
{
    return STAT_OK;
}



//File list related
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
#ifdef __LINUX
    struct stat FileInfo;
#elif defined __WINDOWS
    struct _stati64 FileInfo;
#endif

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
        
#ifdef __LINUX
        if(0 != lstat(pFileName, &FileInfo))
#elif defined __WINDOWS
        if(0 != _stati64(pFileName, &FileInfo))
#endif
        {
            free(pFileName);
            continue;
        }
        
        if((DT_DIR == pEntry->d_type) && (FileInfo.st_mode & S_IFDIR))
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
        else if((DT_REG == pEntry->d_type) && (FileInfo.st_mode & S_IFREG))
        {
            pNewFileList = (FileList_t *)malloc(sizeof(FileList_t));
            if(NULL == pNewFileList)
            {
                free(pFileName);
                FreeFileList(pHeadNode);
                closedir(pDir);
                return NULL;
            }
            
            pNewFileList->FileName = pFileName;
            pNewFileList->FileNameLenght = FileNameLenght;
            pNewFileList->FileSize = FileInfo.st_size;
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



//Static inline functions
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

