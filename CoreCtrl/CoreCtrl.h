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
#ifndef __CORECTRL_H
#define __CORECTRL_H

#include "CorectrlInterface.h"

class CoreCtrl: public ICoreCtrlInterface
{
public:
	CoreCtrl(void);
	virtual ~CoreCtrl(void);

	virtual bool RegisterSampleReceiver(ISampleReceiver* pReveiver);
	virtual bool UnRegisterSampleReceiver();

	virtual HRESULT InitialPlayerSession(HWND windowHandle);

	virtual bool SetSampleReceiver();

	virtual void   GetSwitchFileName(string& filename, bool flag = false);
	virtual void   GetSwitchPathName(string& filename);
	virtual void   GetBreakPointFileName(string& filename);
	virtual void   GetLiveRecordFileName(string& filename);

	virtual bool SetOutPutID(DWORD_PTR dwID);

	virtual HRESULT DeleteSource( DWORD_PTR dwID );
	virtual BOOL CreateGraphCtrl(GraphType type, string fileName, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID);
	virtual BOOL CreateOutputGraphCtrl(GraphType graphType, OutputType outputType, string fileName,
		DWORD pushPort,char * pushUrl, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID);
	virtual BOOL DeleteGraphCtrl(DWORD_PTR& ctrlID);

	virtual bool SignalShowEngine(BOOL signal);
	//for IDeviceInterface
	virtual BOOL GetAudioDevice(DEVICE_LIST& audioDeviceList);
	virtual BOOL GetVideoDevice(DEVICE_LIST& videoDeviceList);
	virtual BOOL FindDevice(const char * inDeviceName, DSDevice& outDevice);

	virtual HRESULT RunVMR9Subgraph(DWORD_PTR dwID);
	virtual HRESULT PauseVMR9Subgraph(DWORD_PTR dwID);
	virtual HRESULT StopVMR9Subgraph(DWORD_PTR dwID);
	virtual CVMR9Subgraph* GetSubgraph(DWORD_PTR dwID);

	virtual HRESULT RegisterReceiverInSubgraph(CMsgReceiver* receiver, DWORD_PTR dwID);

	//for video in show engine
	virtual HRESULT AddVideoFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type);
	virtual HRESULT AddVideoDeviceSource(Device_Type deviceType, string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type);
	//for audio in show engine
	virtual HRESULT AddAudioFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type);
	virtual HRESULT AddAudioDeviceSource(string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type);

	virtual HRESULT SwitchAudioSender(string name);

	//get total time of the filter graph
	virtual HRESULT GetTotalPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second);
	//get current time of the filter graph
	virtual HRESULT GetCurrentPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second);
	//seek from current position
	virtual HRESULT RelativeSeek(DWORD_PTR dwID, REFERENCE_TIME time);

	virtual void GetOutputAudioSampleValueRate(int& value);
	virtual CtrlStatus GetCtrlStatus( DWORD_PTR dwID );
	virtual HRESULT SetCtrlStatus( DWORD_PTR dwID,CtrlStatus status );

	virtual void SetOutputToFileFlag(BOOL flag);

	virtual BOOL IsValidFileName(string fileName, int& freeSpaceCounter, bool flag=false);//flag=false: DV flag = true:wmv
	virtual void SetFileName(string name);
	//for subtitle
	virtual HRESULT   SetTextSubtitleBuffer(unsigned char* buffer, int width, int height, int PosX, int PosY);
	virtual HRESULT   SetTextSubtitleShowFlag(BOOL flag);
	//for logo
	virtual HRESULT SetLogoFileName(unsigned char* fileName);
	virtual HRESULT SetLogoShowFlag(BOOL showFlag);

	virtual void PauseOutPutFile(string audioName, DWORD_PTR videoId);
	virtual void PlayOutPutFile(string audioName, DWORD_PTR videoId);

	virtual BOOL SetNotifyWindow(CVMR9Subgraph* inGraph, HWND inWindow);
	virtual void HandleEvent(CVMR9Subgraph* inGraph, WPARAM inWParam, LPARAM inLParam);
};

#endif
