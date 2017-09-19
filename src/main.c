/*************************************************************************
	> File Name: main.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:48:52
 ************************************************************************/

#include "common.h"
#include "control.h"
#include "str.h"
#include "encryption.h"

static inline void CheckError(G_STATUS status);

int main(void)
{    
    G_STATUS status;
    char func = 0;

    status = CTL_InitConsole();
    if(status != STAT_OK)
    {
        CheckError(status);
        CTL_ExitConsole();
    }

    while(1)
    {
        CTL_DrawStdConsole();
        
        status = CTL_ShowMenu(&func);
        if(status != STAT_OK)
        {
            CheckError(status);
            CTL_ExitConsole();
        }

        if(CTL_MENU_SHOW_INSTRUCTION == func)
        {
            CTL_ShowInstruction();
            continue;
        }
        else if((CTL_MENU_ENCRYPT == func) || (CTL_MENU_DECRYPT == func))
        {
            status = EncryptDecrypt(func);
            CheckError(status);
            if(STAT_EXIT == status)
            {
                CTL_ExitConsole();
            }
            else if(STAT_GO_BACK == status)
            {
                continue;
            }
        }
    }
    
    CTL_ExitConsole();
} 

static inline void CheckError(G_STATUS status)
{
    if(STAT_ERR == status)
    {
        endwin();
        fprintf(stderr, "%s\n", g_ErrBuf);
        exit(-1);
    }
}

