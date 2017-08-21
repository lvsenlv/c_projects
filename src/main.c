/*************************************************************************
	> File Name: main.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年08月09日 星期三 08时48分52秒
 ************************************************************************/

#include "common.h"
#include "control.h"
#include "encryption.h"

int main(void)
{
    G_STATUS status;
    CTL_InitConsole();
    status = CTL_ChooseLanguage();
    if(STAT_EXIT == status)
    {
        CTL_ExitConsole();
        exit(0);
    }
    else if(STAT_ERR == status)
    {
        CTL_ExitConsole();
        exit(-1);
    }
    
    CTL_DrawStdScreen();

    char func = 0;    
    while(1)
    {
        status = CTL_ChooseFunc(&func);
        if(STAT_EXIT == status)
        {
            CTL_ExitConsole();
            exit(0);
        }
        else if(STAT_ERR == status)
        {
            CTL_ExitConsole();
            exit(-1);
        }

        if(CTL_MENU_FUNC_INSTRUCTION == func)
        {
            status = CTL_ShowInstruction();
            if(STAT_EXIT == status)
            {
                CTL_ExitConsole();
                exit(0);
            }
            else if(STAT_ERR == status)
            {
                CTL_ExitConsole();
                exit(-1);
            }
            continue;
        }
        else if((func >= CTL_MENU_FUNC_ENCRYPT_FILE) && (func <= CTL_MENU_FUNC_DECRYPT_FOLDER))
        {
            status = encrypt(func);
            if(STAT_EXIT == status)
            {
                CTL_ExitConsole();
                exit(0);
            }
            else if(STAT_ERR == status)
            {
                CTL_ExitConsole();
                exit(-1);
            }
            else if(STAT_GO_BACK == status)
                continue;
        }
        else if(CTL_MENU_FUNC_CHOOSE_LANGUAGE == func)
        {
            status = CTL_ChooseLanguage();
            if(STAT_EXIT == status)
            {
                CTL_ExitConsole();
                exit(0);
            }
            else if(STAT_ERR == status)
            {
                CTL_ExitConsole();
                exit(-1);
            }
            CTL_DrawStdScreen();
            continue;
        }        
        
    }

    while(1);
    CTL_ExitConsole();
    return 0;
}


