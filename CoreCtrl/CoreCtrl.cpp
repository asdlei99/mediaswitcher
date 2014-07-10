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

CoreCtrl.cpp: implement the interface of MediaSwitcher SDK.
*/
#include "CoreCtrl.h"
#include "../LibApp/AppCtrl.h"

CoreCtrl::CoreCtrl(void)
{
}

CoreCtrl::~CoreCtrl(void)
{
}

bool CoreCtrl::RegisterSampleReceiver(ISampleReceiver* pReveiver)
{
	return AppCtrl::GetInstance().RegisterSampleReceiver(pReveiver);
}
bool CoreCtrl::UnRegisterSampleReceiver()
{
	return AppCtrl::GetInstance().UnRegisterSampleReceiver();
}

HRESULT CoreCtrl::InitialPlayerSession(HWND windowHandle)
{
	return AppCtrl::GetInstance().InitialPlayerSession(windowHandle);
}

bool CoreCtrl::SetSampleReceiver()
{//todo, check it is necessary?
	return AppCtrl::GetInstance().SetSampleReceiver();
}

void   CoreCtrl::GetSwitchFileName(string& filename, bool flag)
{
	return AppCtrl::GetInstance().GetSwitchFileName(filename, flag);
}

void   CoreCtrl::GetSwitchPathName(string& filename)
{
	return AppCtrl::GetInstance().GetSwitchPathName(filename);
}

void   CoreCtrl::GetBreakPointFileName(string& filename)
{
	return AppCtrl::GetInstance().GetBreakPointFileName(filename);
}

void   CoreCtrl::GetLiveRecordFileName(string& filename)
{
	return AppCtrl::GetInstance().GetLiveRecordFileName(filename);
}

bool CoreCtrl::SetOutPutID(DWORD_PTR dwID)
{
	return AppCtrl::GetInstance().SetOutPutID(dwID);
}

HRESULT CoreCtrl::DeleteSource( DWORD_PTR dwID )
{
	return AppCtrl::GetInstance().DeleteSource(dwID);
}

BOOL CoreCtrl::CreateOutputGraphCtrl(GraphType graphType, OutputType outputType, string fileName,
									 DWORD pushPort,char * pushUrl, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID)
{
	return AppCtrl::GetInstance().CreateOutputGraphCtrl(graphType, outputType, fileName, pushPort, pushUrl, hwnd, dwId, retCtrlID);
}

BOOL CoreCtrl::CreateGraphCtrl(GraphType type, string fileName, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID)
{
	return AppCtrl::GetInstance().CreateGraphCtrl(type, fileName, hwnd, dwId, retCtrlID);
}

BOOL CoreCtrl::DeleteGraphCtrl(DWORD_PTR& ctrlID)
{
	return AppCtrl::GetInstance().DeleteGraphCtrl(ctrlID);
}

bool CoreCtrl::SignalShowEngine(BOOL signal)
{
	return AppCtrl::GetInstance().SignalShowEngine(signal);
}
//for IDeviceInterface
BOOL CoreCtrl::GetAudioDevice(DEVICE_LIST& audioDeviceList)
{
	return AppCtrl::GetInstance().GetAudioDevice(audioDeviceList);
}

BOOL CoreCtrl::GetVideoDevice(DEVICE_LIST& videoDeviceList)
{
	return AppCtrl::GetInstance().GetVideoDevice(videoDeviceList);
}

BOOL CoreCtrl::FindDevice(const char * inDeviceName, DSDevice& outDevice)
{
	return AppCtrl::GetInstance().FindDevice(inDeviceName, outDevice);
}

HRESULT CoreCtrl::RunVMR9Subgraph(DWORD_PTR dwID)
{
	return AppCtrl::GetInstance().RunVMR9Subgraph(dwID);
}

HRESULT CoreCtrl::PauseVMR9Subgraph(DWORD_PTR dwID)
{
	return AppCtrl::GetInstance().PauseVMR9Subgraph(dwID);
}

HRESULT CoreCtrl::StopVMR9Subgraph(DWORD_PTR dwID)
{
	return AppCtrl::GetInstance().StopVMR9Subgraph(dwID);
}
CVMR9Subgraph* CoreCtrl::GetSubgraph(DWORD_PTR dwID)
{
	return AppCtrl::GetInstance().GetSubgraph(dwID);
}

HRESULT CoreCtrl::RegisterReceiverInSubgraph(CMsgReceiver* receiver, DWORD_PTR dwID)
{
	return AppCtrl::GetInstance().RegisterReceiverInSubgraph(receiver, dwID);
}

