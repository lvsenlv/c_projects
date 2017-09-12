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
#include <curses.h>
#elif defined __WINDOWS
#include "curses_win.h"
#endif
#include "str.h"

typedef enum {
    CTL_MENU_SHOW_INSTRUCTION = 1,
    CTL_MENU_ENCRYPT,
    CTL_MENU_DECRYPT,
    CTL_MENU_COUNT,
}CTL_MENU;

#define CTL_PASSWORD_LENGHT_MAX             19 //real is 19-1=18
#define CTL_PASSWORD_LENGHT_MIN             8
#define CTL_FILE_NAME_LENGHT                256

#define CTL_CONSOLE_COLS                    80
#define CTL_CONSOLE_LINES                   24
#define CTL_MENU_WIN_COLS                   20
#define CTL_MENU_WIN_LINES                  (CTL_MENU_COUNT+3)
#define CTL_GET_FILE_NAME_WIN_COLS          (sizeof(STR_INPUT_FILE_NAME)-1)
#define CTL_GET_FILE_NAME_WIN_LINES         7
#define CTL_GET_PASSWORD_WIN_COLS           (sizeof(STR_INPUT_PASSWORD_CONFIRM)-1+CTL_PASSWORD_LENGHT_MAX)
#define CTL_GET_PASSWORD_WIN_LINES          6
#define CTL_ENCRYPT_FILE_WIN_COLS           60
#define CTL_DECRYPT_FILE_WIN_COLS           60

#define CTL_PANEL_CYAN                      1
#define CTL_PANEL_RED                       2
#define CTL_PANEL_YELLOW                    3
#define CTL_PANEL_GREEN                     4
#define CTL_SET_WIN_COLOR(w, p)             wattron(w, COLOR_PAIR(p))
#define CTL_RESET_WIN_COLOR(w, p)           wattroff(w, COLOR_PAIR(p))

#ifdef __LINUX
#define CLEAR_STR_SCR()                     system("clear")
#elif defined __WINDOWS
#define CLEAR_STR_SCR()                     system("cls")
#endif
#define CTL_ExitConsole()                   {endwin();exit(0);}
#define CTL_HIDE_CONSOLE_END_LINE()         {mvhline(LINES-2, 2, ' ', COLS-3);refresh();}
#define CTL_SHOW_CONSOLE_END_LINE() \
        { \
            attron(A_REVERSE); \
            mvaddnstr(LINES-2, 2, STR_CONSOLE_END_LINE, COLS-3); \
            attroff(A_REVERSE); \
            refresh(); \
        }


#if defined __WINDOWS
enum 
{
    false,
    true,
};
#endif

typedef struct PtrLinkList
{
    char *ptr;
    struct PtrLinkList *pNext_t;
}PtrLinkList_t;

static inline void FreePtrLinkList(PtrLinkList_t *pHeadNode)
{
    PtrLinkList_t *pCurNode = pHeadNode->pNext_t;
    PtrLinkList_t *pTmpNode;
    while(pCurNode != NULL)
    {
        pTmpNode = pCurNode->pNext_t;
        free(pCurNode);
        pCurNode = pTmpNode;
    }
}

G_STATUS CTL_InitConsole(void);
G_STATUS CTL_ShowMenu(char *pFunc);
void CTL_ShowInstruction(void);
G_STATUS CTL_GetFileName(char *pFileName);
G_STATUS CTL_MakeChoice(const char*format, ...);

G_STATUS CTL_GetPassord(char *pPassword);

#endif
