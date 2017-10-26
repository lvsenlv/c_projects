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

static PROCESS_STATUS EncryptDecrypt(FileList_t *pFileList, CTL_MENU func);
static PROCESS_STATUS EncryptDecrypt_Plus(FileList_t *pFileList, CTL_MENU func);
static G_STATUS BeforeEncryptDecrypt(void);
static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus);
static FileList_t *ScanDirectory(char *pFolderName);
static FileList_t *ScanEncryptFile(char *pFolderName);
static inline void DeleteTailSymbol(char *pFileName, int FileNameLenght);
static inline void InitFileListNode(FileList_t *pFileList);
static inline void FreeFileList(FileList_t *pHeadNode);
static inline int IsEncryptFile(const char *pFileName, int FileNameLenght);

char g_password[CTL_PASSWORD_LENGHT_MAX];
pthread_mutex_t g_LogLock = PTHREAD_MUTEX_INITIALIZER;
static int g_SuccessFailCountTable[2] = {0, 0};



/*
 *  @Briefs: Get file name and password, invoke EncryptDecrypt at the end
 *  @Return: STAT_BACK
 *  @Note:   None
 */
G_STATUS CTL_EncryptDecrypt(CTL_MENU func)
{
#ifdef __DEBUG    
    if((CTL_MENU_ENCRYPT != func) && (CTL_MENU_DECRYPT != func))
    {
        CTL_DispWarning(STR_INVALID_FUNC);
        return STAT_BACK;
    }
#endif
    
#ifdef __LINUX
    struct stat FileInfo;
#elif defined __WINDOWS
    struct _stati64 FileInfo;
#endif
    char FileName[CTL_FILE_NAME_LENGHT];
    int FileNameLenght;
    G_STATUS status;
    
    status = CTL_GetFileName(FileName);
    if(STAT_OK != status)
        return status;

#ifdef __WINDOWS
    ConvertNameFormat(FileName);
#endif        
    FileNameLenght = strlen(FileName);
    DeleteTailSymbol(FileName, FileNameLenght);

#ifdef __LINUX
    if(0 != lstat(FileName, &FileInfo))
#elif defined __WINDOWS
    if(0 != _stati64(FileName, &FileInfo))
#endif
    {
        CTL_DispWarning("%s: %s\n", STR_FAIL_TO_GET_FILE_FOLDER_INFO, FileName);
        return STAT_BACK;
    }
    
    status = CTL_GetPassord(g_password);
    if(STAT_OK != status)
        return status;

    PROCESS_STATUS ProcessStatus = PROCESS_STATUS_ERR;

    if(S_IFREG & FileInfo.st_mode)
    {
        status = BeforeEncryptDecrypt();
        if(STAT_OK != status)
            return STAT_BACK;
    
        FileList_t HeadNode, FileList;
        HeadNode.pFileName = NULL;
        HeadNode.FileNameLenght = 0;
        HeadNode.FileSize = 1;
        HeadNode.pNext = &FileList;
        FileList.pFileName = FileName;
        FileList.FileNameLenght = FileNameLenght;
        FileList.FileSize = FileInfo.st_size;
        FileList.pNext = NULL;

        ProcessStatus = EncryptDecrypt_Plus(&HeadNode, func);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        FileList_t *pFileList = NULL;
        
        pFileList = (CTL_MENU_ENCRYPT == func) ? 
            ScanDirectory(FileName) : ScanEncryptFile(FileName);
            
        if(NULL == pFileList)
        {
            CTL_DispWarning("%s: %s", STR_FAIL_TO_SCAN_DIRECTORY, FileName);
            return STAT_BACK;
        }
        
        status = BeforeEncryptDecrypt();
        if(STAT_OK != status)
        {
            FreeFileList(pFileList);
            return STAT_BACK;
        }

        status = CTL_ConfirmOperation("%s%s\n%s: %d, %s\n", 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING, FileName, 
            STR_TOTAL, pFileList->FileSize, STR_IF_CONTINUE);
        
        if(STAT_OK != status)
        {
            FreeFileList(pFileList);
            return status;
        }
        
        ProcessStatus = EncryptDecrypt(pFileList, func);

        FreeFileList(pFileList);
    }

    status = AfterEncryptDecrypt(ProcessStatus);

    return status;
}



