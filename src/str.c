/*************************************************************************
	> File Name: control_str.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 18th,2017 Friday 08:51:45
 ************************************************************************/

#include <stdio.h>
#include "str.h"

char *g_StrTbl = NULL;
char *g_EnStrTbl[STR_MAX];
char *g_ChStrTbl[STR_MAX];

char *EnMenu[] = {
    "1. Instruction",
    "2. Encrypt",
    "3. Decrypt",
    NULL,
};

char *ChMenu;

char *EnInstruction[] = {
    "Please read it carefully",
    "1. This project supports Windows or Linux, and all functions are in developing.",
    "",
    "2. This project adopts \"non-storage\" way, i.e, don't save user's password.",
    "   It will generate the factor to encrypt the file based on password.",
    "   Decryption is same with encryption. Thus, arbitrary password can decrypt,",
    "   you will get corrupted files if password differs from that used to encrypt.",
    "   Note: it is unable to decrypte foever if the password is lost.",
    "",
    "3. To ensure data security, this program will generate decrypted files,",
    "   i.e, the encrypted file will not be deleted.",
#ifdef __LINUX    
    "   You can run \"clean.sh\" to remove all encrypted files.",
#elif defined __WINDOWS
    "   You can run \"clean.bat\" to remove all encrypted files.",
#endif
    "   Note: when decrypted, the source file would be coverd if exist.",
    "",
    "Tips:",
    "   It is not recommend that Encrypt multi files by running encrypt single files for many times, "
    "i.e, encrypt multi files using different passwords.",
    "   Note: it is easy to be confused and fogotten if there are lots of passwords.",
    "",
    "With any questions, please contact to lvsen46000@163.com",
    NULL,
};

char *ChInstruction;

