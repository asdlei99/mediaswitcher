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
#include "PreviewCtrlMgr.h"
#include "../LibDShowCtrl/PreviewVideoController.h"
#include "../LibDShowCtrl/PreviewAudioController.h"
#include "../LibDShowCtrl/SaveFileController.h"

PreviewCtrlMgr::PreviewCtrlMgr()
{

}

PreviewCtrlMgr::~PreviewCtrlMgr()
{

}

BOOL PreviewCtrlMgr::CreateOutPutGraphCtrl(IVideoOutputObserver* observer, OutputType outputType, DWORD& retCtrlID, string fileName,DWORD pushPort,char * pushUrl)
{
	GraphController* saveFileCtrl = new SaveFileController(observer, outputType, fileName,pushPort,pushUrl);
	if (saveFileCtrl == FALSE)
		return FALSE;

	BOOL activeStatus = saveFileCtrl->Activate();
	if (activeStatus == FALSE)
	{
		delete saveFileCtrl;
		return FALSE;
	}

	PreviewCtrlInfo ctrlInfo;
	ctrlInfo.dwID = (DWORD)saveFileCtrl;
	ctrlInfo.graphCtrl = saveFileCtrl;

	retCtrlID = ctrlInfo.dwID;
	m_vecPreviewCtrl.push_back(ctrlInfo);
	return TRUE;
}

BOOL PreviewCtrlMgr::CreatePreviewCtrl(IVideoPreviewObserver* observer, GraphType graphType, HWND hwnd, DWORD dwId, DWORD& retCtrlID)
{
	GraphController* previewCtrl = NULL;
	retCtrlID = 0;
	if (graphType == VIDEO_ONLY)
	{
		previewCtrl = new PreviewVideoController(observer, dwId);
		if (previewCtrl == FALSE)
			return FALSE;
		previewCtrl->SetVideoDisplayHwnd(hwnd);
	}
	else if(graphType == AUDIO_ONLY)
	{
		previewCtrl = new PreviewAudioController;
		if (previewCtrl == FALSE)
			return FALSE;
	}
		 
	BOOL activeStatus = previewCtrl->Activate();
	if (activeStatus == FALSE)
	{
		delete previewCtrl;
		return FALSE;
	}

	PreviewCtrlInfo ctrlInfo;
	ctrlInfo.dwID = (DWORD)previewCtrl;
	ctrlInfo.graphCtrl = previewCtrl;

	retCtrlID = ctrlInfo.dwID;
	m_vecPreviewCtrl.push_back(ctrlInfo);
	return TRUE;
}

BOOL PreviewCtrlMgr::DeletePreviewCtrl(DWORD ctrlID)
{
	int i = 0;
	vector<PreviewCtrlInfo>::iterator iter = m_vecPreviewCtrl.begin();
	for (; iter < m_vecPreviewCtrl.end(); iter++)
	{
		if (iter->dwID == ctrlID)
		{
			delete iter->graphCtrl;
			m_vecPreviewCtrl.erase(iter);
			return TRUE;
		}
	}
	return FALSE;
}
