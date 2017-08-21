/*************************************************************************
	> File Name: control_str.h
	> Author: 
	> Mail: 
	> Created Time: 2017年08月10日 星期四 13时42分50秒
 ************************************************************************/

#ifndef __CONTROL_STR_H
#define __CONTROL_STR_H

#define CTL_STR_CHOOSE_LANG_ENGLISH         "English"
#define CTL_STR_CHOOSE_LANG_CHINESE         "中文"
#define CTL_STR_NULL                        "                            "

typedef enum CTL_STR_INFO
{
    CTL_STR_ESC_EXIT = 0,
    CTL_STR_LABEL,
    CTL_STR_END_LINE,
    CTL_STR_MENU_FUNC1,
    CTL_STR_MENU_FUNC2,
    CTL_STR_MENU_FUNC3,
    CTL_STR_MENU_FUNC4,
    CTL_STR_MENU_FUNC5,
    CTL_STR_MENU_FUNC6,
    CTL_STR_INPUT_FILE_NAME,
    CTL_STR_INPUT_FILE_NAME_EG,
    CTL_STR_INPUT,
    CTL_STR_RETRY,
    CTL_STR_BACK,
    CTL_STR_FILE_NOT_EXIST,
    CTL_STR_INPUT_PASSWORD,
    CTL_STR_PASSWORD_TOO_SHORT,
    CTL_STR_INPUT_PASSWORD_CONFIRM,
    CTL_STR_PASSWORD_NOT_MATCH,
    CTL_STR_ENTER_KEY_GO_BACK,
    CTL_STR_IN_ENCRYPTING,    
    CTL_STR_IN_DECRYPTING,
    CTL_STR_ANY_KEY,
    CTL_STR_SUCCESS,
    CTL_STR_ERR_GET_FILE_INFO_ERR,
    CTL_STR_ERR_FOPEN_ERR,
    CTL_STR_ERR_MALLOC_ERR,
    CTL_STR_ERR_FAIL_TO_READ_FILE,
    CTL_STR_ERR_PASSWORD_NULL,
    CTL_STR_ERR_FAIL_TO_CREATE_OPEN_FILE,
    CTL_STR_ERR_FAIL_TO_WRITE_FILE,
    CTL_STR_ERR_FAIL_TO_DELETE_OLD_FILE,
}CTL_STR;


extern char *pEnStr[];
extern char *pChStr[];
extern char *pEnInstruction[];
extern char *pChInstruction[];


#endif
