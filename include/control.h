/*************************************************************************
	> File Name: control.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:49:29
 ************************************************************************/

#ifndef __CONTROL_H
#define __CONTROL_H

#include "common.h"
#ifdef __LINUX
#include "curses.h"
#elif defined __WINDOWS
#include "curses.h"
#endif
#include "str.h"

#define CTL_KEY_TAB                             9   //ASCII
#define CTL_KEY_DOWN                            KEY_DOWN
#define CTL_KEY_UP                              KEY_UP
#define CTL_KEY_LEFT                            KEY_LEFT
#define CTL_KEY_RIGHT                           KEY_RIGHT
#define CTL_KEY_ENTER                           13  //ASCII
#define CTL_KEY_ESC                             27  //ASCII
#define CTL_KEY_NPAGE                           KEY_NPAGE
#define CTL_KEY_PPAGE                           KEY_PPAGE
#ifdef __LINUX
#define CTL_KEY_BACKSPACE                       KEY_BACKSPACE
#elif defined __WINDOWS
#define CTL_KEY_BACKSPACE                       8  //ASCII
#endif

#define CTL_PASSWORD_LENGHT_MAX                 19  //real is 19-1=18, end with '\0'
#define CTL_PASSWORD_LENGHT_MIN                 8
#define CTL_FILE_NAME_LENGHT                    (CTL_GET_FILE_NAME_WIN_COLS + 32)
#define CTL_FILE_SIZE_MAX                       (1024*1024*50) //50Mb
#define CTL_SINGLE_LINE_WIDTH                   (COLS*6 + 1) //1~6 bytes per special char, end with '\0'

#define CTL_CONSOLE_COLS                        80
#define CTL_CONSOLE_LINES                       24

#define CTL_CHOOSE_LANGUAGE_WIN_COLS            20
#define CTL_CHOOSE_LANGUAGE_WIN_LINES           4

#define CTL_MENU_WIN_COLS                       25
#define CTL_MENU_WIN_LINES                      (CTL_MENU_MAX+2)

#define CTL_GET_FILE_NAME_WIN_COLS              60
#define CTL_GET_FILE_NAME_WIN_LINES             6

#define CTL_GET_PASSWORD_WIN_COLS               40
#define CTL_GET_PASSWORD_WIN_LINES              6

#define CTL_ENCRYPT_FILE_WIN_COLS               60
#define CTL_DECRYPT_FILE_WIN_COLS               60

#define CTL_RESULT_WIN_COLS                     40
#define CTL_RESULT_WIN_LINES                    4

#define CTL_CONFIRM_WIN_COLS                    40
#define CTL_CONFIRM_WIN_LINES                   7

#define CTL_WARNING_WIN_COLS                    40
#define CTL_WARNING_WIN_LINES                   7

#define CTL_MAKE_CHOICE_WIN_COLS                40
#define CTL_MAKE_CHOICE_WIN_LINES               7

#define CTL_CONFIRM_WIN_COLS                    40
#define CTL_CONFIRM_WIN_LINES                   7

#define CTL_PANEL_CYAN                          1
#define CTL_PANEL_RED                           2
#define CTL_PANEL_YELLOW                        3
#define CTL_PANEL_GREEN                         4
#define CTL_PANEL_MAGENTA                       5
#define CTL_SET_COLOR(w, p)                     wattron(w, COLOR_PAIR(p))
#define CTL_RESET_COLOR(w, p)                   wattroff(w, COLOR_PAIR(p))

//Must invoke CTL_InitConsole before using CTL_*Exit related macro
#define CTL_ForceExit()                         {endwin();exit(0);}
#define CTL_ErrExit(format, arg...) \
        { \
            endwin(); \
            fprintf(stderr, format, ##arg); \
            exit(0); \
        }
        
#define CTL_HIDE_CONSOLE_END_LINE()             {mvhline(LINES-2, 2, ' ', COLS-3);refresh();}
#define CTL_SHOW_CONSOLE_END_LINE() \
        { \
            attron(A_REVERSE); \
            mvaddnstr(LINES-2, 2, STR_CONSOLE_END_LINE, COLS-3); \
            attroff(A_REVERSE); \
            refresh(); \
        }

typedef enum {
    CTL_MENU_SHOW_INSTRUCTION = 0,
    CTL_MENU_ENCRYPT,
    CTL_MENU_DECRYPT,
    CTL_MENU_CHANGE_LANGUAGE,
    CTL_MENU_MAX,
}CTL_MENU;

typedef struct FileContentStruct
{
    char *pContent;
    int ContentWidth;
    int ContentLines;
    char flag; //If ContentWidth % COLS == 0, flag = 1;
    struct FileContentStruct *pPrev;
    struct FileContentStruct *pNext;
}FileContent_t;

void CTL_InitConsole(void);
void CTL_DrawStdConsole(void);
void CTL_ChooseLanguage(void);
void CTL_ShowMenu(CTL_MENU *pFunc);
void CTL_ShowInstruction(void);
G_STATUS CTL_ShowFile(const char *pFileName);
G_STATUS CTL_GetFileName(char *pFileName);
G_STATUS CTL_GetPassord(char *pPassword);
G_STATUS CTL_DispWarning(const char *pFormat, ...);
G_STATUS CTL_MakeChoice(const char*format, ...);
G_STATUS CTL_ConfirmOperation(const char *pFormat, ...);



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Make a choice before exit project
 *  @Return: None
 *  @Note:   Must have been invoked CTL_InitConsole before invoking this function
 */
static inline void CTL_SafeExit(WINDOW *win)
{
    if(STAT_OK == CTL_ConfirmOperation(STR_EXIT_PROJECT))
    {
        endwin();
        exit(0);
    }
    touchwin(win);
}

#ifdef __WINDOWS
/*
 *  @Briefs: Fix bug #2.2
 *  @Return: STAT_ERR / STAT_OK
 *  @Note:   If pStr contain ' ', SpecialSymbolNum should increase 1 but it does not increase in this function
 */
static inline G_STATUS CTL_FixBug_IncompleteDisp(WINDOW *win, char *pStr, int SymbolNum)
{
    if(LAN_EN == g_LanguageFlag)
        return STAT_ERR;
    
    int SpecialSymbolNum = (0 != SymbolNum) ? (SymbolNum - 1) : (UTF8_GetSpecialSymbolNum(pStr) - 1);
    int i;
    
    for(i = 0; i < SpecialSymbolNum; i++)
    {
        waddch(win, ' ');
    }

    return STAT_OK;
}
#endif //#ifdef __WINDOWS


#endif
