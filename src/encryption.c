/*************************************************************************
	> File Name: encryption.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:37:45
 ************************************************************************/

#include "encryption.h"
#include "core_code.h"
#include <unistd.h>
#include <dirent.h>

#ifdef PTHREAD_NUM_MAX
#if (PTHREAD_NUM_MAX != 1) && (PTHREAD_NUM_MAX != 2) && \
    (PTHREAD_NUM_MAX != 4) && (PTHREAD_NUM_MAX != 6) && \
    (PTHREAD_NUM_MAX != 8)
#error "Macro define rule: PTHREAD_NUM_MAX = 1 / 2 / 4 / 6 / 8"
#undef PTHREAD_NUM_MAX
#define PTHREAD_NUM_MAX 1
#endif
#endif

#ifdef FREE_MEMORY
#if (FREE_MEMORY != 120) && (FREE_MEMORY != 240) && \
    (FREE_MEMORY != 360) && (FREE_MEMORY != 480) && \
    (FREE_MEMORY != 600) && (FREE_MEMORY != 720)
#error "Macro define rule: FREE_MEMORY = 120 / 240 / 360 / 480 / 600 / 720"
#endif
#endif

static PROCESS_STATUS EncryptDecrypt(FileList_t *pFileList, CTL_MENU func);
static PROCESS_STATUS EncryptDecrypt_plus(FileList_t *pFileList, CTL_MENU func);
static G_STATUS BeforeEncryptDecrypt(void);
static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus);
static G_STATUS ProcessEncryptDecryptResult(PROCESS_STATUS ProcessStatus);
static FileList_t *ScanDirectory(char *pFolderName);
static FileList_t *ScanEncryptFile(char *pFolderName);
static inline void DeleteTailSymbol(char *pFileName, int FileNameLength);
static inline void InitFileListNode(FileList_t *pFileList);
static inline void FreeFileList(FileList_t *pHeadNode);
static inline G_STATUS IsEncryptFormat(const char *pFileName, int FileNameLength);
static inline G_STATUS CheckEncryptResult(char *pFileName, int FileNameLength);
static inline G_STATUS CheckDecryptResult(char *pFileName, int FileNameLength);

char g_password[CTL_PASSWORD_LENGHT_MAX];
pthread_mutex_t g_LogLock = PTHREAD_MUTEX_INITIALIZER;
static int g_SuccessFailCountTable[2] = {0, 0};



/*
 *  @Briefs: Get file name and password, invoke EncryptDecrypt at the end
 *  @Return: Always STAT_OK
 *  @Note:   None
 */
