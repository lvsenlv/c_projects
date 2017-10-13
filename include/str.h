/*************************************************************************
	> File Name: str.h
	> Author: 
	> Mail: 
	> Created Time: August 10th,2017 Thursday 13:42:50
 ************************************************************************/

#ifndef __STR_H
#define __STR_H

/*************************************************************************
                                 English
 ************************************************************************/
#define STR_EN_BACK                             "[ back ]"
#define STR_EN_CONSOLE_END_LINE                 "Press Esc to exit project | Version 1.0.0 | Copyright 2017-2017"
#define STR_EN_CONSOLE_LABEL                    "Encryption System"
#define STR_EN_CONTINUE                         "[ continue ]"
#define STR_EN_DISP_ERR_INFO                    "Error Info"
#define STR_EN_ESC_EXIT                         "Press Esc to exit project"
#define STR_EN_EXIT_PROJECT                     "Continue to exit project ?"
#define STR_EN_FILE_IS_NULL                     "It is an empty file"
#define STR_EN_FILE_LIST_IS_NULL                "Folder is empty"
#define STR_EN_FILE_NOT_EXIST                   "The file or directory does not exist"
#define STR_EN_FOPEN_ERR                        "Fail to open file"
#define STR_EN_GO_BACK                          "[ go back ]"
#define STR_EN_IN_ENCRYPTING                    "Encrypting: "
#define STR_EN_IN_DECRYPTING                    "Decrypting: "
#define STR_EN_INPUT                            "Input: "
#define STR_EN_INPUT_FILE_NAME                  "Please input the file or folder name with full path"
#ifdef __LINUX
#define STR_EN_INPUT_FILE_NAME_EG               "E.g, /root/document/..."
#elif defined __WINDOWS
#define STR_EN_INPUT_FILE_NAME_EG               "E.g, C:\User\Administrator\..."
#endif 
#define STR_EN_INPUT_PASSWORD                   "Password: "
#define STR_EN_INPUT_PASSWORD_CONFIRM           "Confirm password: "
#define STR_EN_LOG_IS_NULL                      "Log is empty"
#define STR_EN_MENU                             "Menu"
#define STR_EN_NULL                             "NULL"
#define STR_EN_PASSWORD_NOT_MATCH               "Two password do not match"
#define STR_EN_PASSWORD_NULL                    "Length of key could not be null"
#define STR_EN_PASSWORD_TOO_SHORT               "Password too short, at least 8 bits"
#define STR_EN_PRESS_ENTER_TO_GO_BACK           "[ press Enter to go back ]"
#define STR_EN_RATE                             "Rate: "
#define STR_EN_RESULT_WIN_LABEL                 "Pgdn: next    Pgup: last    Enter: go back    Esc: exit    "
#define STR_EN_RETRY                            "[ retry ]"
#define STR_EN_SUCCESS                          "[ success ]"
#define STR_EN_SUCCESS_COUNT                    "Success: "
#define STR_EN_TASK                             "Task"
#define STR_EN_VIEW_LOG                         "[ view log ]"

#define STR_EN_FAIL                             "[ fail ]"
#define STR_EN_FAIL_COUNT                       "Fail: "
#define STR_EN_FAIL_TO_CREATE_FILE_LIST         "Fail to create file_lists"
#define STR_EN_FAIL_TO_CREATE_OPEN_FILE         "Fail to create or open file"
#define STR_EN_FAIL_TO_DELETE_OLD_FILE          "Fail to delete old file"
#define STR_EN_FAIL_TO_GET_FILE_FOLDER_INFO     "Fail to get file or folder info"
#define STR_EN_FAIL_TO_READ_FILE                "Fail to read file, make sure it is readable"
#define STR_EN_FAIL_TO_SCAN_DIRECTORY           "Fail to scan directory"
#define STR_EN_FAIL_TO_WRITE_FILE               "Fail to write to file, make sure it is writeable"

