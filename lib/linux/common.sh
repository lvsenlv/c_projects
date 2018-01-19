#!/bin/bash

function DISP_RED()
{
    echo -e "\033[01;31m$1\033[0m";
}

function DISP_GREEN()
{
    echo -e "\033[01;32m$1\033[0m";
}

function DISP_YELLOW()
{
    echo -e "\033[01;33m$1\033[0m";
}

function DISP_WHITE()
{
    echo -e "\033[01;37m$1\033[0m";
}

function DISP()
{
    echo -e "\033[01;37m$1\033[0m";
}

function DISP_ERR()
{
    echo -e "\033[01;31mFatal error: $1\033[0m";
}

function DISP_WARNING()
{
    echo -e "\033[01;33mWarning: $1\033[0m";
}

function IsDigit()
{
    if [ $1 -gt 0  ] 2>/dev/null ;then
        return 0
    else
        return -1
    fi
}
