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

	void	getBufferSetting(int& size);
	void	getIniFileName(string& strIniFile);
	void	getInitBufferSize(int& size);

	void   getDV1Mode(int& mode);
	void   getDV1FileName(string& filename);
	void   getDV2Mode(int& mode);
	void   getDV2FileName(string& filename);
	void   getDV3Mode(int& mode);
	void   getDV3FileName(string& filename);
	void   getDV4Mode(int& mode);
	void   getDV4FileName(string& filename);
	void   getLogMode(int& mode);

	void   getDV1OutputMode(int& mode);
	void   getDV2OutputMode(int& mode);
	void   getDV3OutputMode(int& mode);

	void   getDV1OutputFileName(string& filename);
	void   getDV2OutputFileName(string& filename);
	void   getDV3OutputFileName(string& filename);
	void   getSwitchOutputFileName(string& filename);
};

#endif // __MUXINISETTINGREAD_H