//for video in show engine
HRESULT CoreCtrl::AddVideoFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type)
{
	return AppCtrl::GetInstance().AddVideoFileSource(sourceName, flag, wcPath, dwID, type);
}

HRESULT CoreCtrl::AddVideoDeviceSource(Device_Type deviceType, string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type)
{
	return AppCtrl::GetInstance().AddVideoDeviceSource(deviceType, sourceName, flag, deviceName, dwID, type);
}

//for audio in show engine
HRESULT CoreCtrl::AddAudioFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type)
{
	return AppCtrl::GetInstance().AddAudioFileSource(sourceName, flag, wcPath, dwID, type);
}

HRESULT CoreCtrl::AddAudioDeviceSource(string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type)
{
	return AppCtrl::GetInstance().AddAudioDeviceSource(sourceName, flag, deviceName, dwID, type);
}

HRESULT CoreCtrl::SwitchAudioSender(string name)
{
	return AppCtrl::GetInstance().SwitchAudioSender(name);
}

//get total time of the filter graph
HRESULT CoreCtrl::GetTotalPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second)
{
	return AppCtrl::GetInstance().GetTotalPlaybackTime(dwID, hour, minute, second);
}
//get current time of the filter graph
HRESULT CoreCtrl::GetCurrentPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second)
{
	return AppCtrl::GetInstance().GetCurrentPlaybackTime(dwID, hour, minute, second);
}
//seek from current position
HRESULT CoreCtrl::RelativeSeek(DWORD_PTR dwID, REFERENCE_TIME time)
{
	return AppCtrl::GetInstance().RelativeSeek(dwID, time);
}

void CoreCtrl::GetOutputAudioSampleValueRate(int& value)
{
	return AppCtrl::GetInstance().GetOutputAudioSampleValueRate(value);
}

CtrlStatus CoreCtrl::GetCtrlStatus( DWORD_PTR dwID )
{
	return AppCtrl::GetInstance().GetCtrlStatus(dwID );
}

HRESULT CoreCtrl::SetCtrlStatus( DWORD_PTR dwID,CtrlStatus status )
{
	return AppCtrl::GetInstance().SetCtrlStatus(dwID, status );
}

void CoreCtrl::SetOutputToFileFlag(BOOL flag)
{
	return AppCtrl::GetInstance().SetOutputToFileFlag(flag);
}

BOOL CoreCtrl::IsValidFileName(string fileName, int& freeSpaceCounter, bool flag)
{
	return AppCtrl::GetInstance().IsValidFileName(fileName, freeSpaceCounter, flag);
}

void CoreCtrl::SetFileName(string name)
{
	return AppCtrl::GetInstance().SetFileName(name);
}

//for subtitle
HRESULT   CoreCtrl::SetTextSubtitleBuffer(unsigned char* buffer, int width, int height, int PosX, int PosY)
{
	return AppCtrl::GetInstance().SetTextSubtitleBuffer(buffer, width, height, PosX, PosY);
}

HRESULT   CoreCtrl::SetTextSubtitleShowFlag(BOOL flag)
{
	return AppCtrl::GetInstance().SetTextSubtitleShowFlag(flag);
}

//for Logo
HRESULT CoreCtrl::SetLogoFileName(unsigned char* fileName)
{
	return AppCtrl::GetInstance().SetLogoFileName(fileName);
}

HRESULT CoreCtrl::SetLogoShowFlag(BOOL showFlag)
{
	return AppCtrl::GetInstance().SetLogoShowFlag(showFlag);
}

void CoreCtrl::PauseOutPutFile(string audioName, DWORD_PTR videoId)
{
	return AppCtrl::GetInstance().PauseOutPutFile(audioName, videoId);
}

void CoreCtrl::PlayOutPutFile(string audioName, DWORD_PTR videoId)
{
	return AppCtrl::GetInstance().PlayOutPutFile(audioName, videoId);
}

BOOL CoreCtrl::SetNotifyWindow(CVMR9Subgraph* inGraph, HWND inWindow)
{
	return AppCtrl::GetInstance().SetNotifyWindow(inGraph,inWindow);
}

void CoreCtrl::HandleEvent(CVMR9Subgraph* inGraph, WPARAM inWParam, LPARAM inLParam)
{
	return AppCtrl::GetInstance().HandleEvent(inGraph, inWParam, inLParam);
}

extern "C"
{
	__declspec(dllexport) ICoreCtrlInterface* CreateCoreCtrl()
	{
		return new CoreCtrl();
	}

	__declspec(dllexport) void DeleteCoreCtrl(void* ptr)
	{
		if (ptr)
		{
			delete (ICoreCtrlInterface*)ptr;
		}
	}
};