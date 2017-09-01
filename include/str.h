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
#define STR_GO_BACK                         "[ Go back ]"


#define STR_ERR_INVALID_COLS                "Error: need wider console, at least: "
#define STR_ERR_INVALID_HEIGHT              "Error: need higher console, at least: "
#define STR_ERR_PTHREAD_CREATE              "Error: in pthread_create: "
#define STR_ERR_PTHREAD_JOIN                "Error: in pthread_join: "

extern char *g_pMenuStr[];
extern char *pInstruction[];

#endif
