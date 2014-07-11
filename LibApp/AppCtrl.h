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
#ifndef __APPCTRL_H
#define __APPCTRL_H

#include <string>
#include <map>
#include <vector>
#include <streams.h>
#include "GlobalDefine.h"
#include "IAuthorEngine.h"
#include "IDeviceInterface.h"
#include "IVideoPreviewObserver.h"
#include "IVideoOutputObserver.h"
#include "IAudioObserver.h"
#include "PreviewCtrlMgr.h"
#include "../LibDevice/DSDevice.h"
#include "../LibBase/MsgReceiver.h"

using namespace std;

struct DeviceNameDefineName
{
	string deviceName;
	string defineName;

	DeviceNameDefineName()
		:deviceName(""),
		defineName("")
	{

	}
};

typedef vector<DeviceNameDefineName> VecDeviceNameDefineName;
typedef vector<DeviceNameDefineName>::iterator VecDeviceNameDefineNameIter;

#define DVRECORDER_MUTEX_STRING			"MediaSwitcher"
class MultiPlayerSession;
class ISampleReceiver;
class SampleCoordinator;
class ISampleSender;
class VMR9Subgraph;
class MsgReceiver;

class AppCtrl:
		public IAuthorEngine,
		public IDeviceInterface,
		public IVideoPreviewObserver,
		public IVideoOutputObserver,
		public IAudioObserver,
		public MsgReceiver
{
public:
	static AppCtrl& GetInstance();

	void SelectCaptureSource(string name);
public:
	~AppCtrl(void);

private:
	AppCtrl(void);
	static AppCtrl* s_pAppCtrl;
	static void CreateInstance();
public:
	//for IAuthorEngine
	virtual BOOL CreateGraphCtrl(GraphType type, string fileName, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID);
	virtual BOOL CreateOutputGraphCtrl(GraphType graphType, OutputType outputType, string fileName,DWORD pushPort,char * pushUrl, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID);
	virtual BOOL DeleteGraphCtrl(DWORD_PTR& ctrlID);
	//IVideoPreviewObserver
	bool RegisterSampleReceiver(ISampleReceiver* pReveiver, DWORD id);
	bool UnRegisterSampleReceiver(ISampleReceiver* pReveiver, DWORD id);
	//IVideoOutputObserver
	bool RegisterOutPutReceiver(ISampleReceiver* pReveiver, DWORD id);
	bool UnRegisterOutPutReceiver();
	//IAudioObserver
	HRESULT UnRegisterAudioSender(ISampleSender* sender, string name);
	void SetAudioReceiverStatus(BOOL flag);
	//for IDeviceInterface
	virtual BOOL GetAudioDevice(DEVICE_LIST& audioDeviceList);
	virtual BOOL GetVideoDevice(DEVICE_LIST& videoDeviceList);
	virtual BOOL FindDevice(const char * inDeviceName, CDSDevice& outDevice);
	//for Callback
	void SetReceiverInVideoDevices(MsgReceiver* receiver);
	//for CFreeSpaceCtrl
	BOOL IsValidFileName(string fileName, int& freeSpaceCounter, bool flag=false);//flag=false: DV flag = true:wmv
    //for CVideoWindow
	BOOL SetNotifyWindow(VMR9Subgraph* inGraph, HWND inWindow);
	void HandleEvent(VMR9Subgraph* inGraph, WPARAM inWParam, LPARAM inLParam);

	void SetFileName(string name);
	void GetFileName(string& name);
    bool RegisterDevieNameDefineName(string deviceName, string defineName);
	bool RemoveDevieNameDefineName(const vector<string> vecDeviceName);
	bool CheckDevieName(string deviceName);
	bool CheckDefineName(string defineName);

	bool SetSampleReceiver();
	bool RegisterShowEngineCtrl(MultiPlayerSession* pMPSession);
	bool SignalShowEngine(BOOL signal);
	void  GetDV1Mode(int& mode);
	void  GetDV1FileName(string& filename);
	void  GetDV2Mode(int& mode);
	void  GetDV2FileName(string& filename);
	void  GetDV3Mode(int& mode);
	void  GetDV3FileName(string& filename);
	void  GetDV4Mode(int& mode);
	void  GetDV4FileName(string& filename);
	int  GetLogMode()
	{
		return m_LogMode;
	}

	void  GetDV1OutputMode(int& mode);
	void  GetDV2OutputMode(int& mode);
	void  GetDV3OutputMode(int& mode);

	void   GetDV1OutputFileName(string& filename);
	void   GetDV2OutputFileName(string& filename);
	void   GetDV3OutputFileName(string& filename);
	void   GetSwitchFileName(string& filename, bool flag = false);
	void   GetSwitchPathName(string& filename);
	void   GetBreakPointFileName(string& filename);
	void   GetLiveRecordFileName(string& filename);
	//for subtitle
	HRESULT   SetTextSubtitleBuffer(unsigned char* buffer, int width, int height, int PosX, int PosY);
	HRESULT   SetTextSubtitleShowFlag(BOOL flag);
	//for logo
	HRESULT SetLogoFileName(unsigned char* fileName);
	HRESULT SetLogoShowFlag(BOOL showFlag);
	//following methods are for show engine
	//for initialzation of show engine	
	HRESULT InitialPlayerSession(HWND windowHandle);
	//for video in show engine
	HRESULT AddVideoFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type);
	HRESULT AddVideoDeviceSource(Device_Type deviceType, string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type);
	//for audio in show engine
	HRESULT AddAudioFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type);
	HRESULT AddAudioDeviceSource(string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type);
	//for ctrl of show engine
	HRESULT RunVMR9Subgraph(DWORD_PTR dwID);
	HRESULT PauseVMR9Subgraph(DWORD_PTR dwID);
	HRESULT StopVMR9Subgraph(DWORD_PTR dwID);
	VMR9Subgraph* GetSubgraph(DWORD_PTR dwID);
	HRESULT RegisterReceiverInSubgraph(MsgReceiver* receiver, DWORD_PTR dwID);
	//get total time of the filter graph
	HRESULT GetTotalPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second);
	//get current time of the filter graph
	HRESULT GetCurrentPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second);
	//seek from current position
	HRESULT RelativeSeek(DWORD_PTR dwID, REFERENCE_TIME time);
	bool RegisterSampleReceiver(ISampleReceiver* pReveiver);
	bool UnRegisterSampleReceiver();

	bool SetOutPutID(DWORD_PTR dwID);

	HRESULT DeleteSource( DWORD_PTR dwID );
	CtrlStatus GetCtrlStatus( DWORD_PTR dwID );
	HRESULT SetCtrlStatus( DWORD_PTR dwID,CtrlStatus status );

	void SetOutputToFileFlag(BOOL flag);
	HRESULT SwitchAudioSender(string name);

	void PauseOutPutFile(string audioName, DWORD_PTR videoId);
	void PlayOutPutFile(string audioName, DWORD_PTR videoId);

	void GetOutputAudioSampleValueRate(int& value);
private:
	HRESULT UpdateVideoReceiverStatus(DWORD_PTR id, BOOL flag);
	HRESULT UpdateAudioReceiverStatus(BOOL flag);

	PreviewCtrlMgr*    m_CPreviewCtrlMgr;
	DEVICE_LIST         m_AudioDeiceList;
	DEVICE_LIST         m_VideoDeiceList;

	string m_SaveFileName;
	VecDeviceNameDefineName m_VecDeviceNameDefineName;
	int m_LogMode;
	ISampleReceiver* m_PushVideoReceiver; 

	SampleCoordinator* m_SampleCoordinator;
	BOOL m_bAudioReceiverCtrlStatus;
	string m_OutputFileName;
	BOOL m_bSetObserverInAudioCtrl;
};
#endif