void InitStr(void)
{
    g_EnStrTbl[STR_BACK                        ] = STR_EN_BACK;
    g_EnStrTbl[STR_CONSOLE_END_LINE            ] = STR_EN_CONSOLE_END_LINE;
    g_EnStrTbl[STR_CONSOLE_LABEL               ] = STR_EN_CONSOLE_LABEL;
    g_EnStrTbl[STR_CONTINUE                    ] = STR_EN_CONTINUE;
    g_EnStrTbl[STR_DISP_ERR_INFO               ] = STR_EN_DISP_ERR_INFO;
    g_EnStrTbl[STR_ESC_EXIT                    ] = STR_EN_ESC_EXIT;
    g_EnStrTbl[STR_EXIT_PROJECT                ] = STR_EN_EXIT_PROJECT;
    g_EnStrTbl[STR_FILE_IS_NULL                ] = STR_EN_FILE_IS_NULL;
    g_EnStrTbl[STR_FILE_LIST_IS_NULL           ] = STR_EN_FILE_LIST_IS_NULL;
    g_EnStrTbl[STR_FILE_NOT_EXIST              ] = STR_EN_FILE_NOT_EXIST;
    g_EnStrTbl[STR_FOPEN_ERR                   ] = STR_EN_FOPEN_ERR;
    g_EnStrTbl[STR_GO_BACK                     ] = STR_EN_GO_BACK;
    g_EnStrTbl[STR_IN_ENCRYPTING               ] = STR_EN_IN_ENCRYPTING;
    g_EnStrTbl[STR_IN_DECRYPTING               ] = STR_EN_IN_DECRYPTING;
    g_EnStrTbl[STR_INPUT                       ] = STR_EN_INPUT;
    g_EnStrTbl[STR_INPUT_FILE_NAME             ] = STR_EN_INPUT_FILE_NAME;
    g_EnStrTbl[STR_INPUT_FILE_NAME_EG          ] = STR_EN_INPUT_FILE_NAME_EG;
    g_EnStrTbl[STR_INPUT_PASSWORD              ] = STR_EN_INPUT_PASSWORD;
    g_EnStrTbl[STR_INPUT_PASSWORD_CONFIRM      ] = STR_EN_INPUT_PASSWORD_CONFIRM;
    g_EnStrTbl[STR_LOG_IS_NULL                 ] = STR_EN_LOG_IS_NULL;
    g_EnStrTbl[STR_MENU                        ] = STR_EN_MENU;
    g_EnStrTbl[STR_NULL                        ] = STR_EN_NULL;
    g_EnStrTbl[STR_PASSWORD_NOT_MATCH          ] = STR_EN_PASSWORD_NOT_MATCH;
    g_EnStrTbl[STR_PASSWORD_NULL               ] = STR_EN_PASSWORD_NULL;
    g_EnStrTbl[STR_PASSWORD_TOO_SHORT          ] = STR_EN_PASSWORD_TOO_SHORT;
    g_EnStrTbl[STR_PRESS_ENTER_TO_GO_BACK      ] = STR_EN_PRESS_ENTER_TO_GO_BACK;
    g_EnStrTbl[STR_RATE                        ] = STR_EN_RATE;
    g_EnStrTbl[STR_RESULT_WIN_LABEL            ] = STR_EN_RESULT_WIN_LABEL;
    g_EnStrTbl[STR_RETRY                       ] = STR_EN_RETRY;
    g_EnStrTbl[STR_SUCCESS                     ] = STR_EN_SUCCESS;
    g_EnStrTbl[STR_SUCCESS_COUNT               ] = STR_EN_SUCCESS_COUNT;
    g_EnStrTbl[STR_TASK                        ] = STR_EN_TASK;
    g_EnStrTbl[STR_VIEW_LOG                    ] = STR_EN_VIEW_LOG;
    g_EnStrTbl[STR_FAIL                        ] = STR_EN_FAIL;
    g_EnStrTbl[STR_FAIL_COUNT                  ] = STR_EN_FAIL_COUNT;
    g_EnStrTbl[STR_FAIL_TO_CREATE_FILE_LIST    ] = STR_EN_FAIL_TO_CREATE_FILE_LIST;
    g_EnStrTbl[STR_FAIL_TO_CREATE_OPEN_FILE    ] = STR_EN_FAIL_TO_CREATE_OPEN_FILE;
    g_EnStrTbl[STR_FAIL_TO_DELETE_OLD_FILE     ] = STR_EN_FAIL_TO_DELETE_OLD_FILE;
    g_EnStrTbl[STR_FAIL_TO_GET_FILE_FOLDER_INFO] = STR_EN_FAIL_TO_GET_FILE_FOLDER_INFO;
    g_EnStrTbl[STR_FAIL_TO_READ_FILE           ] = STR_EN_FAIL_TO_READ_FILE;
    g_EnStrTbl[STR_FAIL_TO_SCAN_DIRECTORY      ] = STR_EN_FAIL_TO_SCAN_DIRECTORY;
    g_EnStrTbl[STR_FAIL_TO_WRITE_FILE          ] = STR_EN_FAIL_TO_WRITE_FILE;
    g_EnStrTbl[STR_ERR_FAIL_TO_GET_FILE_INFO   ] = STR_EN_ERR_FAIL_TO_GET_FILE_INFO;
    g_EnStrTbl[STR_ERR_FAIL_TO_MALLOC          ] = STR_EN_ERR_FAIL_TO_MALLOC;
    g_EnStrTbl[STR_ERR_FAIL_TO_OPEN_LOG_FILE   ] = STR_EN_ERR_FAIL_TO_OPEN_LOG_FILE;
    g_EnStrTbl[STR_ERR_FAIL_TO_OPEN_FILE_LIST  ] = STR_EN_ERR_FAIL_TO_OPEN_FILE_LIST;
    g_EnStrTbl[STR_ERR_FILE_LIST_NOT_EXIST     ] = STR_EN_ERR_FILE_LIST_NOT_EXIST;
    g_EnStrTbl[STR_ERR_INVALID_COLS            ] = STR_EN_ERR_INVALID_COLS;
    g_EnStrTbl[STR_ERR_INVALID_FILE_LIST       ] = STR_EN_ERR_INVALID_FILE_LIST;
    g_EnStrTbl[STR_ERR_INVALID_FILE_LIST_ARG   ] = STR_EN_ERR_INVALID_FILE_LIST_ARG;
    g_EnStrTbl[STR_ERR_INVALID_FUNC            ] = STR_EN_ERR_INVALID_FUNC;
    g_EnStrTbl[STR_ERR_INVALID_LINES           ] = STR_EN_ERR_INVALID_LINES;
    g_EnStrTbl[STR_ERR_INVALID_LOG_FILE        ] = STR_EN_ERR_INVALID_LOG_FILE;
    g_EnStrTbl[STR_ERR_INVALID_PTHREAD_ARG     ] = STR_EN_ERR_INVALID_PTHREAD_ARG;
    g_EnStrTbl[STR_ERR_PTHREAD_CREATE          ] = STR_EN_ERR_PTHREAD_CREATE;
    g_EnStrTbl[STR_ERR_PTHREAD_JOIN            ] = STR_EN_ERR_PTHREAD_JOIN;
    g_EnStrTbl[STR_ERR_PTHREAD_NUM_TOO_BIG     ] = STR_EN_ERR_PTHREAD_NUM_TOO_BIG;
    g_EnStrTbl[STR_ERR_PTHREAD_NUM_TOO_SMALL   ] = STR_EN_ERR_PTHREAD_NUM_TOO_SMALL;
    g_EnStrTbl[STR_ERR_STR_IS_NULL             ] = STR_EN_ERR_STR_IS_NULL;
    g_EnStrTbl[STR_ERR_STR_TOO_SHORT           ] = STR_EN_ERR_STR_TOO_SHORT;
    
    g_ChStrTbl[STR_BACK                        ] = STR_CH_BACK;
    g_ChStrTbl[STR_CONSOLE_END_LINE            ] = STR_CH_CONSOLE_END_LINE;
    g_ChStrTbl[STR_CONSOLE_LABEL               ] = STR_CH_CONSOLE_LABEL;
    g_ChStrTbl[STR_CONTINUE                    ] = STR_CH_CONTINUE;
    g_ChStrTbl[STR_DISP_ERR_INFO               ] = STR_CH_DISP_ERR_INFO;
    g_ChStrTbl[STR_ESC_EXIT                    ] = STR_CH_ESC_EXIT;
    g_ChStrTbl[STR_EXIT_PROJECT                ] = STR_CH_EXIT_PROJECT;
    g_ChStrTbl[STR_FILE_IS_NULL                ] = STR_CH_FILE_IS_NULL;
    g_ChStrTbl[STR_FILE_LIST_IS_NULL           ] = STR_CH_FILE_LIST_IS_NULL;
    g_ChStrTbl[STR_FILE_NOT_EXIST              ] = STR_CH_FILE_NOT_EXIST;
    g_ChStrTbl[STR_FOPEN_ERR                   ] = STR_CH_FOPEN_ERR;
    g_ChStrTbl[STR_GO_BACK                     ] = STR_CH_GO_BACK;
    g_ChStrTbl[STR_IN_ENCRYPTING               ] = STR_CH_IN_ENCRYPTING;
    g_ChStrTbl[STR_IN_DECRYPTING               ] = STR_CH_IN_DECRYPTING;
    g_ChStrTbl[STR_INPUT                       ] = STR_CH_INPUT;
    g_ChStrTbl[STR_INPUT_FILE_NAME             ] = STR_CH_INPUT_FILE_NAME;
    g_ChStrTbl[STR_INPUT_FILE_NAME_EG          ] = STR_CH_INPUT_FILE_NAME_EG;
    g_ChStrTbl[STR_INPUT_PASSWORD              ] = STR_CH_INPUT_PASSWORD;
    g_ChStrTbl[STR_INPUT_PASSWORD_CONFIRM      ] = STR_CH_INPUT_PASSWORD_CONFIRM;
    g_ChStrTbl[STR_LOG_IS_NULL                 ] = STR_CH_LOG_IS_NULL;
    g_ChStrTbl[STR_MENU                        ] = STR_CH_MENU;
    g_ChStrTbl[STR_NULL                        ] = STR_CH_NULL;
    g_ChStrTbl[STR_PASSWORD_NOT_MATCH          ] = STR_CH_PASSWORD_NOT_MATCH;
    g_ChStrTbl[STR_PASSWORD_NULL               ] = STR_CH_PASSWORD_NULL;
    g_ChStrTbl[STR_PASSWORD_TOO_SHORT          ] = STR_CH_PASSWORD_TOO_SHORT;
    g_ChStrTbl[STR_PRESS_ENTER_TO_GO_BACK      ] = STR_CH_PRESS_ENTER_TO_GO_BACK;
    g_ChStrTbl[STR_RATE                        ] = STR_CH_RATE;
    g_ChStrTbl[STR_RESULT_WIN_LABEL            ] = STR_CH_RESULT_WIN_LABEL;
    g_ChStrTbl[STR_RETRY                       ] = STR_CH_RETRY;
    g_ChStrTbl[STR_SUCCESS                     ] = STR_CH_SUCCESS;
    g_ChStrTbl[STR_SUCCESS_COUNT               ] = STR_CH_SUCCESS_COUNT;
    g_ChStrTbl[STR_TASK                        ] = STR_CH_TASK;
    g_ChStrTbl[STR_VIEW_LOG                    ] = STR_CH_VIEW_LOG;
    g_ChStrTbl[STR_FAIL                        ] = STR_CH_FAIL;
    g_ChStrTbl[STR_FAIL_COUNT                  ] = STR_CH_FAIL_COUNT;
    g_ChStrTbl[STR_FAIL_TO_CREATE_FILE_LIST    ] = STR_CH_FAIL_TO_CREATE_FILE_LIST;
    g_ChStrTbl[STR_FAIL_TO_CREATE_OPEN_FILE    ] = STR_CH_FAIL_TO_CREATE_OPEN_FILE;
    g_ChStrTbl[STR_FAIL_TO_DELETE_OLD_FILE     ] = STR_CH_FAIL_TO_DELETE_OLD_FILE;
    g_ChStrTbl[STR_FAIL_TO_GET_FILE_FOLDER_INFO] = STR_CH_FAIL_TO_GET_FILE_FOLDER_INFO;
    g_ChStrTbl[STR_FAIL_TO_READ_FILE           ] = STR_CH_FAIL_TO_READ_FILE;
    g_ChStrTbl[STR_FAIL_TO_SCAN_DIRECTORY      ] = STR_CH_FAIL_TO_SCAN_DIRECTORY;
    g_ChStrTbl[STR_FAIL_TO_WRITE_FILE          ] = STR_CH_FAIL_TO_WRITE_FILE;
    g_ChStrTbl[STR_ERR_FAIL_TO_GET_FILE_INFO   ] = STR_CH_ERR_FAIL_TO_GET_FILE_INFO;
    g_ChStrTbl[STR_ERR_FAIL_TO_MALLOC          ] = STR_CH_ERR_FAIL_TO_MALLOC;
    g_ChStrTbl[STR_ERR_FAIL_TO_OPEN_LOG_FILE   ] = STR_CH_ERR_FAIL_TO_OPEN_LOG_FILE;
    g_ChStrTbl[STR_ERR_FAIL_TO_OPEN_FILE_LIST  ] = STR_CH_ERR_FAIL_TO_OPEN_FILE_LIST;
    g_ChStrTbl[STR_ERR_FILE_LIST_NOT_EXIST     ] = STR_CH_ERR_FILE_LIST_NOT_EXIST;
    g_ChStrTbl[STR_ERR_INVALID_COLS            ] = STR_CH_ERR_INVALID_COLS;
    g_ChStrTbl[STR_ERR_INVALID_FILE_LIST       ] = STR_CH_ERR_INVALID_FILE_LIST;
    g_ChStrTbl[STR_ERR_INVALID_FILE_LIST_ARG   ] = STR_CH_ERR_INVALID_FILE_LIST_ARG;
    g_ChStrTbl[STR_ERR_INVALID_FUNC            ] = STR_CH_ERR_INVALID_FUNC;
    g_ChStrTbl[STR_ERR_INVALID_LINES           ] = STR_CH_ERR_INVALID_LINES;
    g_ChStrTbl[STR_ERR_INVALID_LOG_FILE        ] = STR_CH_ERR_INVALID_LOG_FILE;
    g_ChStrTbl[STR_ERR_INVALID_PTHREAD_ARG     ] = STR_CH_ERR_INVALID_PTHREAD_ARG;
    g_ChStrTbl[STR_ERR_PTHREAD_CREATE          ] = STR_CH_ERR_PTHREAD_CREATE;
    g_ChStrTbl[STR_ERR_PTHREAD_JOIN            ] = STR_CH_ERR_PTHREAD_JOIN;
    g_ChStrTbl[STR_ERR_PTHREAD_NUM_TOO_BIG     ] = STR_CH_ERR_PTHREAD_NUM_TOO_BIG;
    g_ChStrTbl[STR_ERR_PTHREAD_NUM_TOO_SMALL   ] = STR_CH_ERR_PTHREAD_NUM_TOO_SMALL;
    g_ChStrTbl[STR_ERR_STR_IS_NULL             ] = STR_CH_ERR_STR_IS_NULL;
    g_ChStrTbl[STR_ERR_STR_TOO_SHORT           ] = STR_CH_ERR_STR_TOO_SHORT;
}
