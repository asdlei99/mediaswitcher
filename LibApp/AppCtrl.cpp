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
#include <windows.h>
#include "AppCtrl.h"
#include "../LibBase/MsgReceiver.h"
#include "../LibMediaCtrl/MediaController.h"
#include "../LibUtility/PrintDebug.h"
#include "../LibUtility/IniSettingRead.h"
#include "../LibUtility/AudioCalculator.h"
#include "../LibUtility/FreeSpaceCtrl.h"
#include "../LibShowEngineCtrl/MultiPlayerSession.h"
#include "../LibShowEngineCtrl/SampleCoordinator.h"
#include "../LibShowEngineCtrl/AudioSendReceiverCtrl.h"
#include "../LibShowEngineCtrl/VMR9Subgraph.h"
#include "../LibDevice/AudioDevices.h"
#include "../LibDevice/VideoDevices.h"

class isInvalidDevice: public std::binary_function<DeviceNameDefineName, const vector<string>, bool>
{
public:
	result_type operator() (first_argument_type devicenamedefinename, second_argument_type vecDeviceName) const
	{
		vector<string>::const_iterator nameIter = find(vecDeviceName.begin(), vecDeviceName.end(), devicenamedefinename.deviceName);

		if (nameIter == vecDeviceName.end())
		{
			return (result_type) true;
		}
		return (result_type) false;
	}
};

AppCtrl* AppCtrl::s_pAppCtrl = NULL;

AppCtrl::AppCtrl(void)
{
	m_SaveFileName = "";
	m_LogMode  = 0;

	IniSettingRead	iniFileReader;
	iniFileReader.GetLogMode(m_LogMode);

	switch(m_LogMode)
	{
		case 1:
			m_LogMode = LOG_LEVEL0|LOG_LEVEL1;
			break;
		case 2:
			m_LogMode = LOG_LEVEL0|LOG_LEVEL1|LOG_LEVEL2;
			break;
		case 3:
			m_LogMode = LOG_LEVEL0|LOG_LEVEL1|LOG_LEVEL2|LOG_LEVEL3;
			break;
		case 4:
			m_LogMode = LOG_AVSYNC;
			break;
		case 0:
		default:
			m_LogMode = LOG_LEVEL0;
			break;
	}
	OpenDebugFile();
	SetDebugLevel(m_LogMode);

	m_SampleCoordinator = new SampleCoordinator;
	m_bAudioReceiverCtrlStatus = TRUE;
	m_CPreviewCtrlMgr = new PreviewCtrlMgr();	
	m_bSetObserverInAudioCtrl = FALSE;
}

AppCtrl::~AppCtrl(void)
{
	VideoDevices::Instance()->Release();
	AudioDevices::Instance()->Release();

	CloseDebugFile();

	if (m_SampleCoordinator)
	{
		delete m_SampleCoordinator;
		m_SampleCoordinator = NULL;
	}
	if(m_CPreviewCtrlMgr)
	{
		delete m_CPreviewCtrlMgr;
		m_CPreviewCtrlMgr = NULL;
	}
}

AppCtrl& AppCtrl::GetInstance()
{
	if (s_pAppCtrl == NULL)
	{
		CreateInstance();
	}
	return *s_pAppCtrl;
}

void AppCtrl::CreateInstance()
{
	static AppCtrl appctrl;
	s_pAppCtrl = &appctrl;
}

void AppCtrl::SelectCaptureSource(string name)
{
	CMediaController::GetInstance().SwitchSource(name);
}

void AppCtrl::SetFileName(string name)
{
	m_SaveFileName = name;
}

void AppCtrl::GetFileName(string& name)
{
	name = m_SaveFileName ;
}

bool AppCtrl::RegisterDevieNameDefineName(string deviceName, string defineName)
{
	DeviceNameDefineName item;
	item.deviceName = deviceName;
	item.defineName = defineName;
	
	char msg[MAX_PATH];
	sprintf(msg, "Register Device: devicename:%s definename:%s \n",deviceName.c_str(),defineName.c_str());
	DebugMessage(LOG_LEVEL1, msg);

	m_VecDeviceNameDefineName.push_back(item);
	return true;
}