G_STATUS CTL_EncryptDecrypt(CTL_MENU func)
{
#ifdef __DEBUG
    if((CTL_MENU_ENCRYPT != func) && (CTL_MENU_DECRYPT != func))
    {
        CTL_DispWarning(STR_INVALID_FUNC);
        return STAT_OK;
    }
#endif
    
    stat_t FileInfo;
    char FileName[CTL_FILE_NAME_LENGHT];
    int FileNameLength;
    
    if(STAT_OK != CTL_GetFileName(FileName))
        return STAT_OK;

#ifdef __WINDOWS
    ConvertNameFormat(FileName);
#endif        
    FileNameLength = strlen(FileName);
    DeleteTailSymbol(FileName, FileNameLength);

    if(0 != GetFileInfo(FileName, &FileInfo))
    {
        CTL_DispWarning("%s: %s\n", STR_FAIL_TO_GET_FILE_FOLDER_INFO, FileName);
        return STAT_OK;
    }
    
    if(STAT_OK != CTL_GetPassord(g_password))
        return STAT_OK;

    PROCESS_STATUS ProcessStatus = PROCESS_STATUS_ERR;

    if(S_IFREG & FileInfo.st_mode)
    {
        if(STAT_OK != BeforeEncryptDecrypt())
            return STAT_OK;
    
        FileList_t HeadNode, FileList;
        HeadNode.pFileName = NULL;
        HeadNode.FileNameLength = 0;
        HeadNode.FileSize = 1;
        HeadNode.pNext = &FileList;
        FileList.pFileName = FileName;
        FileList.FileNameLength = FileNameLength;
        FileList.FileSize = FileInfo.st_size;
        FileList.pNext = NULL;

        ProcessStatus = EncryptDecrypt(&HeadNode, func);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        FileList_t *pFileList = NULL;
        
        pFileList = (CTL_MENU_ENCRYPT == func) ? 
            ScanDirectory(FileName) : ScanEncryptFile(FileName);
            
        if(NULL == pFileList)
        {
            CTL_DispWarning("%s: %s\n%s\n", STR_FAIL_TO_SCAN_DIRECTORY, FileName, 
                (CTL_MENU_ENCRYPT == func) ? STR_PRO_REASON_NO_ENCRYPT_FILE : STR_PRO_REASON_NO_DECRYPT_FILE);
            return STAT_OK;
        }

//Fix bug #2.1
#ifdef __LINUX
    #ifdef __64BIT
        if(STAT_OK != CTL_ConfirmOperation("%s%s\n%s: %ld, %s\n", 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING, FileName, 
            STR_TOTAL_FILE, pFileList->FileSize, STR_IF_CONTINUE))
     #elif defined __32BIT
        if(STAT_OK != CTL_ConfirmOperation("%s%s\n%s: %d, %s\n", 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING, FileName, 
            STR_TOTAL_FILE, pFileList->FileSize, STR_IF_CONTINUE))
      #endif
#elif defined __WINDOWS
        if(STAT_OK != CTL_ConfirmOperation("%s%s %d      \n", 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING,  
            STR_TOTAL_FILE, pFileList->FileSize))
#endif
        {
            FreeFileList(pFileList);
            return STAT_OK;
        }
        
        if(STAT_OK != BeforeEncryptDecrypt())
        {
            FreeFileList(pFileList);
            return STAT_OK;
        }
        
        //ProcessStatus = EncryptDecrypt(pFileList, func);
        ProcessStatus = EncryptDecrypt(pFileList, func);

        FreeFileList(pFileList);
    }

    AfterEncryptDecrypt(ProcessStatus);

    return STAT_OK;
}



//Static function
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Create task to encrypt or decrypt according to pFileList->FileSize(HeadNode)
 *  @Return: PROCESS_STATUS_ELSE_ERR, PROCESS_STATUS_ERR, 
 *           PROCESS_STATUS_FATAL_ERR, PROCESS_STATUS_SUCCESS
 *  @Note:   pFileList must be the HeadNode
 */
