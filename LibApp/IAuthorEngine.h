/*Copyright (C) 2010 Kevin.Wen, All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

-----------------
*/
#ifndef __IAOTHORENGINE_H
#define __IAOTHORENGINE_H
#include <string>

using namespace std;

class IAuthorEngine
{
public:
	~IAuthorEngine(){}
	virtual BOOL CreateGraphCtrl(GraphType type, string fileName, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID) = 0;
	virtual BOOL CreateOutputGraphCtrl(GraphType graphType, OutputType outputType, string fileName,DWORD pushPort,char * pushUrl, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID) = 0;
	virtual BOOL DeleteGraphCtrl(DWORD_PTR& ctrlID) = 0;
};
#endif