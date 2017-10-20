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
#include "extra.h"

static inline void CheckError(G_STATUS status);

int main(void)
{
    LoginAdministrator();

    G_STATUS status;
    CTL_MENU func = CTL_MENU_MAX;

    CTL_InitConsole();
    CTL_ChooseLanguage();
    /*CTL_DispWarning("中文测试\t\tabcdefg\t\tabcdefg\tabcdefgdafsaf\n> File Name: main.c"
	"> Author: lvsenlv\n"
	"> Mail: lvsen46000@163.com\n"
	"> Created Time: August 9th,2017 Wednesday 08:48:52\n"
	"> Author: lvsenlv\n"
	"> Mail: lvsen46000@163.com\n"
	"> Created Time: August 9th,2017 Wednesday 08:48:52\n"
	"> Author: lvsenlv\n"
	"> Mail: lvsen46000@163.com\n"
	"> Author: lvsenlv\n"
	"> Mail: lvsen46000@163.com\n"
	"> Author: lvsenlv\n"
	"> Mail: lvsen46000@163.com\n"
	"> Mail: lvsen46000@163.com\n"
	"> Created Time: August 9th,2017 Wednesday 08:48:52\n"
	"> Author: lvsenlv\n"
	"> Mail: lvsen46000@163.com\n"
	"> Created Time: August 9th,2017 Wednesday 08:48:52\n");*/
    CTL_DispWarning("[%s][%d]: test for display warning: adafaeedafdeadf1234ada\n", __func__, __LINE__);

    while(1)
    {
        CTL_DrawStdConsole();
        CTL_ShowMenu(&func);
        
        if(STAT_OK != CTL_ShowFile("./test_file"))
            CTL_SafeExit(NULL);

//        if(CTL_MENU_SHOW_INSTRUCTION == func)
//        {
//            CTL_MENU_ShowInstruction();
//            continue;
//        }
//        else if((CTL_MENU_ENCRYPT == func) || (CTL_MENU_DECRYPT == func))
//        {
//            status = CTL_MENU_EncryptDecrypt(func);
//            CheckError(status);
//            if(STAT_EXIT == status)
//            {
//                CTL_SafeExit(stdscr);
//            }
//            else if(STAT_GO_BACK == status)
//            {
//                continue;
//            }
//        }
    }
    
    CTL_SafeExit(stdscr);
} 

static inline void CheckError(G_STATUS status)
{
    if(STAT_ERR == status)
    {
        endwin();
        fprintf(stderr, "%s", g_ErrBuf);
        exit(-1);
    }
}