static PROCESS_STATUS EncryptDecrypt(FileList_t *pFileList, CTL_MENU func)
{
#if (PTHREAD_NUM_MAX == 1)
    return EncryptDecrypt_plus(pFileList, func);
#endif

#ifdef __DEBUG
    if(NULL == pFileList)
    {
        DISP_ERR_DEBUG(STR_INVALID_PARAMTER);
        return PROCESS_STATUS_ELSE_ERR;
    }
#endif

    if(PTHREAD_NUM_MAX <= pFileList->FileSize)
        return EncryptDecrypt_plus(pFileList, func);

    //Create the window >>>
    WINDOW *ScheduleWin[PTHREAD_NUM_MAX];
    WINDOW *win[PTHREAD_NUM_MAX];
    int StartPosY = 1;
    int CycleIndex = pFileList->FileSize; //pFileList->FileSize is lower than PTHREAD_NUM_MAX in this situation
    int i, remainder = (LINES-CycleIndex)%CycleIndex;
    
    for(i = 0; i < remainder; i++)
    {
        ScheduleWin[i] = newwin(1, COLS, StartPosY-1, 0);
        
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
            (LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX + 1 : 
                if (LINES-PTHREAD_NUM_MAX)%PTHREAD_NUM_MAX isn't equal to 0,
                the window should occupy one more line
        */
        win[i] = newwin((LINES-CycleIndex)/CycleIndex + 1, COLS, StartPosY, 0);
        StartPosY += (LINES-CycleIndex)/CycleIndex + 1 + 1; //The last "1" is schedule win's
    }
    
    //Must follow above cycle
    for(; i < CycleIndex; i++) //Can not initialize i here
    {
        ScheduleWin[i] = newwin(1, COLS, StartPosY-1, 0);
    
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
        */
        win[i] = newwin((LINES-CycleIndex)/CycleIndex, COLS, StartPosY, 0);
        StartPosY += (LINES-CycleIndex)/CycleIndex + 1; //The last "1" is schedule win's
    }

    for(i = 0; i < CycleIndex; i++)
    {
        CTL_SET_COLOR(ScheduleWin[i], CTL_PANEL_CYAN);
//Fix bug #2.1
#ifdef __WINDOWS
        CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
#endif
        
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
		    DISP_ERR(STR_FAIL_TO_CREATE_PTHREAD);
            return PROCESS_STATUS_ELSE_ERR;
	    }
    }
    
    char ExitFlag = 0;
    char flag = 0; //Set 1 if it has been refreshed <=> if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
    int StrSuccessWidth = GetWidth(STR_SUCCESS_COUNT);
    int StrFailWidth = GetWidth(STR_FAIL_COUNT);
    int StrRateWidth = GetWidth(STR_RATE);
#ifdef __WINDOWS
    char *pConverFormat;
#endif
	
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
                            mvwprintw(ScheduleWin[i], 0, (COLS*2/5 + StrSuccessWidth), 
                                "%d", PthreadArg[i].SuccessCount);
                            mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + StrRateWidth), "100%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_GREEN);
//Fix bug #2.1
#ifdef __LINUX
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_SUCCESS);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
#elif defined __WINDOWS
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s", STR_SUCCESS);
                            CTL_FixBug_IncompleteDisp(win[i], NULL, 3);
                            wprintw(win[i], "\n");
                            wattroff(win[i], A_BOLD);
                            CTL_RESET_COLOR(win[i], CTL_PANEL_GREEN);
#endif
                            break;
                        case REFRESH_FLAG_FAIL:
                            mvwprintw(ScheduleWin[i], 0, (COLS*3/5 + StrFailWidth), 
                                "%d", PthreadArg[i].FailCount);
                            CTL_SET_COLOR(win[i], CTL_PANEL_RED);
//Fix bug #2.1
#ifdef __LINUX
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_FAIL);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
#elif defined __WINDOWS
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s", STR_FAIL);
                            CTL_FixBug_IncompleteDisp(win[i], NULL, 3);
                            wprintw(win[i], "\n");
                            wattroff(win[i], A_BOLD);
                            CTL_RESET_COLOR(win[i], CTL_PANEL_RED);
#endif
                            break;
                        default:
                            RatioFactor[i] = 0;
                            denominator[i] = (int)(PthreadArg[i].pCurFileList->FileSize / BASE_FILE_SIZE);
                            if(0 == denominator[i])
                                denominator[i] = 100;
#ifdef __LINUX
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
#elif defined __WINDOWS
                            if(TRUE == UTF8_VerifyStrFormat(PthreadArg[i].pCurFileList->pFileName))
                            {
                                wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
                            }
                            else
                            {
                                pConverFormat = ANSIToUTF8(PthreadArg[i].pCurFileList->pFileName, NULL);
                                wprintw(win[i], "%s\n", pConverFormat);
                                if(NULL != pConverFormat)
                                {
                                    free(pConverFormat);
                                }
                            }
#endif
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

    for(i = 0; i < CycleIndex; i++)
    {
        delwin(ScheduleWin[i]);
        delwin(win[i]);
    }
    
    touchwin(stdscr);
    refresh();
    
    for(i = 0; i < CycleIndex; i++)
    {
        g_SuccessFailCountTable[0] += PthreadArg[i].SuccessCount;
        g_SuccessFailCountTable[1] += PthreadArg[i].FailCount;
    }

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

