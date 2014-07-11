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
#ifndef __MUXINISETTINGREAD_H
#define __MUXINISETTINGREAD_H

#include "IniSettings.h"
#include <string>

using namespace std;
class IniSettingRead
{
public:
	IniSettingRead(void);
	~IniSettingRead(void);

	void   GetBufferSetting(int& size);
	void   GetIniFileName(string& strIniFile);
	void   GetInitBufferSize(int& size);

	void   GetDV1Mode(int& mode);
	void   GetDV1FileName(string& filename);
	void   GetDV2Mode(int& mode);
	void   GetDV2FileName(string& filename);
	void   GetDV3Mode(int& mode);
	void   GetDV3FileName(string& filename);
	void   GetDV4Mode(int& mode);
	void   GetDV4FileName(string& filename);
	void   GetLogMode(int& mode);

	void   GetDV1OutputMode(int& mode);
	void   GetDV2OutputMode(int& mode);
	void   GetDV3OutputMode(int& mode);

	void   GetDV1OutputFileName(string& filename);
	void   GetDV2OutputFileName(string& filename);
	void   GetDV3OutputFileName(string& filename);
	void   GetSwitchOutputFileName(string& filename);
};

#endif // __MUXINISETTINGREAD_H