bool AppCtrl::RemoveDevieNameDefineName(const vector<string> vecDeviceName)
{
	char msg[MAX_PATH];
	DebugMessage(LOG_LEVEL1, "Device in system:************************\n");
	if (vecDeviceName.size() > 0)
	{
		for (int i = 0; i < vecDeviceName.size(); i++)
		{	
			sprintf(msg, "Register Device: devicename:%s \n",vecDeviceName[i].c_str());
			DebugMessage(LOG_LEVEL0, msg);
		}
	}

	m_VecDeviceNameDefineName.erase(remove_if(m_VecDeviceNameDefineName.begin(), m_VecDeviceNameDefineName.end(), bind2nd(isInvalidDevice(),vecDeviceName)), m_VecDeviceNameDefineName.end());
	return true;
}

bool AppCtrl::CheckDevieName(string deviceName)
{
	VecDeviceNameDefineNameIter iter = m_VecDeviceNameDefineName.begin();
	while (iter != m_VecDeviceNameDefineName.end())
	{
		if ((*iter).deviceName == deviceName)
		{
			return true;
		}
		iter++;
	}
	return false;
}

bool AppCtrl::CheckDefineName(string defineName)
{
	VecDeviceNameDefineNameIter iter = m_VecDeviceNameDefineName.begin();
	while (iter != m_VecDeviceNameDefineName.end())
	{
		if ((*iter).defineName == defineName)
		{
			return true;
		}
		iter++;
	}
	return false;
}

bool AppCtrl::SetSampleReceiver()
{
	MultiPlayerSession::GetInstance().SetSampleReceiver(m_SampleCoordinator);

	return true;
}

bool AppCtrl::SignalShowEngine(BOOL signal)
{
	HRESULT hr = MultiPlayerSession::GetInstance().SignalShowEngine(signal);	
	if (SUCCEEDED(hr))
	{
		return true;
	}
	return false;		
}
#define GETDVMODE(DV)\
void  AppCtrl::Get##DV##Mode(int& mode)\
{\
	IniSettingRead	iniFileReader;\
	int size = 0;\
	iniFileReader.Get##DV##Mode(size);\
	mode = size;\
}\

#define GETDVFILENAME(DV)\
void  AppCtrl::Get##DV##FileName(string& filename)\
{\
	string name ="";\
	IniSettingRead	iniFileReader;\
	iniFileReader.Get##DV##FileName(name);\
	filename = name;\
}\

#define GETDVOUTPUTFILENAME(DV)\
void   AppCtrl::Get##DV##OutputFileName(string& filename)\
{\
	string name ="";\
	IniSettingRead	iniFileReader;\
	iniFileReader.Get##DV##OutputFileName(name);\
	filename = name;\
}\

#define GETDVOUTPUTMODE(DV)\
void  AppCtrl::Get##DV##OutputMode(int& mode)\
{\
	IniSettingRead	iniFileReader;\
	int size = 0;\
	iniFileReader.Get##DV##OutputMode(size);\
	mode = size;\
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

void   AppCtrl::GetSwitchPathName(string& filename)
{
	string name ="";
	IniSettingRead	iniFileReader;
	iniFileReader.GetSwitchOutputFileName(name);
	filename = name;
}

void   AppCtrl::GetBreakPointFileName(string& filename)
{
	int pos = m_OutputFileName.find(".");
	filename = m_OutputFileName.substr(0, pos+1);
	filename += "txt";
}

void   AppCtrl::GetLiveRecordFileName(string& filename)
{
	int pos = m_OutputFileName.find(".");
	filename = m_OutputFileName.substr(0, pos+1);
	filename += "csv";
}

void   AppCtrl::GetSwitchFileName(string& filename, bool flag)
{
	string name =filename;

	SYSTEMTIME systime;
	GetLocalTime(&systime);
	int pos = name.find("\\");
	char systimeStr[MAX_PATH];
	if (flag == false)
	{
		sprintf(systimeStr,"%02d%02d%02d%02d%02d%02d.avi\0",systime.wYear, systime.wMonth, systime.wDay,
							systime.wHour, systime.wMinute, systime.wSecond);
	}
	else
	{
		sprintf(systimeStr,"%02d%02d%02d%02d%02d%02d.wmv\0",systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond);

	}
	if (pos != string::npos)
		filename = name.substr(0, pos+2)+systimeStr;

	m_OutputFileName = filename;
}


HRESULT   AppCtrl::SetTextSubtitleBuffer(unsigned char* buffer, int width, int height, int PosX, int PosY)
{
	return MultiPlayerSession::GetInstance().SetTextSubtitleBuffer(buffer, width, height, PosX, PosY);	
}

HRESULT   AppCtrl::SetTextSubtitleShowFlag(BOOL flag)
{
	return MultiPlayerSession::GetInstance().SetTextSubtitleShowFlag(flag);	
}

HRESULT AppCtrl::SetLogoFileName(unsigned char* fileName)
{
	return MultiPlayerSession::GetInstance().SetLogoFileName(fileName);
}
HRESULT AppCtrl::SetLogoShowFlag(BOOL showFlag)
{
	return MultiPlayerSession::GetInstance().SetLogoShowFlag(showFlag);	
}

HRESULT  AppCtrl::InitialPlayerSession(HWND windowHandle)
{
	return 	MultiPlayerSession::GetInstance().Initialize(windowHandle);
}

HRESULT  AppCtrl::AddVideoFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type)
{
	if (!m_bSetObserverInAudioCtrl)
	{
		CAudioSendReceiverCtrl::GetInstance().SetAudioSenderCtrlObserver(this);
		m_bSetObserverInAudioCtrl = TRUE;
	}

	HRESULT hr = MultiPlayerSession::GetInstance().AddVideoFileSource(this, sourceName, flag, wcPath, dwID, type);
	return hr;
}