/*
 *  @Briefs: Create task to encrypt or decrypt according to macro PTHREAD_NUM_MAX
 *  @Return: PROCESS_STATUS_ELSE_ERR, PROCESS_STATUS_ERR, 
 *           PROCESS_STATUS_FATAL_ERR, PROCESS_STATUS_SUCCESS
 *  @Note:   pFileList must be the HeadNode
 */
static PROCESS_STATUS EncryptDecrypt_plus(FileList_t *pFileList, CTL_MENU func)
{
#ifdef __DEBUG
    if(NULL == pFileList)
    {
        DISP_ERR_DEBUG(STR_INVALID_PARAMTER);
        return PROCESS_STATUS_ELSE_ERR;
    }
#endif

    //Create the window >>>
    WINDOW *ScheduleWin[PTHREAD_NUM_MAX];
    WINDOW *win[PTHREAD_NUM_MAX];
    int StartPosY = 1;
    int i, remainder = (LINES-PTHREAD_NUM_MAX)%PTHREAD_NUM_MAX;
    
    for(i = 0; i < remainder; i++)
    {
        ScheduleWin[i] = newwin(1, COLS, StartPosY-1, 0);
        
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
            (LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX + 1 : 
                if (LINES-PTHREAD_NUM_MAX)%PTHREAD_NUM_MAX isn't equal to 0,
                the window should occupy one more line
        */
        win[i] = newwin((LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX + 1, COLS, StartPosY, 0);
        StartPosY += (LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX + 1 + 1; //The last "1" is schedule win's
    }

    //Must follow above cycle
    for(; i < PTHREAD_NUM_MAX; i++) //Can not initialize i here
    {
        ScheduleWin[i] = newwin(1, COLS, StartPosY-1, 0);
    
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
        */
        win[i] = newwin((LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX, COLS, StartPosY, 0);
        StartPosY += (LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX + 1; //The last "1" is schedule win's
    }
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        CTL_SET_COLOR(ScheduleWin[i], CTL_PANEL_CYAN);
//Fix bug #2.1
#ifdef __LINUX
        CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
#endif
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
		    DISP_ERR(STR_FAIL_TO_CREATE_PTHREAD);
            return PROCESS_STATUS_ELSE_ERR;
	    }
    }
    
    char ExitFlag = 0;
    char flag = 0; //Set 1 if it has been refreshed <=> if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
    int StrSuccessWidth = GetWidth(STR_SUCCESS_COUNT);
    int StrFailWidth = GetWidth(STR_FAIL_COUNT);
    int StrRateWidth = GetWidth(STR_RATE);
#ifdef __WINDOWS
    char *pConverFormat;
#endif
	
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
                            mvwprintw(ScheduleWin[i], 0, (COLS*2/5 + StrSuccessWidth), 
                                "%d", PthreadArg[i].SuccessCount);
                            mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + StrRateWidth), "100%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_GREEN);
                            //Fix bug #2.1
#ifdef __LINUX
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_SUCCESS);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
#elif defined __WINDOWS
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s", STR_SUCCESS);
                            CTL_FixBug_IncompleteDisp(win[i], NULL, 3);
                            wprintw(win[i], "\n");
                            wattroff(win[i], A_BOLD);
                            CTL_RESET_COLOR(win[i], CTL_PANEL_GREEN);
#endif
                            break;
                        case REFRESH_FLAG_FAIL:
                            mvwprintw(ScheduleWin[i], 0, (COLS*3/5 + StrFailWidth), 
                                "%d", PthreadArg[i].FailCount);
                            CTL_SET_COLOR(win[i], CTL_PANEL_RED);
                            //Fix bug #2.1
#ifdef __LINUX
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_FAIL);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
#elif defined __WINDOWS
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s", STR_FAIL);
                            CTL_FixBug_IncompleteDisp(win[i], NULL, 3);
                            wprintw(win[i], "\n");
                            wattroff(win[i], A_BOLD);
                            CTL_RESET_COLOR(win[i], CTL_PANEL_RED);
#endif
                            break;
                        default:
                            RatioFactor[i] = 0;
                            denominator[i] = (int)(PthreadArg[i].pCurFileList->FileSize / BASE_FILE_SIZE);
                            if(0 == denominator[i])
                                denominator[i] = 100;
#ifdef __LINUX
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
#elif defined __WINDOWS
                            if(TRUE == UTF8_VerifyStrFormat(PthreadArg[i].pCurFileList->pFileName))
                            {
                                wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
                            }
                            else
                            {
                                pConverFormat = ANSIToUTF8(PthreadArg[i].pCurFileList->pFileName, NULL);
                                wprintw(win[i], "%s\n", pConverFormat);
                                if(NULL != pConverFormat)
                                {
                                    free(pConverFormat);
                                }
                            }
#endif
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



//File list related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Search files with all format in pFolderName
 *  @Return: Fail or directory is empty return NULL, else return FileList_t pointer
 *  @Note:   None
 */
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
    int FolderNameLength = strlen(pFolderName);
    int FileNameLength;
    char *pFileName;
    stat_t FileInfo;

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

        FileNameLength = FolderNameLength + strlen(pEntry->d_name) + 2 + sizeof(ENCRYPT_FILE_SUFFIX_NAME);
        /*
            + 2 : Contains '/' or '\' and '\0'
            + sizeof(ENCRYPT_FILE_SUFFIX_NAME) : For CheckEncryptResult()
        */
        
        pFileName = (char *)malloc(FileNameLength);
        if(NULL == pFileName)
        {
            FreeFileList(pHeadNode);
            closedir(pDir);
            return NULL;
        }

        snprintf(pFileName, FileNameLength, "%s%c%s", pFolderName, 
            DIR_DELIMITER, pEntry->d_name);

        if(0 != GetFileInfo(pFileName, &FileInfo))
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
            {
                free(pNewFileList->pFileName);
            }
            free(pNewFileList);
        }
