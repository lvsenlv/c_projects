/*************************************************************************
	> File Name: str.h
	> Author: 
	> Mail: 
	> Created Time: August 10th,2017 Thursday 13:42:50
 ************************************************************************/

#ifndef __STR_H
#define __STR_H

#define STR_CONSOLE_END_LINE                "Press Esc to exit project | Version 1.0.0 | Copyright 2017-2017"
#define STR_CONSOLE_LABEL                   "Encryption System"
#define STR_ESC_EXIT                        "Press Esc to exit project"
#define STR_MENU                            "Menu"
#define STR_PRESS_ENTER_TO_GO_BACK          "[ press Enter to go back ]"
#define STR_INPUT_FILE_NAME                 "Please input the file or folder name with full path"
#ifdef __LINUX
#define STR_INPUT_FILE_NAME_EG              "E.g, /root/document/..."
#elif defined __WINDOWS
#define STR_INPUT_FILE_NAME_EG              "E.g, C:/User/Administrator/..."
#endif 
#define STR_INPUT                           "Input: "
#define STR_RETRY                           "[ retry ]"
#define STR_BACK                            "[ back ]"
#define STR_FILE_NOT_EXIST                  "The file or directory does not exist"
#define STR_DISP_ERR_INFO                   "Error Info"
#define STR_FAIL_TO_GET_FILE_FOLDER_INFO    "Fail to get file or folder info"
#define STR_INPUT_PASSWORD                  "Password: "
#define STR_INPUT_PASSWORD_CONFIRM          "Confirm password: "
#define STR_PASSWORD_TOO_SHORT              "Password too short, at least 8 bits"
#define STR_PASSWORD_NOT_MATCH              "Two password do not match"
#define STR_IN_ENCRYPTING                   "Encrypting: "
#define STR_IN_DECRYPTING                   "Decrypting: "
#define STR_RATE                            "Rate: "
#define STR_FOPEN_ERR                       "Fail to open file"
#define STR_FAIL_TO_READ_FILE               "Fail to read file, make sure it is readable"
#define STR_PASSWORD_NULL                   "Length of key could not be null"
#define STR_FAIL_TO_CREATE_OPEN_FILE        "Fail to create or open file"
#define STR_FAIL_TO_WRITE_FILE              "Fail to write to file, make sure it is writeable"
#define STR_FAIL_TO_DELETE_OLD_FILE         "Fail to delete old file"
#define STR_SUCCESS                         "[ success ]"
#define STR_FAIL                            "[ fail ]"
#define STR_FAIL_TO_CREATE_FILE_LIST        "Fail to create file_lists"
#define STR_FILE_LIST_IS_NULL               "Folder is empty"
#define STR_NULL                            "NULL"
#define STR_FILE_IS_NULL                    "It is an empty file"
#define STR_FAIL_TO_SCAN_DIRECTORY          "Fail to scan directory"
#define STR_TASK                            "Task"
#define STR_SUCCESS_COUNT                   "Success: "
#define STR_FAIL_COUNT                      "Fail: "
#define STR_VIEW_LOG                        "[ view log ]"
#define STR_GO_BACK                         "[ go back ]"
#define STR_CONTINUE                        "[ continue ]"
#define STR_RESULT_WIN_LABEL                "Pgdn: next    Pgup: last    Enter: go back    Esc: exit    "
#define STR_LOG_IS_NULL                     "Log is empty"
#define STR_EXIT_PROJECT                    "Continue to exit project ?"

#define STR_ERR_INVALID_FUNC                "Error: invalid func"
#define STR_ERR_INVALID_COLS                "Error: need wider console, at least: "
#define STR_ERR_INVALID_LINES               "Error: need higher console, at least: "
#define STR_ERR_PTHREAD_CREATE              "Error: in pthread_create: "
#define STR_ERR_PTHREAD_JOIN                "Error: in pthread_join: "
#define STR_ERR_FAIL_TO_MALLOC              "Error: not enough running memory space"
#define STR_ERR_STR_TOO_SHORT               "Error: string is too short in CTL_MakeChoice: "
#define STR_ERR_STR_IS_NULL                 "Error: string is null in CTL_MakeChoice: "
#define STR_ERR_FILE_LIST_NOT_EXIST         "Error: file_list does not exist"
#define STR_ERR_FAIL_TO_GET_FILE_INFO       "Error: fail to get file info"
#define STR_ERR_FAIL_TO_OPEN_FILE_LIST      "Error: fail to open file_list"
#define STR_ERR_INVALID_PTHREAD_ARG         "Error: invalid pthread arguments"
#define STR_ERR_INVALID_FILE_LIST           "Error: invalid FlieList"
#define STR_ERR_INVALID_FILE_LIST_ARG       "Error: invalid FlieList arguments"
#define STR_ERR_PTHREAD_NUM_TOO_SMALL       "Error: PTHREAD_NUM_MAX must be equare to or bigger than 2"
#define STR_ERR_PTHREAD_NUM_TOO_BIG         "Error: PTHREAD_NUM_MAX must be equare to or less than 8"
#define STR_ERR_INVALID_LOG_FILE            "Error: invalid log file format"

extern char *g_pMenuStr[];
extern char *pInstruction[];

#endif