//Static function
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Create task to encrypt or decrypt
 *  @Return: PROCESS_STATUS_ELSE_ERR, PROCESS_STATUS_ERR, 
 *           PROCESS_STATUS_FATAL_ERR, PROCESS_STATUS_SUCCESS
 *  @Note:   pFileList must be the HeadNode
 */
static PROCESS_STATUS EncryptDecrypt(FileList_t *pFileList, CTL_MENU func)
{
#ifdef PTHREAD_NUM_MAX
#if (PTHREAD_NUM_MAX < 2)
    #error STR_EN_ERR_PTHREAD_NUM_TOO_SMALL
#elif (PTHREAD_NUM_MAX > 8)
    #error STR_EN_ERR_PTHREAD_NUM_TOO_BIG
#endif
#endif

#ifdef __DEBUG
    if(NULL == pFileList)
    {
        DISP_ERR(STR_INVALID_PARAMTER);
        return PROCESS_STATUS_ELSE_ERR;
    }
#endif

    FILE *fp = fopen(FILE_LIST_LOG_NAME, "wb");
    if(NULL == fp)
    {
        DISP_ERR(STR_FAIL_TO_OPEN_LOG_FILE);
        return PROCESS_STATUS_ELSE_ERR;
    }

    //Create the window >>>
    WINDOW *ScheduleWin[PTHREAD_NUM_MAX];
    WINDOW *win[PTHREAD_NUM_MAX];
    int i;

    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        ScheduleWin[i] = newwin(1, COLS, i*LINES/PTHREAD_NUM_MAX, 0);
    }
    
    for(i = 0; i < PTHREAD_NUM_MAX-1; i++) //PTHREAD_NUM_MAX-1 : the last win should tackle additionally
    {
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
            i*LINES/PTHREAD_NUM_MAX + 1 : schedule wins occupy the position i*LINES/PTHREAD_NUM_MAX, 
                so the win should start from to next line.
        */
        win[i] = newwin((LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX, COLS, 
            i*LINES/PTHREAD_NUM_MAX + 1, 0);
    }

    /*
        It must be under considered the situation that the result of LINES/PTHREAD_NUM_MAX 
            is decimal fraction.
        Thus, use "LINES-PTHREAD_NUM_MAX - (LINES-PTHREAD_NUM_MAX)*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX"
            instead of "(LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX"
    */
    win[PTHREAD_NUM_MAX-1] = newwin(LINES-PTHREAD_NUM_MAX - 
        (LINES-PTHREAD_NUM_MAX)*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX, 
        COLS, LINES*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX + 1, 0);
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        CTL_SET_COLOR(ScheduleWin[i], CTL_PANEL_CYAN);
        CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
        
        wmove(win[i], 0, 0);
        scrollok(win[i], true);
        
        wattron(ScheduleWin[i], A_REVERSE);
        mvwhline(ScheduleWin[i], 0, 0, ' ', COLS);

        mvwprintw(ScheduleWin[i], 0, 0, "%s%d->%s", STR_TASK, i+1, 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING);
        mvwprintw(ScheduleWin[i], 0, COLS*2/5, "%s0", STR_SUCCESS_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*3/5, "%s0", STR_FAIL_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*4/5, "%s0%%", STR_RATE);
    
        wrefresh(ScheduleWin[i]);
        wrefresh(win[i]);
    }

    g_pCurFilelist = pFileList->pNext; //First FileList is head node
    
    int RatioFactor[PTHREAD_NUM_MAX];
    int denominator[PTHREAD_NUM_MAX];
    PthreadArg_t PthreadArg[PTHREAD_NUM_MAX];
    pthread_t PthreadID[PTHREAD_NUM_MAX];
    int ret = 0;
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        RatioFactor[i] = 0;
        denominator[i] = 100;
        InitPthreadArg(&PthreadArg[i]);
        
        PthreadArg[i].pFunc = (CTL_MENU_ENCRYPT == func) ? encrypt : decrypt;
        PthreadArg[i].pRatioFactor = &RatioFactor[i];
        PthreadArg[i].pCurFileList = g_pCurFilelist;
        PthreadArg[i].ProcessStatus = PROCESS_STATUS_BUSY;
        PthreadArg[i].SuccessCount = 0;
        PthreadArg[i].FailCount = 0;
        PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE;
    }
     
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {   
        ret = pthread_create(&PthreadID[i], NULL, Pthread_EncryptDecrypt, &PthreadArg[i]);
	    if(ret != 0)
	    {
	        fclose(fp);
		    DISP_ERR(STR_FAIL_TO_CREATE_PTHREAD);
            return PROCESS_STATUS_ELSE_ERR;
	    }
    }
    
    char ExitFlag = 0;
    char flag = 0; //Set 1 if it has been refreshed <=> if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
    int StrSuccessWidth = GetWidth(STR_SUCCESS_COUNT);
    int StrFailWidth = GetWidth(STR_FAIL_COUNT);
    int StrRateWidth = GetWidth(STR_RATE);
	
	while(1)
    {
        if(ExitFlag)
            break;
        
        ExitFlag = 1;
        
        for(i = 0; i < PTHREAD_NUM_MAX; i++)
        {
            if(PROCESS_STATUS_BUSY == PthreadArg[i].ProcessStatus)
            {
                ExitFlag = 0;
                mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + StrRateWidth), 
                    "%d%%  ", RatioFactor[i]/denominator[i]);
                
                if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
                {
                    flag = 1;
                    switch(PthreadArg[i].RefreshFlag)
                    {
                        case REFRESH_FLAG_SUCCESS:
                            if(CTL_MENU_DECRYPT == func)
                            {
                                fprintf(fp, "%s\n", PthreadArg[i].pCurFileList->pFileName);
                            }
                            mvwprintw(ScheduleWin[i], 0, (COLS*2/5 + StrSuccessWidth), 
                                "%d", PthreadArg[i].SuccessCount);
                            mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + StrRateWidth), "100%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_GREEN);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_SUCCESS);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        case REFRESH_FLAG_FAIL:
                            mvwprintw(ScheduleWin[i], 0, (COLS*3/5 + StrFailWidth), 
                                "%d", PthreadArg[i].FailCount);
                            CTL_SET_COLOR(win[i], CTL_PANEL_RED);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_FAIL);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        default:
                            RatioFactor[i] = 0;
                            denominator[i] = (int)(PthreadArg[i].pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
                            if(0 == denominator[i])
                                denominator[i] = 100;
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
                            break;
                    }
                    
                    wrefresh(win[i]);
                    PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE; //Clear refresh flag, continue en/decrypting
                }
            
                wrefresh(ScheduleWin[i]);
            }
        }

        if(flag)
        {
            flag = 0;
            continue;
        }
        
        delay(REFRESH_INTERVAL);
	}
	
	fclose(fp);
        
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        delwin(ScheduleWin[i]);
        delwin(win[i]);
    }
    
    touchwin(stdscr);
    refresh();

    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        g_SuccessFailCountTable[0] += PthreadArg[i].SuccessCount;
        g_SuccessFailCountTable[1] += PthreadArg[i].FailCount;
    }
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        if(PROCESS_STATUS_SUCCESS != PthreadArg[i].ProcessStatus)
        {
            if(PROCESS_STATUS_FATAL_ERR == PthreadArg[i].ProcessStatus)
                return PROCESS_STATUS_FATAL_ERR;
                
            return PROCESS_STATUS_ERR;
        }
    }
        
    return PROCESS_STATUS_SUCCESS;
}