#ifdef __LINUX
        else if(DT_REG == pEntry->d_type)
#elif defined __WINDOWS
        else if(FileInfo.st_mode & S_IFREG)
#endif
        {
            if(STAT_OK != CheckEncryptResult(pFileName, FileNameLength))
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
            pNewFileList->FileNameLength = FileNameLength - sizeof(ENCRYPT_FILE_SUFFIX_NAME);
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

/*
 *  @Briefs: Search files with .ept format in pFolderName
 *  @Return: Fail or directory is empty return NULL, else return FileList_t pointer
 *  @Note:   Only search *.ept file
 */
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
    int FolderNameLength = strlen(pFolderName);
    int FileNameLength;
    char *pFileName;
    stat_t FileInfo;

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
        
        FileNameLength = FolderNameLength + strlen(pEntry->d_name) + 2; //contains '/' or '\' and '\0'
        pFileName = (char *)malloc(FileNameLength);
        if(NULL == pFileName)
        {
            FreeFileList(pHeadNode);
            closedir(pDir);
            return NULL;
        }

        snprintf(pFileName, FileNameLength, "%s%c%s", pFolderName, 
            DIR_DELIMITER, pEntry->d_name);
        
        if(0 != GetFileInfo(pFileName, &FileInfo))
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
            {
                free(pNewFileList->pFileName);
            }
            free(pNewFileList);
        }
#ifdef __LINUX
        else if(DT_REG == pEntry->d_type)
#elif defined __WINDOWS
        else if(FileInfo.st_mode & S_IFREG)
#endif
        {
            if(STAT_OK != IsEncryptFormat(pFileName, FileNameLength))
            {
                free(pFileName);
                continue;
            }
            
            if(STAT_OK != CheckDecryptResult(pFileName, FileNameLength))
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
            pNewFileList->FileNameLength = FileNameLength;
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

/*
 *  @Briefs: Do something before encrypt or decrypt
 *  @Return: STAT_ERR / STAT_OK
 *  @Note:   None
 */
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

    g_SuccessFailCountTable[0] = 0;
    g_SuccessFailCountTable[1] = 0;
    
    CTL_DisableButton();
    
    return STAT_OK;
}

/*
 *  @Briefs: Do something after encrypt or decrypt
 *  @Return: Always STAT_OK
 *  @Note:   None
 */
