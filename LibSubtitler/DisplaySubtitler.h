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

DisplaySubtitler.h: provide the wrapper for MediaSwitcher to support subtitle composition.
*/
#ifndef f_DISPLAYSUBTITLER_H
#define f_DISPLAYSUBTITLER_H

#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "filter.h"
#include "DialogueList.h"
#include "RenderList.h"
#include "FontCache.h"
#include "List.h"

class DisplaySubtitler
{
public:
	DisplaySubtitler(void);
	~DisplaySubtitler(void);

	void Init(int width, int height);
	void SelectStyle(const DialogueStyleDefinition* style);
	void Render(int videoWidth, int videoHeight, int videoPitch, unsigned long* videoBuffer);

private:
	bool m_bInit;
	DialogueList *m_Dlist;
	FontCache *m_Pfc;

	RenderList *m_Render;
	HDC m_HdcAntialias;
	bool	m_Antialias;

	int m_CanvasWidth;
	int m_CanvasHeight;
};

#endif