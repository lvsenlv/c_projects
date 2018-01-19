#!/bin/bash

PathBackup=`pwd`
path=`dirname $0`
. ${path}/common.sh

PackageVersion="ncurses-6.0"
PackageName="${path}/${PackageVersion}.tar.gz"

if [ $# != 1 ];then
    DISP_ERR "lack of an option, please input the path of library as an option"
    exit -1
fi

if [ X"$1" == X"clean" ];then
    DISP "Delete file *.a in $path"
    rm -rf ${path}/*.a
    DISP "Delete directory $PackageVersion in $path"
    rm -rf ${path}/${PackageVersion}
    exit 0
elif [ ! -d $1 ];then
    DISP_ERR "no such directory: $1"
    exit -1
fi

LibDir=$1

if [[ -f ${LibDir}/libformw.a && -f ${LibDir}/libmenuw.a && -f ${LibDir}/libncursesw.a && -f ${LibDir}/libpanelw.a ]];then
    exit 0
fi

if [ ! -f $PackageName ];then
    DISP_ERR "no such file: $PackageName"
    exit -1
fi

tar -xzvf $PackageName -C $path
if [ X"$?" != X"0" ];then
    DISP_ERR "fail to decompress $PackageName"
    exit -1
fi

if [ ! -d ${path}/${PackageVersion} ];then
    DISP_ERR "no such directory: ${path}/${PackageVersion}"
    exit -1
fi

DISP "Enter directory: ${path}/${PackageVersion}"
cd ${path}/${PackageVersion}
if [ ! -f configure ];then
    DISP_ERR "no such file: configure"
    exit -1
fi

if [ ! -x configure ];then
    chmod +x configure
fi

./configure --enable-widec
if [ X"$?" != X"0" ];then
    DISP_ERR "fail to execute configure"
    exit -1
fi

make -j4
if [ X"$?" != X"0" ];then
    DISP_ERR "fail to execute make"
    exit -1
fi

if [ ! -f lib/libformw.a ];then
    DISP_ERR "no such file: ${path}/${PackageVersion}/lib/libformw.a"
    exit -1;
fi

if [ ! -f lib/libmenuw.a ];then
    DISP_ERR "no such file: ${path}/${PackageVersion}/lib/libmenuw.a"
    exit -1;
fi

if [ ! -f lib/libncursesw.a ];then
    DISP_ERR "no such file: ${path}/${PackageVersion}/lib/libncursesw.a"
    exit -1;
fi

if [ ! -f lib/libpanelw.a ];then
    DISP_ERR "no such file: ${path}/${PackageVersion}/lib/libpanelw.a"
    exit -1;
fi

cp lib/libformw.a lib/libmenuw.a lib/libncursesw.a lib/libpanelw.a ${PathBackup}/${LibDir}

