/*	Subtitler - Sub Station Alpha v2.x/4.x subtitling plugin for VirtualDub
	Copyright (C) 2000 Avery Lee, All Rights Reserved.

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

	FontCache.h: Attempts to maintain a cache of frequently used fonts.
*/

#ifndef f_SUBTITLER_FONTCACHE_H
#define f_SUBTITLER_FONTCACHE_H

#include <windows.h>

class FontDescriptor;

class FontCache {
private:
	FontDescriptor *pFonts;
	int nFontCount;
	FontDescriptor *pRealized;
	HDC hdc;
	long next_id;

public:
	FontCache(HDC hdc, int max);
	~FontCache();

	long Select(long id, const TCHAR *szName, int ptsize, bool fItalics, bool fUnderline, bool fBold, DWORD dwCharSet);
	void Deselect();
};

#endif
