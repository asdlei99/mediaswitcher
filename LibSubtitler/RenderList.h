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

	RenderList.h: Keeps track of and renders active text fragments.
*/
#ifndef f_RENDERLIST_H
#define f_RENDERLIST_H

#include <windows.h>

#include "List.h"
#include "../../filter.h"

class FontCache;
class FontDescriptor;
class DialogueItem;
class Rasterizer;

class RenderNode : public ListNode2<RenderNode> {
public:
	RenderNode *pLineStart;
	int			nLineWidth;
	int			nLineHeight;
	int			nHardBreaksAfter;
	int			nStartTime;

	Rasterizer		*pImageCache;
	const wchar_t	*pszText;
	wchar_t			*pszTextAlloc;
	int				nChars;
	RECT			r;
	int				xsplit;
	long			xsplitstart, xsplittimelen;		// in milliseconds
	long			xsplitdelay;
	long			rgbFore, rgbFore2, rgbBack;
	const char		*pszFontName;
	char			*pszAllocedName;
	int				nFontHeight;
	bool			fUnderline, fBold, fItalics, fOpaqueBox;
	long			nFontId;
	int				nAscent, nDescent, nBaseline;
	int				nBorder;
	int				nShadow;
	const void		*pKey;
	DWORD			dwCharSet;
	DWORD			dwCodePage;
	int				nXScrollRate;
	int				nYScrollRate;
	int				nYWindowPos;
	int				nYWindowSize;			// vertical scroll clippers

	RenderNode();
	~RenderNode();

	void Clear();
};

class RenderList {
public:
	enum eWrapMode {
		kWrapSmartAuto,
		kWrapAutomatic,
		kWrapManual
	};

private:
	List2<RenderNode> listActive;
	List2<RenderNode> listFree;
	RenderNode *pNodes;
	FontCache *pfc;

	int xmul, xdiv, ymul, ydiv;
	unsigned char	nGammaRamp[65];

	eWrapMode	mDefaultWrapMode;
	bool		mbAssumeGDISucks;

	bool our_isspace(char c) const;
	bool our_wisspace(wchar_t c) const;
	bool our_isnewline(const char *s, eWrapMode) const;
	bool our_isoverridestart(const char *s) const;

	RenderNode *AllocNode();
	void Delete(const void *pKey);
	int BreakIntoFragments(List2<RenderNode> &list, const DialogueItem *pdi, FontCache *pfc, HDC hdc, bool fAntialias, eWrapMode& nWrapMode);
	void Backward(RenderNode *&pnode, int& nChar, int nCount);
	void Forward(RenderNode *&pnode, int& nChar, int nCount);
	bool AtEnd(RenderNode *&pnode, int &nChar);
	SIZE SizeText(RenderNode *&pnode, int& nChar, RenderNode *pnode_limit, int nCharLimit, FontCache *pfc, HDC hdc, bool fAntialias, int &nMaxDescent, long nMaxWidth);
	void SplitAt(RenderNode *&pnode, int &nChar);
	int AttemptWordWrap(List2<RenderNode> &list, const DialogueItem *pdi, FontCache *pfc, HDC hdc, bool fAntialias, int &nMaxWidth, bool fReal);

public:
	RenderList(int maxactive, double gamma, bool bAssumeGDISucks, eWrapMode nDefaultWrapMode);
	~RenderList();

	int scalex(int i);
	int scaley(int i);
	int scaleya(int i);
	int scalea(int i);
	int scaleia(int i);

	void SetRenderingScale(int _xmul, int _xdiv, int _ymul, int _ydiv);
	void RemoveItem(const DialogueItem *pdi);
	void AddItem(const DialogueItem *pdi, FontCache *, /*VFBitmap *,*/int videoWidth, int videoHeight, HDC hdc, bool fMagnify);
	void RenderNormal(long ms, VFBitmap *, FontCache *);
	void RenderAntialiased(long ms, /*VFBitmap *vbm,*/int videoWidth, int videoHeight, int videoPitch, unsigned long* videoBuffer, FontCache *, HDC hdcAntialias);
	void Clear();
};
#endif