#define STR_EN_ERR_FAIL_TO_GET_FILE_INFO        "Error: fail to get file info"
#define STR_EN_ERR_FAIL_TO_MALLOC               "Error: not enough running memory space"
#define STR_EN_ERR_FAIL_TO_OPEN_LOG_FILE        "Error: fail to open log file"
#define STR_EN_ERR_FAIL_TO_OPEN_FILE_LIST       "Error: fail to open file_list"
#define STR_EN_ERR_FILE_LIST_NOT_EXIST          "Error: file_list does not exist"
#define STR_EN_ERR_INVALID_COLS                 "Error: need wider console, at least: "
#define STR_EN_ERR_INVALID_FILE_LIST            "Error: invalid FlieList"
#define STR_EN_ERR_INVALID_FILE_LIST_ARG        "Error: invalid FlieList arguments"
#define STR_EN_ERR_INVALID_FUNC                 "Error: invalid func"
#define STR_EN_ERR_INVALID_LINES                "Error: need higher console, at least: "
#define STR_EN_ERR_INVALID_LOG_FILE             "Error: invalid log file format"
#define STR_EN_ERR_INVALID_PTHREAD_ARG          "Error: invalid pthread arguments"
#define STR_EN_ERR_PTHREAD_CREATE               "Error: in pthread_create: "
#define STR_EN_ERR_PTHREAD_JOIN                 "Error: in pthread_join: "
#define STR_EN_ERR_PTHREAD_NUM_TOO_BIG          "Error: PTHREAD_NUM_MAX must be equare to or less than 8"
#define STR_EN_ERR_PTHREAD_NUM_TOO_SMALL        "Error: PTHREAD_NUM_MAX must be equare to or bigger than 2"
#define STR_EN_ERR_STR_IS_NULL                  "Error: string is null in CTL_MakeChoice: "
#define STR_EN_ERR_STR_TOO_SHORT                "Error: string is too short in CTL_MakeChoice: "



/*************************************************************************
                                 Chinese
 ************************************************************************/
#define STR_CH_BACK                             "[ back ]"
#define STR_CH_CONSOLE_END_LINE                 "Press Esc to exit project | Version 1.0.0 | Copyright 2017-2017"
#define STR_CH_CONSOLE_LABEL                    "Encryption System"
#define STR_CH_CONTINUE                         "[ continue ]"
#define STR_CH_DISP_ERR_INFO                    "Error Info"
#define STR_CH_ESC_EXIT                         "Press Esc to exit project"
#define STR_CH_EXIT_PROJECT                     "Continue to exit project ?"
#define STR_CH_FILE_IS_NULL                     "It is an empty file"
#define STR_CH_FILE_LIST_IS_NULL                "Folder is empty"
#define STR_CH_FILE_NOT_EXIST                   "The file or directory does not exist"
#define STR_CH_FOPEN_ERR                        "Fail to open file"
#define STR_CH_GO_BACK                          "[ go back ]"
#define STR_CH_IN_ENCRYPTING                    "Encrypting: "
#define STR_CH_IN_DECRYPTING                    "Decrypting: "
#define STR_CH_INPUT                            "Input: "
#define STR_CH_INPUT_FILE_NAME                  "Please input the file or folder name with full path"
#ifdef __LINUX
#define STR_CH_INPUT_FILE_NAME_EG               "E.g, /root/document/..."
#elif defined __WINDOWS
#define STR_CH_INPUT_FILE_NAME_EG               "E.g, C:\User\Administrator\..."
#endif 
#define STR_CH_INPUT_PASSWORD                   "Password: "
#define STR_CH_INPUT_PASSWORD_CONFIRM           "Confirm password: "
#define STR_CH_LOG_IS_NULL                      "Log is empty"
#define STR_CH_MENU                             "Menu"
#define STR_CH_NULL                             "NULL"
#define STR_CH_PASSWORD_NOT_MATCH               "Two password do not match"
#define STR_CH_PASSWORD_NULL                    "Length of key could not be null"
#define STR_CH_PASSWORD_TOO_SHORT               "Password too short, at least 8 bits"
#define STR_CH_PRESS_ENTER_TO_GO_BACK           "[ press Enter to go back ]"
#define STR_CH_RATE                             "Rate: "
#define STR_CH_RESULT_WIN_LABEL                 "Pgdn: next    Pgup: last    Enter: go back    Esc: exit    "
#define STR_CH_RETRY                            "[ retry ]"
#define STR_CH_SUCCESS                          "[ success ]"
#define STR_CH_SUCCESS_COUNT                    "Success: "
#define STR_CH_TASK                             "Task"
#define STR_CH_VIEW_LOG                         "[ view log ]"
    
#define STR_CH_FAIL                             "[ fail ]"
#define STR_CH_FAIL_COUNT                       "Fail: "
#define STR_CH_FAIL_TO_CREATE_FILE_LIST         "Fail to create file_lists"
#define STR_CH_FAIL_TO_CREATE_OPEN_FILE         "Fail to create or open file"
#define STR_CH_FAIL_TO_DELETE_OLD_FILE          "Fail to delete old file"
#define STR_CH_FAIL_TO_GET_FILE_FOLDER_INFO     "Fail to get file or folder info"
#define STR_CH_FAIL_TO_READ_FILE                "Fail to read file, make sure it is readable"
#define STR_CH_FAIL_TO_SCAN_DIRECTORY           "Fail to scan directory"
#define STR_CH_FAIL_TO_WRITE_FILE               "Fail to write to file, make sure it is writeable"
    
