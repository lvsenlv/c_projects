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
#define STR_GO_BACK                         "[ Press Enter to go back ]"
#define STR_INPUT_FILE_NAME                 "Please input the file or folder name with full path"
#ifdef __LINUX
#define STR_INPUT_FILE_NAME_EG              "E.g, /root/document/..."
#elif defined __WINDOWS
#define STR_INPUT_FILE_NAME_EG              "E.g, C:/User/Administrator/..."
#endif 
#define STR_INPUT                           "Input: "
#define STR_RETRY                           "[ Retry ]"
#define STR_BACK                            "[ Back ]"
#define STR_FILE_NOT_EXIST                  "The file or directory does not exist"



#define STR_ERR_INVALID_COLS                "Error: need wider console, at least: "
#define STR_ERR_INVALID_LINES               "Error: need higher console, at least: "
#define STR_ERR_PTHREAD_CREATE              "Error: in pthread_create: "
#define STR_ERR_PTHREAD_JOIN                "Error: in pthread_join: "
#define STR_ERR_FAIL_TO_MALLOC              "Error: not enough running memory space"
#define STR_ERR_STR_TOO_SHORT               "Error: string is too short in CTL_MakeChoice: "
#define STR_ERR_STR_IS_NULL                 "Error: string is null in CTL_MakeChoice: "

extern char *g_pMenuStr[];
extern char *pInstruction[];

#endif
