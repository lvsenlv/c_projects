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

#define CTL_ExitConsole()                   {endwin();exit(0);}
#define CTL_PASSWORD_LENGHT_MAX             19
#define CTL_PASSWORD_LENGHT_MIN             8
#define CTL_FILE_NAME_LENGHT                128

extern char **g_pStr;
extern char g_FlagLanguage;

typedef enum
{
    CTL_MENU_INSTRUCTION = 1,
    CTL_MENU_ENCRYPT,
    CTL_MENU_DECRYPT,
    CTL_MENU_CHOOSE_LANGUAGE,
    CTL_MENU_FUNC_NUM,
}CTL_MENU_FUNC;

#if defined __WINDOWS
enum 
{
    false,
    true,
};
#endif

void CTL_InitConsole(void);
void CTL_DrawStdScreen(void);
void CTL_ShowInstruction(void);

G_STATUS CTL_ChooseLanguage(void);
G_STATUS CTL_ChooseFunc(char *pFunc);
G_STATUS CTL_GetFileName(char *pFileName);
G_STATUS CTL_GetPassord(char *pPassword);
G_STATUS CTL_MakeChoice(WINDOW *pWin, int lines, int cols, char *pStr);

#endif
