#!/bin/bash

getchar() 
{ 
    SAVEDSTTY=`stty -g` 
    stty -echo 
    stty cbreak 
    dd if=/dev/tty bs=1 count=1 2> /dev/null 
    stty -raw 
    stty echo 
    stty $SAVEDSTTY 
}

if [ ! -f file_list.log ];then
    echo "Error: file_list.log does not exist"
    exit 1
fi


#if [ ! -s file_list.log ];then
    #echo "Error: file_list.log is empty"
    #exit 1
#fi

FILE_LIST=`cat file_list.log`
for file in $FILE_LIST
do
    echo $file
done

flag=0
while ((1))
do
    read -p "Delete above files, y or n ? " choice
    case $choice in
        y | Y )
            flag=1
            break
            ;;
        n | N )
            flag=0
            break
            ;;
        *)
            continue
            ;;
    esac
done

if [ $flag = 1 ];then
    for file in $FILE_LIST
    do
       rm -f $file
    done
    echo "success"
else
    echo "do nothing"
fi
