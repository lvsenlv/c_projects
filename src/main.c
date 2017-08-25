/*************************************************************************
	> File Name: main.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:48:52
 ************************************************************************/

#include "common.h"
#include "control.h"
#include "encryption.h"

int main(void)
{
    G_STATUS status;
    
    CTL_InitConsole();
    
    status = CTL_ChooseLanguage();
    if(status != STAT_OK)
        CTL_ExitConsole();
    
    CTL_DrawStdScreen();

    char func = 0;
    while(1)
    {
        status = CTL_ChooseFunc(&func);
        if(status != STAT_OK)
            CTL_ExitConsole();

        if(CTL_MENU_INSTRUCTION == func)
        {
            CTL_ShowInstruction();
            continue;
        }
        else if((func == CTL_MENU_ENCRYPT) || (func == CTL_MENU_DECRYPT))
        {
            status = encrypt(func);
            if(STAT_GO_BACK == status)
                continue;
            else
                CTL_ExitConsole();
        }
        else if(CTL_MENU_CHOOSE_LANGUAGE == func)
        {
            status = CTL_ChooseLanguage();
            if(status != STAT_OK)
            {
                CTL_ExitConsole();
                exit(0);
            }
            CTL_DrawStdScreen();
            continue;
        }        
        
    }

    while(1);
    CTL_ExitConsole();
    return 0;
}