//Create pthreads according to pFileList->FileSize(HeadNode)
static PROCESS_STATUS EncryptDecrypt_Plus(FileList_t *pFileList, CTL_MENU func)
{
#ifdef PTHREAD_NUM_MAX
#if (PTHREAD_NUM_MAX < 2)
    #error STR_ERR_PTHREAD_NUM_TOO_SMALL
#elif (PTHREAD_NUM_MAX > 8)
    #error STR_ERR_PTHREAD_NUM_TOO_BIG
#endif
#endif

#ifdef __DEBUG
    if(NULL == pFileList)
    {
        DISP_ERR(STR_INVALID_PARAMTER);
        return PROCESS_STATUS_ELSE_ERR;
    }
#endif

    if(PTHREAD_NUM_MAX <= pFileList->FileSize)
        return EncryptDecrypt(pFileList, func);

    FILE *fp = fopen(FILE_LIST_LOG_NAME, "wb");
    if(NULL == fp)
    {
        DISP_ERR(STR_FAIL_TO_OPEN_LOG_FILE);
        return PROCESS_STATUS_ELSE_ERR;
    }

    //Create the window >>>
    WINDOW *ScheduleWin[PTHREAD_NUM_MAX];
    WINDOW *win[PTHREAD_NUM_MAX];
    int i, CycleIndex;

    CycleIndex = (PTHREAD_NUM_MAX <= pFileList->FileSize) ? 
        PTHREAD_NUM_MAX : pFileList->FileSize;

    for(i = 0; i < CycleIndex; i++)
    {
        ScheduleWin[i] = newwin(1, COLS, i*LINES/CycleIndex, 0);
    }
    
    for(i = 0; i < CycleIndex-1; i++) //PTHREAD_NUM_MAX-1 : the last win should tackle additionally
    {
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
            i*LINES/PTHREAD_NUM_MAX + 1 : schedule wins occupy the position i*LINES/PTHREAD_NUM_MAX, 
                so the win should start from to next line.
        */
        win[i] = newwin((LINES-CycleIndex)/CycleIndex, COLS, 
            i*LINES/CycleIndex + 1, 0);
    }

    /*
        It must be under considered the situation that the result of LINES/PTHREAD_NUM_MAX 
            is decimal fraction.
        Thus, use "LINES-PTHREAD_NUM_MAX - (LINES-PTHREAD_NUM_MAX)*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX"
            instead of "(LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX"
    */
    win[CycleIndex-1] = newwin(LINES-CycleIndex - 
        (LINES-CycleIndex)*(CycleIndex-1)/CycleIndex, 
        COLS, LINES*(CycleIndex-1)/CycleIndex + 1, 0);
    
    for(i = 0; i < CycleIndex; i++)
    {
        CTL_SET_COLOR(ScheduleWin[i], CTL_PANEL_CYAN);
        CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
        
        wmove(win[i], 0, 0);
        scrollok(win[i], true);
        
        wattron(ScheduleWin[i], A_UNDERLINE);
        mvwhline(ScheduleWin[i], 0, 0, ' ', COLS);

        mvwprintw(ScheduleWin[i], 0, 0, "%s%d->%s", STR_TASK, i+1, 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING);
        mvwprintw(ScheduleWin[i], 0, COLS*2/5, "%s0", STR_SUCCESS_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*3/5, "%s0", STR_FAIL_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*4/5, "%s0%%", STR_RATE);
    
        wrefresh(ScheduleWin[i]);
        wrefresh(win[i]);
    }

    g_pCurFilelist = pFileList->pNext; //First FileList is head node
    
    int RatioFactor[PTHREAD_NUM_MAX];
    int denominator[PTHREAD_NUM_MAX];
    PthreadArg_t PthreadArg[PTHREAD_NUM_MAX];
    pthread_t PthreadID[PTHREAD_NUM_MAX];
    int ret = 0;
    
    for(i = 0; i < CycleIndex; i++)
    {
        RatioFactor[i] = 0;
        denominator[i] = 100;
        InitPthreadArg(&PthreadArg[i]);
        
        PthreadArg[i].pFunc = (CTL_MENU_ENCRYPT == func) ? encrypt : decrypt;
        PthreadArg[i].pRatioFactor = &RatioFactor[i];
        PthreadArg[i].pCurFileList = g_pCurFilelist;
        PthreadArg[i].ProcessStatus = PROCESS_STATUS_BUSY;
        PthreadArg[i].SuccessCount = 0;
        PthreadArg[i].FailCount = 0;
        PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE;
    }
     
    for(i = 0; i < CycleIndex; i++)
    {   
        ret = pthread_create(&PthreadID[i], NULL, Pthread_EncryptDecrypt, &PthreadArg[i]);
	    if(ret != 0)
	    {
	        fclose(fp);
		    DISP_ERR(STR_FAIL_TO_CREATE_PTHREAD);
            return PROCESS_STATUS_ELSE_ERR;
	    }
    }
    
    char ExitFlag = 0;
    char flag = 0; //Set 1 if it has been refreshed <=> if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
    int StrSuccessWidth = GetWidth(STR_SUCCESS_COUNT);
    int StrFailWidth = GetWidth(STR_FAIL_COUNT);
    int StrRateWidth = GetWidth(STR_RATE);
	
	while(1)
    {
        if(ExitFlag)
            break;
        
        ExitFlag = 1;
        
        for(i = 0; i < CycleIndex; i++)
        {
            if(PROCESS_STATUS_BUSY == PthreadArg[i].ProcessStatus)
            {
                ExitFlag = 0;
                mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + StrRateWidth), 
                    "%d%%  ", RatioFactor[i]/denominator[i]);
                
                if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
                {
                    flag = 1;
                    switch(PthreadArg[i].RefreshFlag)
                    {
                        case REFRESH_FLAG_SUCCESS:
                            fprintf(fp, "%s\n", PthreadArg[i].pCurFileList->pFileName);
                            mvwprintw(ScheduleWin[i], 0, (COLS*2/5 + StrSuccessWidth), 
                                "%d", PthreadArg[i].SuccessCount);
                            mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + StrRateWidth), "100%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_GREEN);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_SUCCESS);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        case REFRESH_FLAG_FAIL:
                            mvwprintw(ScheduleWin[i], 0, (COLS*3/5 + StrFailWidth), 
                                "%d", PthreadArg[i].FailCount);
                            //mvwhline(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), ' ', 4);
                            //mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), "0%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_RED);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_FAIL);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        default:
                            RatioFactor[i] = 0;
                            denominator[i] = (int)(PthreadArg[i].pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
                            if(0 == denominator[i])
                                denominator[i] = 100;
                            //mvwhline(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), ' ', 4);
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
                            break;
                    }
                    
                    wrefresh(win[i]);
                    PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE; //Clear refresh flag, continue en/decrypting
                }
            
                wrefresh(ScheduleWin[i]);
            }
        }

        if(flag)
        {
            flag = 0;
            continue;
        }
        
        delay(REFRESH_INTERVAL);
	}

    fclose(fp);
        
    for(i = 0; i < CycleIndex; i++)
    {
        delwin(ScheduleWin[i]);
        delwin(win[i]);
    }
    
    touchwin(stdscr);
    refresh();
    
    for(i = 0; i < CycleIndex; i++)
    {
        if(PROCESS_STATUS_SUCCESS != PthreadArg[i].ProcessStatus)
        {
            if(PROCESS_STATUS_FATAL_ERR == PthreadArg[i].ProcessStatus)
                return PROCESS_STATUS_FATAL_ERR;
                
            return PROCESS_STATUS_ERR;
        }
    }
        
    return PROCESS_STATUS_SUCCESS;
}



