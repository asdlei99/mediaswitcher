/*	Subtitler - Sub Station Alpha v2.x/4.x subtitling plugin for VirtualDub
	Copyright (C) 2000-2002 Avery Lee, All Rights Reserved.

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

	FontCache.cpp: Attempts to keep a cache of fonts to avoid font
				   rerendering.
*/

#include <stdlib.h>
#include <crtdbg.h>
#include <string.h>

//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#include <windows.h>

#include "FontCache.h"

///////////////////////////////////////////////////////////////////////////

class FontDescriptor {
private:
	TCHAR		name[64];
	int			nHeight;
	DWORD		dwCharSet;
	long		id;

	bool		fItalics;
	bool		fUnderline;
	bool		fBold;

	HGDIOBJ			hgoOld;
	HFONT			hFont;

public:
	FontDescriptor();
	~FontDescriptor();

	long getID() { return id; }
	bool isEquiv(const char *_name, int _nHeight, bool _fItalics, bool _fUnderline, bool _fBold, DWORD dwCharSet);
	void Assume(long id, const TCHAR *_name, int _nHeight, bool _fItalics, bool _fUnderline, bool _fBold, DWORD dwCharSet);
	bool Realize(HDC);
	void Unrealize(HDC);
	void Discard();
};

FontDescriptor::FontDescriptor() {
}

FontDescriptor::~FontDescriptor() {
	Discard();
}

bool FontDescriptor::isEquiv(const char *_name, int _nHeight, bool _fItalics, bool _fUnderline, bool _fBold, DWORD _dwCharSet) {
	return !strcmp(_name, name) && _nHeight == nHeight && _fItalics == fItalics && _fUnderline == fUnderline && _fBold == fBold && _dwCharSet == dwCharSet;
}

void FontDescriptor::Assume(long _id, const TCHAR *_name, int _nHeight, bool _fItalics, bool _fUnderline, bool _fBold, DWORD _dwCharSet) {
	lstrcpy(name, _name);
	nHeight		= _nHeight;
	fItalics	= _fItalics;
	fUnderline	= _fUnderline;
	fBold		= _fBold;
	id			= _id;
	dwCharSet	= _dwCharSet;
}

bool FontDescriptor::Realize(HDC hdc) {
	if (!hFont) {
		hFont = CreateFont(nHeight, 0, 0, 0, fBold ? FW_BOLD : FW_NORMAL, fItalics, fUnderline, FALSE,
				dwCharSet, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE,
				name);

		if (!hFont)
			return false;
	}

	hgoOld = SelectObject(hdc, hFont);

	return true;
}

void FontDescriptor::Unrealize(HDC hdc) {
	SelectObject(hdc, hgoOld);
}

void FontDescriptor::Discard() {
	if (hFont) {
		DeleteObject(hFont);
		hFont = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////

FontCache::FontCache(HDC hdc, int max) {
	nFontCount = max;
	pFonts = new FontDescriptor [nFontCount];
	pRealized = NULL;
	this->hdc = hdc;
	next_id = 0;
}

FontCache::~FontCache() {
	Deselect();

	delete[] pFonts;
}

long FontCache::Select(long id, const TCHAR *szName, int ptsize, bool fItalics, bool fUnderline, bool fBold, DWORD dwCharSet) {
	int i;
	int oldest = 0;

	// Already realized?

	if (pRealized && id == pRealized->getID())
		return id;

	// Deselect current font

	Deselect();

	for(i=0; i<nFontCount; i++)
		if (pFonts[i].isEquiv(szName, ptsize, fItalics, fUnderline, fBold, dwCharSet)) {

			if (pFonts[i].Realize(hdc)) {
				pRealized = &pFonts[i];
				return pFonts[i].getID();
			}
			return -1;
		} else if (pFonts[i].getID() < pFonts[oldest].getID())
			oldest = i;

	// cache miss - delete lowest font, add this font and realize

	pFonts[oldest].Discard();
	pFonts[oldest].Assume(next_id++, szName, ptsize, fItalics, fUnderline, fBold, dwCharSet);

	if (pFonts[oldest].Realize(hdc)) {
		pRealized = &pFonts[oldest];
		return true;
	}
	return false;
}

void FontCache::Deselect() {
	if (pRealized) {
		pRealized->Unrealize(hdc);
		pRealized = NULL;
	}
}
