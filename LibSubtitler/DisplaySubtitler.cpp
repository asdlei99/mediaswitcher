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

DisplaySubtitler.cpp: provide the wrapper for MediaSwitcher to support subtitle composition.
*/
#include "DisplaySubtitler.h"

bool g_bUseMMX;

DisplaySubtitler::DisplaySubtitler(void)
{
	m_bInit = false;
	m_CanvasWidth = 0;
	m_CanvasHeight = 0;
	
	m_Antialias = false;	
	
	m_Dlist = NULL;
	m_Pfc = NULL;
	m_Render = NULL;	
}

DisplaySubtitler::~DisplaySubtitler(void)
{
	if(m_Dlist)
		delete m_Dlist;
		
	if(m_Pfc)
		delete m_Pfc;
		
	if(m_Render)
		delete m_Render;
		
	DeleteDC(m_HdcAntialias);
}

long (*g_pGetHostVerInfo)(char *buf, int len);

static long NullGetHostVerInfo(char *buf, int len) {
	buf[0] = 0;
	return 0;
}

static char sTextBuffer[MAX_PATH];

void DisplaySubtitler::Init(int width, int height)
{
	if(m_bInit)
		return;

	g_pGetHostVerInfo = NullGetHostVerInfo;

	m_Antialias = true;

	g_bUseMMX = true;

	bool bAssumeGDISucks = (LONG)GetVersion() < 0;

	m_Dlist = new DialogueList();
	if (!m_Dlist)
		return;

	int nWrapMode = 0;

	m_Render = new RenderList(128, 1.4, bAssumeGDISucks, (RenderList::eWrapMode)nWrapMode);

	if (!m_Render)
		return;

	// If Antialiasing is enabled, create a display context for us to draw text
	// paths in.

	if (m_Antialias) {
		m_HdcAntialias = CreateCompatibleDC(NULL);
		SetTextAlign(m_HdcAntialias, TA_TOP|TA_LEFT);
		SetTextColor(m_HdcAntialias, 0xffffff);
		SetBkColor(m_HdcAntialias, 0);
		SetBkMode(m_HdcAntialias, TRANSPARENT);
	}

	m_Pfc = new FontCache(m_HdcAntialias, 8);
	if (!m_Pfc)
		return;


	m_CanvasWidth = width;
	m_CanvasHeight = height;
}

void DisplaySubtitler::SelectStyle(const DialogueStyleDefinition* style)
{
	if(m_bInit)
		return;

	DialogueItem* pdi;
	if (!(pdi = new DialogueItem)) {
		throw "[SSA v2 parser] out of memory";
	}

	pdi->order		= 0;
	pdi->text		= sTextBuffer;
	pdi->start_ms	= 0  * 3600000
		+ 0 *   60000
		+ 0 *    1000
		+ 0  *      10;
	pdi->duration_ms=(0  * 3600000
		+ 0 *   60000
		+ 0 *    1000
		+ 0  *      10) - pdi->start_ms;
	pdi->nLeftMargin	= 0;
	pdi->nRightMargin	= 0;
	pdi->nBottomMargin	= 0;
	pdi->nXScrollRate	= 0;
	pdi->nYScrollRate	= 0;
	pdi->nYWindowPos	= 0;
	pdi->nYWindowSize	= 0;

	m_Dlist->AddCurDialogueItem(pdi);

	m_Dlist->Start();

	m_Dlist->AddCurStyle(style);

	TCHAR testText[MAX_PATH] = {"我是Chinese, 字幕叠加测试中！"};
	pdi = m_Dlist->GetCurDialogueItem();
	pdi->pStyle = (DialogueStyle *)style;//sfd->m_Dlist->GetCurStyle();
	pdi->pDSD = (DialogueStyleDefinition *)style;

	lstrcpy(pdi->text, TEXT("我是Chinese, 字幕叠加测试中！"));

	pdi->nLeftMargin = style->nLeftMargin;
	pdi->nRightMargin = style->nRightMargin;
	pdi->nBottomMargin = style->nBottomMargin;

	m_Render->Clear();//clear last rendernode
	m_Render->AddItem(pdi, m_Pfc, m_CanvasWidth, m_CanvasHeight, m_HdcAntialias, m_Antialias);

	m_bInit = true;
}

void DisplaySubtitler::Render(int videoWidth, int videoHeight, int videoPitch, unsigned long* videoBuffer)
{
	static int loop = 0;

	if (m_Antialias)
		m_Render->RenderAntialiased(loop, videoWidth, videoHeight, videoPitch, videoBuffer, m_Pfc, m_HdcAntialias);

	GdiFlush();
	_CrtCheckMemory();
}
