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
#ifndef __FREESPACE_CTRL_H
#define __FREESPACE_CTRL_H

#include <string>
#include <map>
using namespace std;

class TimeDef 
{
public:
	string hour;
	string min;
	string sec;

	TimeDef()
	{	
		hour = "";
		min = "";
		sec = "";
	}
};

/** Class handle Free Space calculator.
*/
class FreeSpaceCtrl
{
public:
	~FreeSpaceCtrl(void);
	static FreeSpaceCtrl& GetInstance();
private:
	FreeSpaceCtrl(void);
	static FreeSpaceCtrl* s_pFreeSpaceCtrl;
	static void CreateInstance();

public:

	typedef map<string, TimeDef> MapFilePathTimeDef;
	typedef map<string, TimeDef>::iterator MapFilePathTimeDefIter;
	/** Get free space for a specific file name.
	@remarks
	*/
	TimeDef GetTimeDefForPath(string fileName);
	/** Add a file name to ctrl.
	@remarks
	*/
	void AddFileNameToCtrl(string fileName);
	/** Remove a file name from ctrl.
	@remarks
	*/
	void RemoveFileNameFromCtrl(string fileName);

	/** Check the filename's disc's free space is larger than 5min or not.
	@remarks
	*/
	bool IsValidFileName(string fileName, int& freeSpaceCounter, bool flag=false);//flag=false: DV flag = true:wmv

private:
	MapFilePathTimeDef m_mapFilePathTimeDef;
	/** Thread for calculating free space.
	@remarks
	*/
	static unsigned int __stdcall CalcThread(void *arg);
};
#endif