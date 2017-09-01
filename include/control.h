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
#include <ncurses.h>
#elif defined __WINDOWS
#include "curses_win.h"
#endif

typedef enum {
    CTL_MENU_SHOW_INSTRUCTION = 1,
    CTL_MENU_ENCRYPT,
    CTL_MENU_DECRYPT,
    CTL_MENU_COUNT,
}CTL_MENU;


#define CLEAR_STR_SCR()                     system("clear");

#define CTL_CONSOLE_WIDTH                   80
#define CTL_CONSOLE_HEIGHT                  24
#define CTL_MENU_WIN_COLS                   20
#define CTL_MENU_WIN_LINES                  (CTL_MENU_COUNT+3)

#define CTL_ExitConsole()                   {endwin();exit(0);}
#define CTL_PASSWORD_LENGHT_MAX             19
#define CTL_PASSWORD_LENGHT_MIN             8
#define CTL_FILE_NAME_LENGHT                128

#if defined __WINDOWS
enum 
{
    false,
    true,
};
#endif

G_STATUS CTL_InitConsole(void);
G_STATUS CTL_ChooseFunc(char *pFunc);
void CTL_ShowInstruction(void);

G_STATUS CTL_ChooseLanguage(void);
G_STATUS CTL_GetFileName(char *pFileName);
G_STATUS CTL_GetPassord(char *pPassword);
G_STATUS CTL_MakeChoice(int lines, int cols, char *pStr);

#endif