HRESULT  AppCtrl::AddVideoDeviceSource(Device_Type deviceType, string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type)
{
	if (!m_bSetObserverInAudioCtrl)
	{
		CAudioSendReceiverCtrl::GetInstance().SetAudioSenderCtrlObserver(this);
		m_bSetObserverInAudioCtrl = TRUE;
	}

	return 	MultiPlayerSession::GetInstance().AddVideoDeviceSource(this,deviceType, sourceName, flag, deviceName, dwID, type);
}

HRESULT AppCtrl::AddAudioFileSource(string sourceName, BOOL flag, WCHAR *wcPath, DWORD_PTR& dwID, GraphType& type)
{
	if (!m_bSetObserverInAudioCtrl)
	{
		CAudioSendReceiverCtrl::GetInstance().SetAudioSenderCtrlObserver(this);
		m_bSetObserverInAudioCtrl = TRUE;
	}

	HRESULT hr = MultiPlayerSession::GetInstance().AddAudioFileSource(this,sourceName, flag, wcPath, dwID, type);
	return hr;
}

HRESULT AppCtrl::AddAudioDeviceSource(string sourceName, BOOL flag, string deviceName, DWORD_PTR& dwID, GraphType& type)
{
	if (!m_bSetObserverInAudioCtrl)
	{
		CAudioSendReceiverCtrl::GetInstance().SetAudioSenderCtrlObserver(this);
		m_bSetObserverInAudioCtrl = TRUE;
	}

	HRESULT hr = MultiPlayerSession::GetInstance().AddAudioDeviceSource(this,sourceName, flag, deviceName, dwID, type);
	return hr;
}
//get total time of the filter graph
HRESULT AppCtrl::GetTotalPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second)
{
	HRESULT hr = MultiPlayerSession::GetInstance().GetTotalPlaybackTime(dwID, hour, minute, second);
	return hr;
}
//get current time of the filter graph
HRESULT AppCtrl::GetCurrentPlaybackTime(DWORD_PTR dwID, int& hour, int& minute, int& second)
{
	HRESULT hr = MultiPlayerSession::GetInstance().GetCurrentPlaybackTime(dwID, hour, minute, second);
	return hr;
}

HRESULT AppCtrl::RelativeSeek(DWORD_PTR dwID, REFERENCE_TIME time)
{
	HRESULT hr = MultiPlayerSession::GetInstance().RelativeSeek(dwID, time);
	return hr;
}

HRESULT  AppCtrl::RunVMR9Subgraph(DWORD_PTR dwID)
{
	HRESULT hr = MultiPlayerSession::GetInstance().RunVMR9Subgraph(dwID);
	return hr;
}

HRESULT AppCtrl::PauseVMR9Subgraph(DWORD_PTR dwID)
{
	HRESULT hr = MultiPlayerSession::GetInstance().PauseVMR9Subgraph(dwID);
	return hr;
}

HRESULT AppCtrl::StopVMR9Subgraph(DWORD_PTR dwID)
{
	HRESULT hr = MultiPlayerSession::GetInstance().StopVMR9Subgraph(dwID);
	return hr;
}