static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus)
{
    ProcessEncryptDecryptResult(ProcessStatus);

    return STAT_OK;
}

/*
 *  @Briefs: Process the result of encryption or decryption
 *  @Return: Always STAT_OK
 *  @Note:   None
 */
static G_STATUS ProcessEncryptDecryptResult(PROCESS_STATUS ProcessStatus)
{
    fclose(g_LogFile);
    g_LogFile = NULL;
    
    if(PROCESS_STATUS_ELSE_ERR == ProcessStatus)
    {
        unlink(LOG_FILE_NAME);
        CTL_DispWarning("%s\n", g_ErrBuf);
        return STAT_OK;
    }
    
    WINDOW *win = newwin(CTL_RESULT_WIN_LINES, CTL_RESULT_WIN_COLS, 
            (LINES-CTL_RESULT_WIN_LINES)/2, (COLS-CTL_RESULT_WIN_COLS)/2);
    
    int key;
    char buf[COLS - 4];

    keypad(win, true);
    
    if(PROCESS_STATUS_SUCCESS == ProcessStatus)
    {
        unlink(LOG_FILE_NAME);

        wattron(win, A_BOLD);
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        snprintf(buf, sizeof(buf), "%s%s %d", STR_SUCCESS_COUNT, STR_TOTAL_FILE, 
            g_SuccessFailCountTable[0]);
#ifdef __LINUX
        mvwaddstr(win, 1, (CTL_RESULT_WIN_COLS-GetWidth(buf))/2, buf);
#elif defined __WINDOWS
        mvwaddstr(win, 1, 1, buf);
        CTL_FixBug_IncompleteDisp(win, buf, UTF8_GetSpecialSymbolNum(buf)+1);
#endif
        CTL_RESET_COLOR(win, CTL_PANEL_GREEN);
        wattroff(win, A_BOLD);

        CTL_SET_COLOR(win, CTL_PANEL_CYAN);
        wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');
        wattron(win, A_REVERSE);
        mvwaddstr(win, 2, (CTL_RESULT_WIN_COLS-GetWidth(STR_GO_BACK))/2, STR_GO_BACK);
        CTL_FixBug_IncompleteDisp(win, buf, 3);
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
    wattron(win, A_BOLD);
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    snprintf(buf, sizeof(buf), "%s%d   %s%d", STR_SUCCESS_COUNT, 
        g_SuccessFailCountTable[0], STR_FAIL_COUNT, g_SuccessFailCountTable[1]);
#ifdef __LINUX
    mvwaddstr(win, 1, (CTL_RESULT_WIN_COLS-GetWidth(buf))/2, buf);
#elif defined __WINDOWS
    mvwaddstr(win, 1, 1, buf);
    CTL_FixBug_IncompleteDisp(win, buf, UTF8_GetSpecialSymbolNum(buf)+1);
#endif
    wattroff(win, A_BOLD);
        
    wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');
    
    char *pStr1 = STR_VIEW_LOG;
    char *pStr2 = STR_GO_BACK;
    int Str1StartX = (CTL_RESULT_WIN_COLS - GetWidth(pStr1) - GetWidth(pStr2))/3;
    int Str2StartX = CTL_RESULT_WIN_COLS - Str1StartX - GetWidth(pStr2);
    char flag = 0;

    mvwaddstr(win, 2, Str2StartX, pStr2);
    CTL_FixBug_IncompleteDisp(win, pStr2, 3);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 2, Str1StartX, pStr1);
    CTL_FixBug_IncompleteDisp(win, pStr1, 3);
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
            mvwaddstr(win, 2, Str1StartX, pStr1);
            CTL_FixBug_IncompleteDisp(win, pStr1, 3);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 2, Str2StartX, pStr2);
            CTL_FixBug_IncompleteDisp(win, pStr2, 3);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, 2, Str2StartX, pStr2);
            CTL_FixBug_IncompleteDisp(win, pStr2, 3);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 2, Str1StartX, pStr1);
            CTL_FixBug_IncompleteDisp(win, pStr1, 3);
            wattroff(win, A_REVERSE);
        }
    }
    
    delwin(win);
    touchline(stdscr, (LINES-CTL_RESULT_WIN_LINES)/2, CTL_RESULT_WIN_LINES);
    refresh();

    if(flag)
        return STAT_OK;

    CTL_ShowFile(LOG_FILE_NAME);
    
    return STAT_OK;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Delete the '\\' or '/' in the tail of pFileName
 *  @Return: None
 *  @Note:   pFileName can't be NULL and FileNameLength must be the length of pFileName
 */
