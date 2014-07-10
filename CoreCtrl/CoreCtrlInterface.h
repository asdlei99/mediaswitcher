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

CoreCtrlInterface.h: the interface for MediaSwitcher SDK.
*/
#ifndef __CORECTRL_INTERFACE_H
#define __CORECTRL_INTERFACE_H
#include <windows.h>
#include <string>
#include "../LibApp/GlobalDefine.h"
#include "../LibDevice/DSDevice.h"
#include "../LibShowEngineCtrl/VMR9Subgraph.h"

using namespace std;
class ISampleReceiver;

class ICoreCtrlInterface
{
public:
	virtual ~ICoreCtrlInterface(){}
	virtual bool RegisterSampleReceiver(ISampleReceiver* pReveiver) = 0;
	virtual bool UnRegisterSampleReceiver() = 0;

	virtual HRESULT InitialPlayerSession(HWND windowHandle) = 0;

	virtual bool SetSampleReceiver() = 0;

	virtual void   GetSwitchFileName(string& filename, bool flag = false) = 0;
	virtual void   GetSwitchPathName(string& filename) = 0;
	virtual void   GetBreakPointFileName(string& filename) = 0;
	virtual void   GetLiveRecordFileName(string& filename) = 0;

	virtual bool SetOutPutID(DWORD_PTR dwID) = 0;

	virtual HRESULT DeleteSource( DWORD_PTR dwID ) = 0;
	virtual BOOL CreateGraphCtrl(GraphType type, string fileName, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID) = 0;
	virtual BOOL CreateOutputGraphCtrl(GraphType graphType, OutputType outputType, string fileName,DWORD pushPort,char * pushUrl, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID) = 0;
	virtual BOOL DeleteGraphCtrl(DWORD_PTR& ctrlID) = 0;

	virtual bool SignalShowEngine(BOOL signal) = 0;
	//for IDeviceInterface
	virtual BOOL GetAudioDevice(DEVICE_LIST& audioDeviceList) = 0;
	virtual BOOL GetVideoDevice(DEVICE_LIST& videoDeviceList) = 0;
	virtual BOOL FindDevice(const char * inDeviceName, DSDevice& outDevice) = 0;

	virtual HRESULT RunVMR9Subgraph(DWORD_PTR dwID) = 0;
	virtual HRESULT PauseVMR9Subgraph(DWORD_PTR dwID) = 0;
	virtual HRESULT StopVMR9Subgraph(DWORD_PTR dwID) = 0;
	virtual CVMR9Subgraph* GetSubgraph(DWORD_PTR dwID) = 0;

	virtual HRESULT RegisterReceiverInSubgraph(CMsgReceiver* receiver, DWORD_PTR dwID) = 0;

	//for video in show engine
	virtual HRESULT AddVideoFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type) = 0;
	virtual HRESULT AddVideoDeviceSource(Device_Type deviceType, string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type) = 0;
	//for audio in show engine
	virtual HRESULT AddAudioFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type) = 0;
	virtual HRESULT AddAudioDeviceSource(string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type) = 0;

	virtual HRESULT SwitchAudioSender(string name) = 0;

	//get total time of the filter graph
	virtual HRESULT GetTotalPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second) = 0;
	//get current time of the filter graph
	virtual HRESULT GetCurrentPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second) = 0;
	//seek from current position
	virtual HRESULT RelativeSeek(DWORD_PTR dwID, REFERENCE_TIME time) = 0;

	virtual void GetOutputAudioSampleValueRate(int& value) = 0;
	virtual CtrlStatus GetCtrlStatus( DWORD_PTR dwID ) = 0;
	virtual HRESULT SetCtrlStatus( DWORD_PTR dwID,CtrlStatus status ) = 0;

	virtual void SetOutputToFileFlag(BOOL flag) = 0;

	virtual BOOL IsValidFileName(string fileName, int& freeSpaceCounter, bool flag=false) = 0;//flag=false: DV flag = true:wmv
	virtual void SetFileName(string name) = 0;
	//for subtitle
	virtual HRESULT   SetTextSubtitleBuffer(unsigned char* buffer, int width, int height, int PosX, int PosY) = 0;
	virtual HRESULT   SetTextSubtitleShowFlag(BOOL flag) = 0;
	//for logo
	virtual HRESULT SetLogoFileName(unsigned char* fileName) = 0;
	virtual HRESULT SetLogoShowFlag(BOOL showFlag) = 0;

	virtual void PauseOutPutFile(string audioName, DWORD_PTR videoId) = 0;
	virtual void PlayOutPutFile(string audioName, DWORD_PTR videoId) = 0;

	virtual BOOL SetNotifyWindow(CVMR9Subgraph* inGraph, HWND inWindow) = 0;
	virtual void HandleEvent(CVMR9Subgraph* inGraph, WPARAM inWParam, LPARAM inLParam) = 0;
};

typedef ICoreCtrlInterface* (*CreateCoreCtrlFcn)();
typedef void (*DeleteCoreCtrlFcn)(void* ptr);

#endif