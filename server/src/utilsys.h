//========================================================================
//FILE: utilsys.h
//========================================================================
//	Copyright (c) 2001 by Sheppard Norfleet and Charles Kerr
//  All Rights Reserved
// 
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided the following conditions are met:
//
//	Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer. Redistributions
//	in binary form must reproduce the above copyright notice, this list of
//	conditions and the following disclaimer in the documentation and/or
//	other materials provided with the distribution.
//
//	Neither the name of the SWORDS  nor the names of its contributors may
//	be used to endorse or promote products derived from this software
//	without specific prior written permission. 
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  `AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	A PARTICULAR PURPOSE ARE DISCLAIMED. .
//========================================================================
//Directives
#ifndef	IN_UTILSYS_H
#define IN_UTILSYS_H
//========================================================================
//Platform Specs
#include "platform.h"
//========================================================================
//System Files

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
// Includes we need for time handling
#if defined(OBSOLETETIME)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
// Includes we need for directory manipulation 
#if defined(_POSIX)
#include <dirent.h>
#include <glob.h>
#include <unistd.h>
#define _stat stat
#define _mkdir mkdir
#define _rmdir rmdir
#elif defined(WIN32)
#include <direct.h>
#include <io.h>
#if defined(_MSC_VER)
#define S_ISDIR(m) m&_S_IFDIR

#define _mkdir(a,b) _mkdir(a)
#define mode_t int
#endif
#if defined(__borland__)
#define _mkdir(a,b) _mkdir(a)
#define mode_t int
#endif
#endif
using namespace std;
//=========================================================================
//Prototypes
UI32 getNormalizedTime() ;
UI32 getPlatformTime() ;
UI32 getPlatformDay() ;
string getRealTimeString() ;
bool makeDirectory(string sDirectory) ;
bool isDirectory(string sDirectory) ;
vector<string> listDirectory(string sDirectory) ;
bool deleteDirectory(string sDirectory) ;
bool deleteFile(string sFile) ;
//========================================================================
//========================================================================
//=======================  End of utilsys.h  ===============================
//========================================================================
//========================================================================
#endif