static inline void DeleteTailSymbol(char *pFileName, int FileNameLength)
{
    if(('/' == pFileName[FileNameLength-1]) || (92 == pFileName[FileNameLength-1])) //'\' ASCII is 92
    {
        pFileName[FileNameLength-1] = '\0';
    }
}

/*
 *  @Briefs: Initialize the member of pFileList
 *  @Return: None
 *  @Note:   pFileList can't be NULL
 */
static inline void InitFileListNode(FileList_t *pFileList)
{
    pFileList->pFileName = NULL;
    pFileList->FileNameLength = 0;
    pFileList->FileSize = 0;
    pFileList->pNext = NULL;
}

/*
 *  @Briefs: Free FileList_t
 *  @Return: None
 *  @Note:   pHeadNode must be the head node of link list
 */
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

/*
 *  @Briefs: Check for .ept format
 *  @Return: STAT_ERR / STAT_OK
 *  @Note:   The FileNameLength must the length of pFileName(count in '\0')
 */
static inline G_STATUS IsEncryptFormat(const char *pFileName, int FileNameLength)
{
    if(('t' == pFileName[FileNameLength-2]) && ('p' == pFileName[FileNameLength-3]) && 
       ('e' == pFileName[FileNameLength-4]) && ('.' == pFileName[FileNameLength-5]))
       return STAT_OK;

    return STAT_ERR;
}

/*
 *  @Briefs: Remove the encrypt suffix and check if it does exist or not
 *  @Return: STAT_ERR / STAT_OK, STAT_ERR means it does exist
 *  @Note:   1. The Length of pFileName must be enough, the length must be equal to
                length of pFileName(count in '\0') + sizeof(ENCRYPT_FILE_SUFFIX_NAME)
 */
static inline G_STATUS CheckEncryptResult(char *pFileName, int FileNameLength)
{
    G_STATUS status;
    char *ptr = pFileName + FileNameLength-1 - sizeof(ENCRYPT_FILE_SUFFIX_NAME);
    
    if('\0' != *ptr)
        return STAT_ERR;
    
    *ptr++ = '.';
    *ptr++ = 'e';
    *ptr++ = 'p';
    *ptr++ = 't';
    *ptr   = '\0';
    
    status = (0 == access(pFileName, F_OK)) ? STAT_ERR : STAT_OK;
    *(ptr-sizeof(ENCRYPT_FILE_SUFFIX_NAME)+1) = '\0'; 
    /*
        <=> *(ptr-4) = '\0'; 
        //Varible ptr has been increased 4 before
    */
    

    return status;
}

/*
 *  @Briefs: Remove the encrypt suffix and check if it does exist or not
 *  @Return: STAT_ERR / STAT_OK, STAT_ERR means it does exist
 *  @Note:   The FileNameLength must count in '\0'
 */
static inline G_STATUS CheckDecryptResult(char *pFileName, int FileNameLength)
{
    G_STATUS status;
    char backup = pFileName[FileNameLength - sizeof(ENCRYPT_FILE_SUFFIX_NAME)]; //For .ept format, this position is character '.'
    
    pFileName[FileNameLength - sizeof(ENCRYPT_FILE_SUFFIX_NAME)] = '\0';
    status = (0 == access(pFileName, F_OK)) ? STAT_ERR : STAT_OK;
    pFileName[FileNameLength - sizeof(ENCRYPT_FILE_SUFFIX_NAME)] = backup;

    return status;
}
