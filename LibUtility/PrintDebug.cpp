/*Copyright (C) 2010 Kevin.Wen, All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

-----------------
*/
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "PrintDebug.h"
#include "../LibApp/AppCtrl.h"

#define DEBUG_TO_FILE
#ifdef DEBUG_TO_FILE
static HANDLE hFile ;
#endif

static int debug_level = 0;

void SetDebugLevel(int level)
{
	debug_level = level;
}

void OpenDebugFile()
{
#ifdef DEBUG_TO_FILE
	hFile = CreateFile("mediaswitcher.log",           
		GENERIC_WRITE,              
		FILE_SHARE_READ,           
		NULL,                      
		OPEN_ALWAYS,               
		FILE_ATTRIBUTE_NORMAL,     
		NULL);     
#endif
}

void CloseDebugFile()
{
#ifdef DEBUG_TO_FILE
	if (hFile)
	{
		CloseHandle(hFile);
	}
#endif
}

void DebugMessage(LOG_LEVEL level, char* fmt, ...) 
{
#ifdef DEBUG_TO_FILE
	if(debug_level&level)
	{
		va_list argptr;
		char bufferline[512];
		char osDate[30],osTime[30]; 
		char temp[1024]; 
		int len ;
		DWORD nbBytesWritten;
		boolean b;	

		va_start(argptr, fmt);
		_vsnprintf(bufferline, 512, fmt, argptr);
		va_end(argptr);

		_strtime( osTime );
		_strdate( osDate );
		len = _snprintf(temp,1021,"%s %s: %s",osDate,osTime,bufferline);
		temp[len++] = 0x0d;
		temp[len++] = 0x0a;
		temp[len+1] = 0;

		if (hFile != INVALID_HANDLE_VALUE) 
		{ 
			nbBytesWritten = 0;
			SetFilePointer(hFile, 0, NULL, FILE_END);
			b = WriteFile(
				hFile,                    
				temp,                
				len,     
				&nbBytesWritten,  
				NULL        
				);
		}
	}

#else 
#ifdef _DEBUG
	char s[1024];
	sprintf(s, fmt, (va_list)(&fmt+1) );
	strcat(s,("\n"));
	OutputDebugString(s);
#endif
#endif //DEBUG_TO_FILE
}