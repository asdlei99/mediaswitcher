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
#include "IniSettingRead.h"
#include <windows.h>

IniSettingRead::IniSettingRead(void)
{
}

IniSettingRead::~IniSettingRead(void)
{
}

void	IniSettingRead::GetBufferSetting(int& size)
{
	string strIniFile = INI_DEF_FILE;
	getIniFileName(strIniFile);
	unsigned long	nFlag = 0;
	nFlag=  GetPrivateProfileInt(BUFFER_SETTING_INI, SIZE_FLAG, 0, strIniFile.c_str());
	size = ( nFlag > 0 && nFlag < 1000) ? nFlag : 1000;
}

void IniSettingRead::GetInitBufferSize(int& size)
{
	string strIniFile = INI_DEF_FILE;
	getIniFileName(strIniFile);
	unsigned long	nFlag = 0;
	nFlag=  GetPrivateProfileInt(BUFFER_SETTING_INI, INITIALIZE_SIZE_FLAG, 0, strIniFile.c_str());
	size = ( nFlag > 0 && nFlag < 1000) ? nFlag : 1000;	
}

void IniSettingRead::GetIniFileName(string& strIniFile)
{
	char appfilePath[256];
	if(::GetModuleFileName(NULL,appfilePath,256) == 0)
	{
		return ;
	}
	char iniPath[_MAX_PATH];
	char Drive[_MAX_DRIVE];
	char Dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(appfilePath,Drive,Dir,fname,ext);
	sprintf(iniPath,"%s%s%s",Drive,Dir,strIniFile.c_str());
	strIniFile = iniPath;
}

#define GETDVMODE(DV)\
void   IniSettingRead::Get##DV##Mode(int& mode)\
{\
	string strIniFile = INI_DEF_FILE;\
	getIniFileName(strIniFile);\
	unsigned long	nFlag = 0;\
	nFlag=  GetPrivateProfileInt(Test_DV_SETTING, Test_##DV##_SETTING_MODE, 0, strIniFile.c_str());\
	mode = ( nFlag >= 0 && nFlag < 2) ? nFlag : 0;\
}\

#define GETDVFILENAME(DV)\
	void   IniSettingRead::Get##DV##FileName(string& filename)\
{\
	string strIniFile = INI_DEF_FILE;\
	getIniFileName(strIniFile);\
	char name[1024];\
	string strError = "NOT FOUND";\
	GetPrivateProfileString(Test_DV_SETTING, Test_##DV##_FILE_Name, strError.c_str(), name,1024, strIniFile.c_str());\
	string strname(name);\
	if(strname == strError)\
	{\
		return;\
	}\
	filename = strname;\
}\

#define GETDVOUTPUTFILENAME(DV)\
void   IniSettingRead::Get##DV##OutputFileName(string& filename)\
{\
	string strIniFile = INI_DEF_FILE;\
	getIniFileName(strIniFile);\
	char name[1024];\
	string strError = "NOT FOUND";\
	GetPrivateProfileString(OUTPUT_SETTING, DV##_FILENAME, strError.c_str(), name,1024, strIniFile.c_str());\
	string strname(name);\
	if(strname == strError)\
	{\
		return;\
	}\
	filename = strname;\
}\

#define GETDVOUTPUTMODE(DV)\
void   IniSettingRead::Get##DV##OutputMode(int& mode)\
{\
	string strIniFile = INI_DEF_FILE;\
	getIniFileName(strIniFile);\
	unsigned long	nFlag = 0;\
	nFlag=  GetPrivateProfileInt(OUTPUT_SETTING,DV##_OUTPUT_MODE, 0, strIniFile.c_str());\
	mode = ( nFlag >= 0 && nFlag < 2) ? nFlag : 0;\
}\

GETDVMODE(DV1);
GETDVMODE(DV2);
GETDVMODE(DV3);
GETDVMODE(DV4);

GETDVFILENAME(DV1);
GETDVFILENAME(DV2);
GETDVFILENAME(DV3);
GETDVFILENAME(DV4);

GETDVOUTPUTFILENAME(DV1);
GETDVOUTPUTFILENAME(DV2);
GETDVOUTPUTFILENAME(DV3);

GETDVOUTPUTMODE(DV1);
GETDVOUTPUTMODE(DV2);
GETDVOUTPUTMODE(DV3);

void   IniSettingRead::GetLogMode(int& mode)
{
	string strIniFile = INI_DEF_FILE;
	getIniFileName(strIniFile);
	unsigned long	nFlag = 0;
	nFlag=  GetPrivateProfileInt(LOG_MODE_SETTING, LOG_MODE_VALUE, 0, strIniFile.c_str());
	mode = ( nFlag >= 0 && nFlag < 5) ? nFlag : 0;
}

void   IniSettingRead::GetSwitchOutputFileName(string& filename)
{
	string strIniFile = INI_DEF_FILE;
	getIniFileName(strIniFile);
	char name[1024];
	string strError = "NOT FOUND";
	GetPrivateProfileString(OUTPUT_SETTING, SWITCH_FILENAME, strError.c_str(), name,1024, strIniFile.c_str());
	string strname(name);
	if(strname == strError)
	{
		return;
	}

	filename = strname;
}