#define STR_CH_ERR_FAIL_TO_GET_FILE_INFO        "Error: fail to get file info"
#define STR_CH_ERR_FAIL_TO_MALLOC               "Error: not enough running memory space"
#define STR_CH_ERR_FAIL_TO_OPEN_LOG_FILE        "Error: fail to open log file"
#define STR_CH_ERR_FAIL_TO_OPEN_FILE_LIST       "Error: fail to open file_list"
#define STR_CH_ERR_FILE_LIST_NOT_EXIST          "Error: file_list does not exist"
#define STR_CH_ERR_INVALID_COLS                 "Error: need wider console, at least: "
#define STR_CH_ERR_INVALID_FILE_LIST            "Error: invalid FlieList"
#define STR_CH_ERR_INVALID_FILE_LIST_ARG        "Error: invalid FlieList arguments"
#define STR_CH_ERR_INVALID_FUNC                 "Error: invalid func"
#define STR_CH_ERR_INVALID_LINES                "Error: need higher console, at least: "
#define STR_CH_ERR_INVALID_LOG_FILE             "Error: invalid log file format"
#define STR_CH_ERR_INVALID_PTHREAD_ARG          "Error: invalid pthread arguments"
#define STR_CH_ERR_PTHREAD_CREATE               "Error: in pthread_create: "
#define STR_CH_ERR_PTHREAD_JOIN                 "Error: in pthread_join: "
#define STR_CH_ERR_PTHREAD_NUM_TOO_BIG          "Error: PTHREAD_NUM_MAX must be equare to or less than 8"
#define STR_CH_ERR_PTHREAD_NUM_TOO_SMALL        "Error: PTHREAD_NUM_MAX must be equare to or bigger than 2"
#define STR_CH_ERR_STR_IS_NULL                  "Error: string is null in CTL_MakeChoice: "
#define STR_CH_ERR_STR_TOO_SHORT                "Error: string is too short in CTL_MakeChoice: "

extern char *g_StrTbl;
extern char *g_EnStrTbl[];
extern char *g_ChStrTbl[];

typedef enum
{
    STR_MENU_LIST = 0,
    STR_INSTRUCTION,
    
    STR_BACK,
    STR_CONSOLE_END_LINE,
    STR_CONSOLE_LABEL,
    STR_CONTINUE,
    STR_DISP_ERR_INFO,
    STR_ESC_EXIT,
    STR_EXIT_PROJECT,
    STR_FILE_IS_NULL,
    STR_FILE_LIST_IS_NULL,
    STR_FILE_NOT_EXIST,
    STR_FOPEN_ERR,
    STR_GO_BACK,
    STR_IN_ENCRYPTING,
    STR_IN_DECRYPTING,
    STR_INPUT,
    STR_INPUT_FILE_NAME,
    STR_INPUT_FILE_NAME_EG,
    STR_INPUT_PASSWORD,
    STR_INPUT_PASSWORD_CONFIRM,
    STR_LOG_IS_NULL,
    STR_MENU,
    STR_NULL,
    STR_PASSWORD_NOT_MATCH,
    STR_PASSWORD_NULL,
    STR_PASSWORD_TOO_SHORT,
    STR_PRESS_ENTER_TO_GO_BACK,
    STR_RATE,
    STR_RESULT_WIN_LABEL,
    STR_RETRY,
    STR_SUCCESS,
    STR_SUCCESS_COUNT,
    STR_TASK,
    STR_VIEW_LOG,
    
    STR_FAIL,
    STR_FAIL_COUNT,
    STR_FAIL_TO_CREATE_FILE_LIST,
    STR_FAIL_TO_CREATE_OPEN_FILE,
    STR_FAIL_TO_DELETE_OLD_FILE,
    STR_FAIL_TO_GET_FILE_FOLDER_INFO,
    STR_FAIL_TO_READ_FILE,
    STR_FAIL_TO_SCAN_DIRECTORY,
    STR_FAIL_TO_WRITE_FILE,
    
    STR_ERR_FAIL_TO_GET_FILE_INFO,
    STR_ERR_FAIL_TO_MALLOC,
    STR_ERR_FAIL_TO_OPEN_LOG_FILE,
    STR_ERR_FAIL_TO_OPEN_FILE_LIST,
    STR_ERR_FILE_LIST_NOT_EXIST,
    STR_ERR_INVALID_COLS,
    STR_ERR_INVALID_FILE_LIST,
    STR_ERR_INVALID_FILE_LIST_ARG,
    STR_ERR_INVALID_FUNC,
    STR_ERR_INVALID_LINES,
    STR_ERR_INVALID_LOG_FILE,
    STR_ERR_INVALID_PTHREAD_ARG,
    STR_ERR_PTHREAD_CREATE,
    STR_ERR_PTHREAD_JOIN,
    STR_ERR_PTHREAD_NUM_TOO_BIG,
    STR_ERR_PTHREAD_NUM_TOO_SMALL,
    STR_ERR_STR_IS_NULL,
    STR_ERR_STR_TOO_SHORT,
    
    STR_MAX,
}STR;

void InitStr(void);

#endif