VMR9Subgraph* AppCtrl::GetSubgraph(DWORD_PTR dwID)
{
	return MultiPlayerSession::GetInstance().GetSubgraph(dwID);
}

HRESULT AppCtrl::RegisterReceiverInSubgraph(MsgReceiver* receiver, DWORD_PTR dwID)
{
	HRESULT hr = MultiPlayerSession::GetInstance().RegisterReceiverInSubgraph(receiver, dwID);
	return hr;
}

bool AppCtrl::RegisterSampleReceiver(ISampleReceiver* pReveiver, DWORD id)
{
	if (pReveiver == NULL)
	{
		return false;
	}
	HRESULT hr = m_SampleCoordinator->SetReceiver(pReveiver, id);
	if (SUCCEEDED(hr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AppCtrl::RegisterSampleReceiver(ISampleReceiver* pReveiver)
{
	if (pReveiver == NULL)
	{
		return false;
	}
	HRESULT hr = m_SampleCoordinator->SetLargeWindowReceiver(pReveiver);
	if (SUCCEEDED(hr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AppCtrl::UnRegisterSampleReceiver()
{
	HRESULT hr = m_SampleCoordinator->RemoveLargeWindowReceiver();
	if (SUCCEEDED(hr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AppCtrl::UnRegisterSampleReceiver(ISampleReceiver* pReveiver, DWORD id)
{
	if (pReveiver == NULL)
	{
		return false;
	}
	m_SampleCoordinator->RemoveReceiver(pReveiver, id);
	return true;
}

bool AppCtrl::RegisterOutPutReceiver(ISampleReceiver* pReveiver, DWORD id)
{
	if (pReveiver == NULL)
	{
		return false;
	}
	m_SampleCoordinator->SetFileOutPutReceiver(pReveiver);
	return true;
}

bool AppCtrl::UnRegisterOutPutReceiver()
{
	m_SampleCoordinator->RemoveFileOutPutReceiver();

	return true;
}


bool  AppCtrl::SetOutPutID(DWORD_PTR dwID)
	{
	if (m_SampleCoordinator)
	{
		m_SampleCoordinator->SetOutPutID(dwID);
	}

	MultiPlayerSession::GetInstance().SetOutPutID(dwID);

	return true;
}

HRESULT AppCtrl::DeleteSource( DWORD_PTR dwID )
{
	return MultiPlayerSession::GetInstance().DeleteSource(dwID);
}

CtrlStatus AppCtrl::GetCtrlStatus( DWORD_PTR dwID )
{
	return MultiPlayerSession::GetInstance().GetCtrlStatus(dwID);
}

HRESULT AppCtrl::SetCtrlStatus( DWORD_PTR dwID,CtrlStatus status )
{
	return MultiPlayerSession::GetInstance().SetCtrlStatus(dwID, status);
}

void AppCtrl::SetOutputToFileFlag(BOOL flag)
{
	if (m_SampleCoordinator)
	{
		m_SampleCoordinator->SetOutputToFileFlag(flag);
	}

	CAudioSendReceiverCtrl::GetInstance().SetOutputToFileFlag(flag);
}

HRESULT AppCtrl::SwitchAudioSender(string name)
{
	return CAudioSendReceiverCtrl::GetInstance().SwitchSender(name);
}

HRESULT AppCtrl::UpdateVideoReceiverStatus(DWORD_PTR id, BOOL flag)
{
	return m_SampleCoordinator->UpdateReceiverStatus(id, flag);
}

HRESULT AppCtrl::UpdateAudioReceiverStatus(BOOL flag)
{
	return CAudioSendReceiverCtrl::GetInstance().UpdateReceiverStatus(flag);
}

HRESULT AppCtrl::UnRegisterAudioSender(ISampleSender* sender, string name)
{
	if (m_bAudioReceiverCtrlStatus)
	{
		HRESULT hr = CAudioSendReceiverCtrl::GetInstance().UnRegisterSender(sender, name);
		return hr;
	}
	return E_FAIL;
}

void AppCtrl::SetAudioReceiverStatus(BOOL flag)
{
	m_bAudioReceiverCtrlStatus = flag;
}

void AppCtrl::PauseOutPutFile(string audioName, DWORD_PTR videoId)
{
	if (m_SampleCoordinator)
	{
		m_SampleCoordinator->PauseOutPutVideoSample(videoId);
	}

	CAudioSendReceiverCtrl::GetInstance().PauseOutPutAudioSample(audioName);
}

void AppCtrl::PlayOutPutFile(string audioName, DWORD_PTR videoId)
{	
	CAudioSendReceiverCtrl::GetInstance().PlayOutPutAudioSample(audioName);
	if (m_SampleCoordinator)
	{
		m_SampleCoordinator->PlayOutPutVideoSample(videoId);
	}
}

void AppCtrl::GetOutputAudioSampleValueRate(int& value)
{
	CAudioCalculator::GetInstance().GetAudioSampleValue(value);
}

//for IAuthorEngine
BOOL AppCtrl::CreateGraphCtrl(GraphType type, string fileName, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID)
{
	BOOL ret = FALSE;

	if ((type != AUDIO_ONLY)&&(type != VIDEO_ONLY)
		&&(type != AUDIO_VIDEO_BOTH))
		return FALSE;

	ret = m_CPreviewCtrlMgr->CreatePreviewCtrl(this, type, hwnd, dwId, retCtrlID);
	if (ret)
	{
		if (type == VIDEO_ONLY)
		{	
			UpdateVideoReceiverStatus(dwId, TRUE);
		}
		else if (type == AUDIO_ONLY)
		{
			UpdateAudioReceiverStatus(TRUE);
		}
	}
	return ret;
}

BOOL AppCtrl::CreateOutputGraphCtrl(GraphType graphType, OutputType outputType, string fileName,DWORD pushPort,char * pushUrl, HWND hwnd, DWORD_PTR dwId, DWORD_PTR& retCtrlID)
{
	BOOL ret = FALSE;
	
	if(graphType != SAVE_FILE_GRAPH)
		return FALSE;

	ret = m_CPreviewCtrlMgr->CreateOutPutGraphCtrl(this, outputType, retCtrlID, fileName, pushPort, pushUrl);

	return ret;
}

BOOL AppCtrl::DeleteGraphCtrl(DWORD_PTR& ctrlID)
{
	if (ctrlID)
	{	
		BOOL ret =  m_CPreviewCtrlMgr->DeletePreviewCtrl(ctrlID);
		if (ret)
		{
			ctrlID = 0;
			return TRUE;
		}		
	}
	return FALSE;
}

BOOL AppCtrl::GetAudioDevice(DEVICE_LIST& audioDeviceList)
{
	m_AudioDeiceList.clear();
	audioDeviceList.clear();
	DEVICE_LIST::iterator iter = AudioDevices::Instance()->mList.begin();
	for(; iter != AudioDevices::Instance()->mList.end(); iter++)
	{
		audioDeviceList.push_back(*iter);
		m_AudioDeiceList.push_back(*iter);
	}
	return TRUE;
}

BOOL AppCtrl::GetVideoDevice(DEVICE_LIST& videoDeviceList)
{
	m_VideoDeiceList.clear();
	videoDeviceList.clear();
	DEVICE_LIST::iterator iter = VideoDevices::Instance()->mList.begin();
	for(; iter != VideoDevices::Instance()->mList.end(); iter++)
	{
		videoDeviceList.push_back(*iter);
		m_VideoDeiceList.push_back(*iter);
	}
	return TRUE;
}

BOOL AppCtrl::FindDevice(const char * inDeviceName, CDSDevice& outDevice)
{
	return VideoDevices::Instance()->FindDevice(inDeviceName, outDevice);
}

void AppCtrl::SetReceiverInVideoDevices(MsgReceiver* receiver)
{
	VideoDevices::Instance()->AddMsgReceiver(receiver);
}

BOOL AppCtrl::IsValidFileName(string fileName, int& freeSpaceCounter, bool flag)//flag=false: DV flag = true:wmv
{
	return CFreeSpaceCtrl::GetInstance().IsValidFileName(fileName.c_str(), freeSpaceCounter, flag);
}


BOOL AppCtrl::SetNotifyWindow(VMR9Subgraph* inGraph, HWND inWindow)
{
	if (inGraph)
	{
		inGraph->SetNotifyWindow(inWindow);
		return TRUE;
	}
	return FALSE;
}

void AppCtrl::HandleEvent(VMR9Subgraph* inGraph, WPARAM inWParam, LPARAM inLParam)
{
	if (inGraph)
	{
		inGraph->HandleEvent(inWParam, inLParam);
	}
}