//File list related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static FileList_t *ScanDirectory(char *pFolderName)
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

        //Ignore ".", ".."
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

        snprintf(pFileName, FileNameLenght, "%s%c%s", pFolderName, 
            DIR_DELIMITER, pEntry->d_name);
        
#ifdef __LINUX
        if(0 != lstat(pFileName, &FileInfo))
#elif defined __WINDOWS
        if(0 != _stati64(pFileName, &FileInfo))
#endif
        {
            free(pFileName);
            continue;
        }

#ifdef __LINUX
        if(DT_DIR == pEntry->d_type)
#elif defined __WINDOWS
        if(FileInfo.st_mode & S_IFDIR)
#endif
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

            while(NULL != pCurFileList->pNext)
            {
                pCurFileList = pCurFileList->pNext;
            }
            
            if(NULL != pNewFileList->pFileName)
                free(pNewFileList->pFileName);
            free(pNewFileList);
        }
#ifdef __LINUX
        else if(DT_REG == pEntry->d_type)
#elif defined __WINDOWS
        else if(FileInfo.st_mode & S_IFREG)
#endif
        {
            pNewFileList = (FileList_t *)malloc(sizeof(FileList_t));
            if(NULL == pNewFileList)
            {
                free(pFileName);
                FreeFileList(pHeadNode);
                closedir(pDir);
                return NULL;
            }
            
            pNewFileList->pFileName = pFileName;
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

static FileList_t *ScanEncryptFile(char *pFolderName)
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
            
        //Ignore ".", "..", "...", ".ept" or "..ept" or "...ept"
        if('.' == pEntry->d_name[0])
        {
            if('\0' == pEntry->d_name[1])
                continue;
            else if('.' == pEntry->d_name[1])
            {
                if('\0' == pEntry->d_name[2])
                    continue;
                else if('.' == pEntry->d_name[2])
                {
                    if('\0' == pEntry->d_name[3])
                        continue;
                    else if(('e' == pEntry->d_name[3]) && ('p' == pEntry->d_name[4]) && 
                            ('t' == pEntry->d_name[5]) && ('\0' == pEntry->d_name[6]))
                        continue;
                }
                else if(('e' == pEntry->d_name[2]) && ('p' == pEntry->d_name[3]) && 
                        ('t' == pEntry->d_name[4]) && ('\0' == pEntry->d_name[5]))
                    continue;
            }
            else if(('e' == pEntry->d_name[1]) && ('p' == pEntry->d_name[2]) && 
                    ('t' == pEntry->d_name[3]) && ('\0' == pEntry->d_name[4]))
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

        snprintf(pFileName, FileNameLenght, "%s%c%s", pFolderName, 
            DIR_DELIMITER, pEntry->d_name);
        
#ifdef __LINUX
        if(0 != lstat(pFileName, &FileInfo))
#elif defined __WINDOWS
        if(0 != _stati64(pFileName, &FileInfo))
#endif
        {
            free(pFileName);
            continue;
        }
        
#ifdef __LINUX
        if(DT_DIR == pEntry->d_type)
#elif defined __WINDOWS
        if(FileInfo.st_mode & S_IFDIR)
#endif
        {
            pNewFileList = ScanEncryptFile(pFileName);
            if(NULL == pNewFileList)
            {
                free(pFileName);
                continue;
            }
            
            pHeadNode->FileSize += pNewFileList->FileSize;
            pCurFileList->pNext = pNewFileList->pNext;
            pCurFileList = pNewFileList;

            while(NULL != pCurFileList->pNext)
            {
                pCurFileList = pCurFileList->pNext;
            }
            
            if(NULL != pNewFileList->pFileName)
                free(pNewFileList->pFileName);
            free(pNewFileList);
        }
#ifdef __LINUX
        else if(DT_REG == pEntry->d_type)
#elif defined __WINDOWS
        else if(FileInfo.st_mode & S_IFREG)
#endif
        {
            if(0 != IsEncryptFile(pEntry->d_name, FileNameLenght-FolderNameLenght-2))
            {
                free(pFileName);
                continue;
            }
            
            pNewFileList = (FileList_t *)malloc(sizeof(FileList_t));
            if(NULL == pNewFileList)
            {
                free(pFileName);
                FreeFileList(pHeadNode);
                closedir(pDir);
                return NULL;
            }
            
            pNewFileList->pFileName = pFileName;
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

static G_STATUS BeforeEncryptDecrypt(void)
{
    if(NULL != g_LogFile)
    {
        fclose(g_LogFile);
    }
    
    g_LogFile = fopen(LOG_FILE_NAME, "w+");
    if(NULL == g_LogFile)
    {
        CTL_DispWarning(STR_FAIL_TO_OPEN_LOG_FILE);
        return STAT_ERR;
    }
    fclose(g_LogFile);

    g_LogFile = fopen(LOG_FILE_NAME, "a+");
    if(NULL == g_LogFile)
    {
        CTL_DispWarning(STR_FAIL_TO_OPEN_LOG_FILE);
        return STAT_ERR;
    }
    
    return STAT_OK;
}

static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus)
{
    if(PROCESS_STATUS_ELSE_ERR == ProcessStatus)
    {
        CTL_DispWarning(g_ErrBuf);
        return STAT_ERR;
    }

//    if(PROCESS_STATUS_FATAL_ERR == ProcessStatus)
//    {
//        return STAT_FATAL_ERR;
//    }
    
    WINDOW *win = newwin(CTL_RESULT_WIN_LINES, CTL_RESULT_WIN_COLS, 
            (LINES-CTL_RESULT_WIN_LINES)/2, (COLS-CTL_RESULT_WIN_COLS)/2);
    int key;
    char buf[COLS - 4];

    keypad(win, true);
    
    if(PROCESS_STATUS_SUCCESS == ProcessStatus)
    {
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        wattron(win, A_BOLD);
        snprintf(buf, sizeof(buf), "%s [ %s: %d ]", STR_SUCCESS, STR_TOTAL, 
            g_SuccessFailCountTable[0]);
        mvwaddstr(win, 2, (CTL_RESULT_WIN_COLS-GetWidth(buf))/2, buf);
        wattroff(win, A_BOLD);
        
        wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');
        wattron(win, A_REVERSE);
        mvwaddstr(win, 3, (CTL_RESULT_WIN_COLS-GetWidth(STR_GO_BACK))/2, STR_GO_BACK);
        wattroff(win, A_REVERSE);
        wrefresh(win);
        
        while(1)
        {
            key = wgetch(win);
            if(CTL_KEY_ENTER == key)
                break;
            else
                continue;
        }
        
        delwin(win);
        touchline(stdscr, (LINES-CTL_RESULT_WIN_LINES)/2, CTL_RESULT_WIN_LINES);
        refresh();

        return STAT_OK;
    }
    
    //If error >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    snprintf(buf, sizeof(buf), "%s[ %d ]     %s[ %d ]", STR_SUCCESS, 
        g_SuccessFailCountTable[0], STR_FAIL, g_SuccessFailCountTable[1]);
    CTL_SET_COLOR(win, CTL_PANEL_GREEN);
    wattron(win, A_BOLD);
    mvwprintw(win, 2, (CTL_RESULT_WIN_COLS-GetWidth(buf))/2, 
        "%s[ %d ]     ", STR_SUCCESS, g_SuccessFailCountTable[0]);
    CTL_SET_COLOR(win, CTL_PANEL_RED);
    wprintw(win, "%s[ %d ]", STR_FAIL, g_SuccessFailCountTable[1]);
    wattroff(win, A_BOLD);
        
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');
    
    char *pStr1 = STR_VIEW_LOG;
    char *pStr2 = STR_GO_BACK;
    int Str1StartX = (CTL_RESULT_WIN_COLS - GetWidth(pStr1) - GetWidth(pStr2))/3;
    int Str2StartX = CTL_RESULT_WIN_COLS - Str1StartX - GetWidth(pStr2);
    char flag = 0;

    mvwaddstr(win, 4, Str2StartX, pStr2);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 4, Str1StartX, pStr1);
    wattroff(win, A_REVERSE);
    
    while(1)
    {
        key = wgetch(win);
        if((CTL_KEY_LEFT == key) || (CTL_KEY_RIGHT == key))
        {
            flag ^= 1;
        }
        else if(CTL_KEY_ENTER == key) //Enter key
            break;
        else
            continue;

        if(flag)
        {
            mvwaddstr(win, 4, Str1StartX, pStr1);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 4, Str2StartX, pStr2);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, 4, Str2StartX, pStr2);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 4, Str1StartX, pStr1);
            wattroff(win, A_REVERSE);
        }
    }
    
    delwin(win);
    touchline(stdscr, (LINES-CTL_RESULT_WIN_LINES)/2, CTL_RESULT_WIN_LINES);
    refresh();

    if(flag)
        return STAT_BACK;

    fclose(g_LogFile);
    g_LogFile = NULL;
    CTL_ShowFile(LOG_FILE_NAME);
    
    return STAT_OK;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void DeleteTailSymbol(char *pFileName, int FileNameLenght)
{
    if(('/' == pFileName[FileNameLenght-1]) || (92 == pFileName[FileNameLenght-1])) //'\' ASCII is 92
    {
        pFileName[FileNameLenght-1] = '\0';
    }
}

static inline void InitFileListNode(FileList_t *pFileList)
{
    pFileList->pFileName = NULL;
    pFileList->FileNameLenght = 0;
    pFileList->FileSize = 0;
    pFileList->pNext = NULL;
}

static inline void FreeFileList(FileList_t *pHeadNode)
{
    FileList_t *CurNode = pHeadNode;
    FileList_t *TmpNode;
    while(NULL != CurNode)
    {
        TmpNode = CurNode->pNext;
        if(NULL != CurNode->pFileName)
            free(CurNode->pFileName);
        free(CurNode);
        CurNode = TmpNode;
    }
}

static inline int IsEncryptFile(const char *pFileName, int FileNameLenght)
{
    if(('t' == pFileName[FileNameLenght-1]) && ('p' == pFileName[FileNameLenght-2]) && 
       ('e' == pFileName[FileNameLenght-3]) && ('.' == pFileName[FileNameLenght-4]))
       return 0;

    return -1;
}
