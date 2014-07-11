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
#include "FreeSpaceCtrl.h"
#include <windows.h>
#include <io.h>
#include "PrintDebug.h"

FreeSpaceCtrl* FreeSpaceCtrl::s_pFreeSpaceCtrl = NULL;

FreeSpaceCtrl::FreeSpaceCtrl(void)
{
}

FreeSpaceCtrl::~FreeSpaceCtrl(void)
{
}

FreeSpaceCtrl& FreeSpaceCtrl::GetInstance()
{
	if (s_pFreeSpaceCtrl == NULL)
	{
		CreateInstance();
	}

	return *s_pFreeSpaceCtrl;
}

void FreeSpaceCtrl::CreateInstance()
{
	static FreeSpaceCtrl freespacectrl;
	s_pFreeSpaceCtrl = &freespacectrl;
}

TimeDef FreeSpaceCtrl::GetTimeDefForPath(string fileName)
{
	TimeDef timeDef;

	return timeDef;
}

void FreeSpaceCtrl::AddFileNameToCtrl(string fileName)
{
	if (fileName != "")
	{
		TimeDef timeDef;
		m_mapFilePathTimeDef[fileName] = timeDef;
	}
}
void FreeSpaceCtrl::RemoveFileNameFromCtrl(string fileName)
{
	MapFilePathTimeDefIter iter = m_mapFilePathTimeDef.find(fileName);
	if (iter != m_mapFilePathTimeDef.end())
	{
		m_mapFilePathTimeDef.erase(iter);
	}
}

unsigned int __stdcall FreeSpaceCtrl::CalcThread(void *arg)
{
	return true;
}

bool FreeSpaceCtrl::IsValidFileName(string fileName, int& freeSpaceCounter, bool flag)
{
	freeSpaceCounter = 0;
	if (!_access(fileName.c_str(), 0))
	{//this file has existed
		return false;
	}
	bool isValid = true;
	size_t nPos = fileName.find("\\");
	string rootPathName = fileName.substr(0, nPos);
	nPos = fileName.find_last_of("\\");
	string filePath = fileName.substr(0, nPos);
	if (_access(filePath.c_str(), 0))
	{
		return false;
	}
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD numberOfFreeClusters;
	DWORD totalNumberOfClusters;
	GetDiskFreeSpace(rootPathName.c_str(), 
		&sectorsPerCluster,
		&bytesPerSector,
		&numberOfFreeClusters,
		&totalNumberOfClusters);
	
	DWORD freeSapceSize = (double)numberOfFreeClusters/1024/1024*sectorsPerCluster*bytesPerSector;//MB

	if (flag)
	{
		freeSpaceCounter = (double)freeSapceSize/(0.07);
	}
	else
	{
		freeSpaceCounter = (double)freeSapceSize/(3.6);
	}
	
	char msg[MAX_PATH];
	sprintf(msg, "GetDiskFreeSpace:numberOfFreeClusters:%d sectorsPerCluster:%d bytesPerSector:%d  freeSpaceCounter:%d \n",numberOfFreeClusters,sectorsPerCluster,bytesPerSector,freeSpaceCounter);
	DebugMessage(LOG_LEVEL1,msg);
	return isValid;
}
