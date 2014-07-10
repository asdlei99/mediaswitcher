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

	RenderList.cpp: Keeps track of and renders active text fragments.
*/

#include <stdlib.h>
#include <crtdbg.h>
#include <string.h>
#include <math.h>

//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#include <windows.h>

#include "RenderList.h"
#include "DialogueList.h"
#include "FontCache.h"
#include "Rasterizer.h"

extern long (*g_pGetHostVerInfo)(char *buf, int len);

unsigned char	bitcnt[256],		// bit count
				bitexl[256],		// expand to left bit
				bitexr[256],		// expand to right bit
				bitexp[7][256];		// expand center by 1-8 bits

void InitBitMasks() {
	static bool fInited = false;

	if (!fInited) {
		int i;

		for(i=0; i<256; i++) {
			unsigned char b=0, l=0, r=0, v;

			if (i&  1) { b=1; l|=0x01; r|=0xFF; }
			if (i&  2) { ++b; l|=0x03; r|=0xFE; }
			if (i&  4) { ++b; l|=0x07; r|=0xFC; }
			if (i&  8) { ++b; l|=0x0F; r|=0xF8; }
			if (i& 16) { ++b; l|=0x1F; r|=0xF0; }
			if (i& 32) { ++b; l|=0x3F; r|=0xE0; }
			if (i& 64) { ++b; l|=0x7F; r|=0xC0; }
			if (i&128) { ++b; l|=0xFF; r|=0x80; }

			v = i;
			bitexp[0][i] = v = v | (i<<1) | (i>>1);
			bitexp[1][i] = v = v | (i<<2) | (i>>2);
			bitexp[2][i] = v = v | (i<<3) | (i>>3);
			bitexp[3][i] = v = v | (i<<4) | (i>>4);
			bitexp[4][i] = v = v | (i<<5) | (i>>5);
			bitexp[5][i] = v = v | (i<<6) | (i>>6);
			bitexp[6][i] = v = v | (i<<7) | (i>>7);

			bitcnt[i] = b;
			bitexl[i] = l;
			bitexr[i] = r;
		}

		fInited = true;
	}
}

long revcolor(long c) {
	return ((c&0xff0000)>>16) + (c&0xff00) + ((c&0xff)<<16);
}

bool RenderList::our_isspace(char c) const {
//	return c!=(char)0xA0 && isspace((unsigned char)c);
	return c == ' ';
}

bool RenderList::our_wisspace(wchar_t c) const {
//	return c!=(char)0xA0 && isspace((unsigned char)c);
	return c == L' ';
}

bool RenderList::our_isnewline(const char *s, eWrapMode mode) const {
	return s[0]=='\\' && (s[1]=='N' || (s[1]=='n' && mode == kWrapManual));
}

bool RenderList::our_isoverridestart(const char *s) const {
	return *s=='{';
}

////////////////////////////////////

RenderNode::RenderNode()
: pszTextAlloc(NULL)
, pszAllocedName(NULL)
, pImageCache(NULL)
{
}

RenderNode::~RenderNode() {
	Clear();
}

void RenderNode::Clear() {
	delete pImageCache;		pImageCache = NULL;
	free(pszAllocedName);	pszAllocedName = NULL;
	delete[] pszTextAlloc;	pszTextAlloc = NULL;
}

////////////////////////////////////

RenderList::RenderList(int maxactive, double gamma, bool bAssumeGDISucks, eWrapMode nWrapDefault)
: mbAssumeGDISucks(bAssumeGDISucks)
, mDefaultWrapMode(nWrapDefault)
{
	int i;

	pNodes = new RenderNode[maxactive];

	for(i=0; i<maxactive; i++)
		listFree.AddTail(pNodes + i);

	xmul = ymul = xdiv = ydiv = 1;

	InitBitMasks();

	for(i=0; i<=64; i++)
		nGammaRamp[i] = 64.0 * pow((double)i / 64.0, 1.0 / gamma) + 0.5;
}

RenderList::~RenderList() {
	delete[] pNodes;
}

////////////////////////////////////

void RenderList::SetRenderingScale(int _xmul, int _xdiv, int _ymul, int _ydiv) {
	xmul = _xmul;
	xdiv = _xdiv;
	ymul = _ymul;
	ydiv = _ydiv;
}

int RenderList::scalex(int i) {
	return (i*xmul + xdiv - 1) / xdiv;
}

int RenderList::scaley(int i) {
	return (i*ymul + ydiv - 1) / ydiv;
}

int RenderList::scaleya(int i) {
	return (i*ymul*64 + ydiv - 1) / ydiv;
}

int RenderList::scalea(int i) {
	return i*64;
}

int RenderList::scaleia(int i) {
	return (i+63)/64;
}

////////////////////////////////////

RenderNode *RenderList::AllocNode() {
	return listFree.RemoveHead();
}

void RenderList::Delete(const void *pKey) {
	RenderNode *pnode, *pnode_next;

	pnode = listActive.AtHead();
	while(pnode_next = pnode->NextFromHead()) {
		if (pnode->pKey == pKey) {
			pnode->Remove();
			pnode->Clear();
			listFree.AddHead(pnode);
		}

		pnode = pnode_next;
	}
}

void RenderList::Clear() {
	RenderNode *pnode;

	while(pnode = listActive.RemoveTail()) {
		pnode->Clear();
		listFree.AddHead(pnode);
	}
}

////////////////////////////////////

void RenderList::RemoveItem(const DialogueItem *pdi) {
	Delete(pdi);
}

static DWORD CharSetToCodePage(DWORD dwCharSet) {
	CHARSETINFO cs={0};

	::TranslateCharsetInfo((DWORD *)dwCharSet, &cs, TCI_SRCCHARSET);

	return cs.ciACP;
}

int RenderList::BreakIntoFragments(List2<RenderNode> &list, const DialogueItem *pdi, FontCache *pfc, HDC hdc, bool fAntialias, eWrapMode& nWrapMode) {
	long fontid = -1;
	char fontnamebuf[128];
	const char *fontname = pdi->pDSD->fontname;
	char *fontallocname = NULL;
	int nAscent, nDescent;

	bool fBold				= pdi->pDSD->fBold;
	bool fItalics			= pdi->pDSD->fItalics;
	bool fUnderline			= pdi->pDSD->fUnderline;
	int nFontHeight			= pdi->pDSD->nHeight;
	COLORREF rgbFore		= pdi->pDSD->rgbColors[0];
	COLORREF rgbFore2		= pdi->pDSD->rgbColors[1];
	int	nBorder				= pdi->pDSD->nBorderWidth;
	int nShadow				= pdi->pDSD->nShadowDepth;
	DWORD dwCharSet			= pdi->pDSD->dwCharSet;
	DWORD dwCodePage		= CharSetToCodePage(dwCharSet);
	int nAlignmentOverride	= 0;

	const char *s = pdi->text;
	const char *fragment_start;
	bool fAfterForcedBreak = true;
	int hardline = 0;
	RenderNode *pnode = NULL;
	int karaoketime = -1;
	int karaokestart = 0;
	int karaokedelay = 0;

	// Break into fragments

	while(*s) {
		// Skip whitespace at the beginning and after forced breaks (\N).

		if (fAfterForcedBreak) {
			while(our_isspace(*s))
				++s;

			if (*s && !our_isoverridestart(s) && !our_isnewline(s, nWrapMode))
				fAfterForcedBreak = false;
		}

		// Process tags.

		while(our_isoverridestart(s)) {

			// Process enclosed tags for this set of braces.

			while(*s && *s++ != '}') {
				if (*s == '\\') {
					++s;
					switch(*s++) {

					// Sub Station Alpha allows multiple \a overrides, but only the first one
					// affects the rendering.  The last one is the one used in the preview.
					// *sigh*

					case 'a':
						if (!nAlignmentOverride)
							nAlignmentOverride = strtol(s, (char **)&s, 10);
						break;

					case 'b':
						if (isdigit(*s))
							fBold = (*s++ == '1');
						break;
					case 'i':
						if (isdigit(*s))
							fItalics = (*s++ == '1');
						break;
					case 'f':
						switch(*s++) {
						case 's':
							nFontHeight = strtol(s, (char **)&s, 10);
							break;
						case 'n':
							{
								char *u = fontnamebuf;

								free(fontallocname);

								while(*s && *s != '\\' && *s!='}')
									*u++ = *s++;

								*u = 0;

								fontname = fontallocname = strdup(fontnamebuf);
							}
							break;
						case 'e':
							dwCharSet = strtoul(s, (char **)&s, 10);
							dwCodePage = CharSetToCodePage(dwCharSet);

							break;
						default:
							break;
						}
						break;
					case 'c':
						{
							if (*s == '&')
								++s;

							if (toupper(*s) == 'H')
								++s;

							rgbFore = strtol(s, (char **)&s, 16);

							if (*s == '&')
								++s;
						}
						break;

					case 'q':
						{
							int mode = strtol(s, (char **)&s, 10);

							if (mode<0 || mode>3)
								nWrapMode = kWrapSmartAuto;
							else
								nWrapMode = (eWrapMode)mode;
						}
						break;

					case 'r':
						free(fontallocname); fontallocname = NULL;
						fontname = pdi->pDSD->fontname;

						fBold		= pdi->pDSD->fBold;
						fItalics	= pdi->pDSD->fItalics;
						fUnderline	= pdi->pDSD->fUnderline;
						nFontHeight	= pdi->pDSD->nHeight;
						rgbFore		= pdi->pDSD->rgbColors[0];
						dwCharSet	= pdi->pDSD->dwCharSet;
						dwCodePage = CharSetToCodePage(dwCharSet);
						break;

					case 'k':
						if (karaoketime > 0 || karaokedelay > 0) {
							karaokestart += karaoketime + karaokedelay;
							karaokedelay = 0;
						}
						karaokedelay += strtoul(s, (char **)&s, 10);
						karaoketime = 0;
						break;

					case 'K':
						if (karaoketime > 0 || karaokedelay > 0) {
							karaokestart += karaoketime + karaokedelay;
							karaokedelay = 0;
						}
						karaoketime = strtoul(s, (char **)&s, 10);
						break;
					}

					--s;
				}
			}

			// If we have an unmatched brace, don't generate a null fragment.

			if (!*s)
				goto quit_wordwrapping;
		}

		// Skip whitespace again.

		if (fAfterForcedBreak) {
			while(our_isspace(*s))
				++s;

			if (*s && !our_isoverridestart(s) && !our_isnewline(s, nWrapMode))
				fAfterForcedBreak = false;
		}

		// We've found the start of the fragment text, so mark it.

		fragment_start = s;

		// Scan until another tag set starts, or a newline is hit.

		while(*s && !our_isoverridestart(s) && !our_isnewline(s, nWrapMode)) {
			if (*s == '\\')
				++s;

			s = CharNextExA(dwCodePage, s, 0);
		}

		// Add a fragment.

		if (s >= fragment_start || !our_isnewline(s, nWrapMode)) {
			pnode = AllocNode();

			fontid = pfc->Select(fontid, fontname, fAntialias ? scaleya(nFontHeight) : scaley(nFontHeight), fItalics, fUnderline, fBold, dwCharSet);

			{
				TEXTMETRIC tm;

				GetTextMetrics(hdc, &tm);

				nAscent = tm.tmAscent;
				nDescent = tm.tmDescent;
			}

			pfc->Deselect();

			{
				// Process escapes.

				char szTextBuffer[8192];
				const char *src = fragment_start;
				const char *src2;
				char *dst = szTextBuffer;

				while(src < s) {
					if (*src == '\\') {
						char c;

						++src;

						switch(c=*src++) {
						case 'n':			// convert soft breaks to spaces
							*dst++ = ' ';
							continue;
						case 'h':
							*dst++ = (char)0xA0;
							continue;
						case '!':
							g_pGetHostVerInfo(dst, (szTextBuffer+sizeof szTextBuffer)-dst);
							while(*dst)
								++dst;
							continue;
						default:
							--src;
						}
					}

					src2 = CharNextExA(dwCodePage, src, 0);
					while(src < src2)
						*dst++ = *src++;
				}

				*dst = 0;

				// Convert strings to Unicode.  This is required for Japanese support to
				// work under Windows 95/98/ME.  It's also a hell of a lot easier to work
				// with and is faster under Windows NT/2000.

				int nRequired = MultiByteToWideChar(dwCodePage, 0, szTextBuffer, -1, NULL, 0);

				if (nRequired>0 && (pnode->pszText = pnode->pszTextAlloc = new wchar_t[nRequired])) {
					pnode->nChars = nRequired-1;

					if (!MultiByteToWideChar(dwCodePage, 0, szTextBuffer, -1, pnode->pszTextAlloc, nRequired)) {
						delete[] pnode->pszTextAlloc;
						pnode->pszTextAlloc = NULL;
					}
				}
			}
			pnode->r.top			= hardline;

			if (pnode->nChars) {
				pnode->rgbFore			= rgbFore;
				pnode->rgbFore2			= rgbFore2;
				pnode->rgbBack			= pdi->pDSD->rgbShadowColor;
				pnode->pszFontName		= fontname;
				pnode->pszAllocedName	= fontallocname;
				pnode->nFontHeight		= nFontHeight;
				pnode->fBold			= fBold;
				pnode->fItalics			= fItalics;
				pnode->fUnderline		= fUnderline;
				pnode->fOpaqueBox		= pdi->pDSD->fOpaqueBox;
				pnode->xsplit			= 0;
				pnode->xsplittimelen	= karaoketime;
				pnode->xsplitdelay		= karaokestart*10;
				pnode->nAscent			= nAscent;
				pnode->nDescent			= nDescent;
				pnode->nBorder			= nBorder;
				pnode->nShadow			= nShadow;

				pnode->pKey				= pdi;
				pnode->nFontId			= fontid;
				pnode->dwCharSet		= dwCharSet;
				pnode->dwCodePage		= dwCodePage;
				pnode->nXScrollRate		= pdi->nXScrollRate;
				pnode->nYScrollRate		= pdi->nYScrollRate;
				pnode->nYWindowPos		= pdi->nYWindowPos;
				pnode->nYWindowSize		= pdi->nYWindowSize;

				list.AddTail(pnode);
				_RPT1(0,"Adding: [%s...\n", pnode->pszText);
			} else {
				listFree.AddHead(pnode);
			}

			if (karaoketime >= 0)
				karaokestart += karaoketime + karaokedelay;

			fontallocname = NULL;
			karaoketime = -1;
			karaokedelay = 0;
		}

		// Skip hard newlines.

		while(our_isnewline(s, nWrapMode)) {
			s += 2;
			++hardline;
			fAfterForcedBreak = true;
		}

		if (pnode) {
			pnode->nHardBreaksAfter = hardline;
			hardline = 0;
		}
	}
quit_wordwrapping:

	if (fontallocname)
		free(fontallocname);

	return nAlignmentOverride;
}

void RenderList::Backward(RenderNode *&pnode, int& nChar, int nCount) {
	while(nCount--) {
		if (nChar > 0)
			--nChar;
		else {
			if (!pnode->NextFromTail()->NextFromTail())
				return;

			pnode = pnode->NextFromTail();
			nChar = pnode->nChars;
			++nCount;
		}
	}
}

void RenderList::Forward(RenderNode *&pnode, int& nChar, int nCount) {
	while(nCount--) {
		if (++nChar >= pnode->nChars) {
			if (!pnode->NextFromHead()->NextFromHead()) {
				break;
			}

			pnode = pnode->NextFromHead();
			nChar = 0;
		}
	}
}

bool RenderList::AtEnd(RenderNode *&pnode, int &nChar) {
	return nChar >= pnode->nChars;
}

SIZE RenderList::SizeText(RenderNode *&pnode, int& nChar, RenderNode *pnode_limit, int nCharLimit, FontCache *pfc, HDC hdc, bool fAntialias, int &nMaxDescent, long nMaxWidth) {
	SIZE size;
	SIZE size_max;

	size_max.cx = 0;
	size_max.cy = 0;
	nMaxDescent = 0;

	for(;;) {
		int nAttempted = pnode->nChars - nChar;
		INT nFit;
		bool bDBCSFault = false;

		if (pnode_limit == pnode) {
			nAttempted = nCharLimit - nChar;
			if (!nAttempted)
				break;
		}

		// Anything left?

		if (nAttempted) {
			int nWidthLimit = fAntialias ? scalea(nMaxWidth) : nMaxWidth;

			pnode->nFontId = pfc->Select(pnode->nFontId, pnode->pszFontName, fAntialias ? scaleya(pnode->nFontHeight) : scaley(pnode->nFontHeight), pnode->fItalics, pnode->fUnderline, pnode->fBold, pnode->dwCharSet);

			if (mbAssumeGDISucks) {		// Windows 95/98 implementation
				const wchar_t *text = pnode->pszText + nChar;
				SIZE tmpsize;
				int limit = nWidthLimit;

				size.cx = size.cy = 0;
				nFit = 0;

				while(text[nFit] && nFit < nAttempted) {
					if (!GetTextExtentPoint32W(hdc, text+nFit, 1, &tmpsize))
						return size_max;

					limit -= tmpsize.cx;

					if (limit < 0)
						break;

					size.cx += tmpsize.cx;
					if (tmpsize.cy > size.cy)
						size.cy = tmpsize.cy;

					++nFit;
				}

				_RPT2(0, "fitting [%ls...] [encoding %d]", pnode->pszText + nChar, pnode->dwCharSet);
				_RPT4(0, " size=(%d,%d) chars=%d max=%d\n", size.cx, size.cy, nFit, nAttempted);

			} else {					// Windows NT/2000 implementation
				if (!GetTextExtentExPointW(hdc, pnode->pszText + nChar, nAttempted, nWidthLimit, &nFit, NULL, &size))
					return size_max;
			}

			pfc->Deselect();
		} else {
			size.cx = size.cy = 0;
			nFit = 0;
		}

		pnode->r.left = size_max.cx;
		pnode->r.right = size_max.cx + (fAntialias ? scaleia(size.cx) : size.cx);
		pnode->r.top = 0;
		pnode->r.bottom = fAntialias ? scaleia(pnode->nAscent) + scaleia(pnode->nDescent) : pnode->nAscent + pnode->nDescent;

		if (fAntialias) {
			size_max.cx += scaleia(size.cx);
			nMaxWidth -= scaleia(size.cx);
		} else {
			size_max.cx += size.cx;
			nMaxWidth -= size.cx;
		}

		if (size_max.cy < pnode->nAscent)
			size_max.cy = pnode->nAscent;

		if (nMaxDescent < pnode->nDescent)
			nMaxDescent = pnode->nDescent;

		// Do not word wrap across hard breaks (\N).

		if (nChar+nFit >= pnode->nChars && pnode->nHardBreaksAfter) {
			Forward(pnode, nChar, nFit);
			break;
		}

		Forward(pnode, nChar, nFit);

		if (nFit < nAttempted || !nAttempted)
			break;
	}

	return size_max;
}

void RenderList::SplitAt(RenderNode *&pnode, int &nChar) {
	if (!nChar || nChar >= pnode->nChars)
		return;

	RenderNode *pnode2 = AllocNode();

	if (!pnode2)
		return;

	*pnode2 = *pnode;

	pnode->nChars = nChar;
	pnode->nHardBreaksAfter = 0;
	pnode2->pszText += nChar;
	pnode2->nChars -= nChar;
	pnode2->pszAllocedName = NULL;
	pnode2->pImageCache = NULL;
	pnode2->pszTextAlloc = NULL;
	pnode2->xsplittimelen = -1;
	pnode2->InsertAfter(pnode);

	pnode = pnode2;
	nChar = 0;
}

int RenderList::AttemptWordWrap(List2<RenderNode> &list, const DialogueItem *pdi, FontCache *pfc, HDC hdc, bool fAntialias, int &nMaxWidth, bool fReal) {
	RenderNode *pnode;
	int nChar;

	RenderNode *pnodeLine;
	int nCharLine;

	RenderNode *pnodeEnd;
	int nCharEnd;

	int y = 0;
	int lines = 0;
	int maxwidth_encountered = 0;
	int nMaxDescent;

	pnode = list.AtHead();
	nChar = 0;

	for(;;) {
		bool iNewlineVacuum = 1;

		// If this is the beginning of a line, we can skip whitespace.

		_RPT2(0,"Beginning line: %p [%ls...\n", pnode, pnode->pszText + nChar);

		for(;;) {
			if (AtEnd(pnode, nChar)) {

				// special case: If we have no characters in this fragment, kill it.

				if (!nChar) {
					pnode->nChars = 0;
					pnode->pLineStart = NULL;
					pnode->r.left = pnode->r.right = 0;
					pnode->r.top = y;
					pnode->r.bottom = y+(fAntialias ? scaleia(pnode->nAscent) + scaleia(pnode->nDescent) : pnode->nAscent + pnode->nDescent);
				}

				goto finish;
			}

			if (!our_wisspace(pnode->pszText[nChar])) {

				// Kill off whitespace.

				if (fReal) {
					pnode->pszText += nChar;
					pnode->nChars -= nChar;
					nChar = 0;
				}
				break;
			}

			Forward(pnode, nChar, 1);

			// If we're doing the split for real, kill off whitespace fragments we'll never draw.

			if (fReal) {
				if (!nChar) {
					RenderNode *pnodeNull = pnode->NextFromTail();

					pnodeNull->nChars = 0;
					pnodeNull->pLineStart = NULL;
					pnodeNull->r.left = 0;
					pnodeNull->r.right = 0;
					pnodeNull->r.top = y;
					pnodeNull->r.bottom = y+(fAntialias ? scaleia(pnodeNull->nAscent) + scaleia(pnodeNull->nDescent) : pnodeNull->nAscent + pnodeNull->nDescent);

					// At the beginning of a line, swallow the first newline since we don't
					// want to have vertical space if we hit a forced newline after wrapped
					// whitespace.

					if (pnodeNull->nHardBreaksAfter) {
						lines += pnodeNull->nHardBreaksAfter - iNewlineVacuum;
						y += (pnodeNull->r.bottom - pnodeNull->r.top)/2 * (pnodeNull->nHardBreaksAfter - iNewlineVacuum);

						iNewlineVacuum = 0;
					}
				} else if (nChar == pnode->nChars) {
					pnode->nChars = 0;
					pnode->pLineStart = NULL;
					pnode->r.left = pnode->r.right = 0;
					pnode->r.top = y;
					pnode->r.bottom = y+(fAntialias ? scaleia(pnode->nAscent) + scaleia(pnode->nDescent) : pnode->nAscent + pnode->nDescent);
				}
			}
		}

		// Mark start of line.

		pnodeLine = pnode;
		nCharLine = nChar;

		// Attempt to fit text in this line.

		SizeText(pnode, nChar, NULL, 0, pfc, hdc, fAntialias, nMaxDescent, nMaxWidth);

		// Back off while we are on a non-space and there is a non-space before us.

		pnodeEnd = pnode;
		nCharEnd = nChar;

		if (!AtEnd(pnodeEnd, nCharEnd) && !our_wisspace(pnodeEnd->pszText[nCharEnd]))
			for(;;) {
				// Are we at the beginning?

				if (pnodeEnd == pnodeLine && nCharEnd == nCharLine) {
					// Nothing fits, so cram in one word.

					// Advance until we are at the end, hit a space, or cross a hard
					// break.

// fixme: doesn't set sizes!!

					while(!AtEnd(pnodeEnd, nCharEnd) && !our_wisspace((unsigned char)pnodeEnd->pszText[nCharEnd])) {
						int nHardBr = pnodeEnd->nHardBreaksAfter;

						Forward(pnodeEnd, nCharEnd, 1);

						if (!nCharEnd && nHardBr)
							break;

					}

					pnode = pnodeEnd;
					nChar = nCharEnd;

					break;
				}

				Backward(pnodeEnd, nCharEnd, 1);

				// Don't back up across hard breaks!

				if (nCharEnd == pnodeEnd->nChars-1 && pnodeEnd->nHardBreaksAfter) {
					// Oops...

					Forward(pnodeEnd, nCharEnd, 1);
					pnode = pnodeEnd;
					nChar = nCharEnd;
					break;
				}

				if (our_wisspace(pnodeEnd->pszText[nCharEnd])) {
					pnode = pnodeEnd;
					nChar = nCharEnd;
					break;
				}
			}

		// Back off while there is a space before us.

		while(pnode != pnodeLine || nChar != nCharLine) {
			Backward(pnode, nChar, 1);

			if (!our_wisspace(pnode->pszText[nChar])) {
				Forward(pnode, nChar, 1);
				break;
			}
		}

		// Split the node.

		if (fReal)
			SplitAt(pnode, nChar);

		// Compute sizes for fragments and the line.

		_RPT2(0,"Preclipped line: [%.*ls]\n", (pnode->pszText + nChar) - (pnodeLine->pszText + nCharLine), pnodeLine->pszText + nChar);

		RenderNode *linestart = pnodeLine;
		int linestartchar = nChar;

		nMaxDescent = 0;
		SIZE s = SizeText(pnodeLine, nCharLine, pnode, nChar, pfc, hdc, fAntialias, nMaxDescent, 0x00FFFFFF);

		_RPT2(0,"Clipped line: [%.*ls]\n", (pnodeLine->pszText + nCharLine) - (linestart->pszText + linestartchar), linestart->pszText + linestartchar);

		if (s.cx > maxwidth_encountered)
			maxwidth_encountered = s.cx;

		if (fReal) {
			pnodeLine = linestart;
			nCharLine = linestartchar;

			while(pnodeLine != pnode || (pnodeLine == pnode && nChar)) {
				_RPT1(0,"Adjusting: %p\n", pnodeLine);

				pnodeLine->pLineStart = linestart;

				pnodeLine->r.top += y;
				pnodeLine->r.bottom += y;

				if (pnodeLine == pnode)
					break;

				pnodeLine = pnodeLine->NextFromHead();
			}

			linestart->pLineStart = NULL;
		}

		// Advance.

		if (fAntialias)
			y += scaleia(s.cy) + scaleia(nMaxDescent);
		else
			y += s.cy + nMaxDescent;
		++lines;

		// Handle forced breaks.

		Backward(pnode, nChar, 1);

		if (pnode->nHardBreaksAfter>1) {
			lines += (pnode->nHardBreaksAfter-1);
			y += (pnode->r.bottom - pnode->r.top)/2 * (pnode->nHardBreaksAfter-1);
		}

		Forward(pnode, nChar, 1);
	}
finish:

	if (fReal) {
		_RPT0(0,"--------start wrap results------\n");
		for(pnode = list.AtHead(); pnode->NextFromHead(); pnode = pnode->NextFromHead()) {
			_RPT4(0,"%p (%p): [%-*ls...\n", pnode, pnode->pLineStart, pnode->nChars, pnode->pszText);
		}
		_RPT1(0,"---------end wrap results------- %d\n", y);
	}

	nMaxWidth = maxwidth_encountered;

	return fReal ? y : lines;
}

void RenderList::AddItem(const DialogueItem *pdi, FontCache *pfc, /*VFBitmap *,*/int videoWidth, int videoHeight, HDC hdc, bool fAntialias) {
	RenderNode *pnode, *pnode_next, *pnodeLine;
	List2<RenderNode> list;
	int total_height = 0;
	int nMaxAscent=0, nMaxDescent = 0;
	int total_width = 0;
	long x, y, w;
	RECT rBounds;
	int daPos = pdi->pDSD->daPos;
	int nAlignmentOverride;
	eWrapMode nWrapMode = mDefaultWrapMode;

	// Break into fragments.
	
	nAlignmentOverride = BreakIntoFragments(list, pdi, pfc, hdc, fAntialias, nWrapMode);

	// If the list is empty...

	if (list.IsEmpty())
		return;

	// Process an alignment override if there is one.

	if (nAlignmentOverride) {
		daPos = nAlignmentOverride & 3;

		if (nAlignmentOverride & 4) daPos |= DALIGN_TOP;
		else if (nAlignmentOverride & 8) daPos |= DALIGN_MIDDLE;
		else daPos |= DALIGN_BOTTOM;
	}

	// Word wrapping time!

	rBounds.top		= scaley(pdi->nBottomMargin);
	rBounds.left	= scalex(pdi->nLeftMargin);
	//rBounds.right	= vbm->w - scalex(pdi->nRightMargin);
	//rBounds.bottom	= vbm->h - scaley(pdi->nBottomMargin);
	rBounds.right	= videoWidth - scalex(pdi->nRightMargin);
	rBounds.bottom	= videoHeight - scaley(pdi->nBottomMargin);

	int wrap_lines, wrap_width, wrap_lines_new, wrap_width_new;

	// Do initial wordwrap.

	if (pdi->nXScrollRate || nWrapMode == kWrapManual)
		wrap_width = 0x1000000;
	else
		wrap_width = rBounds.right - rBounds.left;

	wrap_lines = AttemptWordWrap(list, pdi, pfc, hdc, fAntialias, wrap_width, false);

	// Smart wrapping time.

	if (nWrapMode == kWrapSmartAuto) {
		if (wrap_lines > 1)
			for(;;) {
				wrap_width_new = wrap_width - 1;
				wrap_lines_new = AttemptWordWrap(list, pdi, pfc, hdc, fAntialias, wrap_width_new, false);

				if (wrap_lines_new != wrap_lines || wrap_width_new >= wrap_width)
					break;

				wrap_width = wrap_width_new;
			}
	}

	// Wrap for real.

	total_height = AttemptWordWrap(list, pdi, pfc, hdc, fAntialias, wrap_width, true);
	total_width = wrap_width;

	// Adjust karaoke.

	for(pnodeLine = list.AtHead(); pnode_next = pnodeLine->NextFromHead(); pnodeLine = pnode_next) {
		int karaoketime = pnodeLine->xsplittimelen;

		if (karaoketime>=0) {
			RenderNode *next;
			int pixelength = 0;
			int accum = 0;
			int totaltime = pdi->start_ms + pnodeLine->xsplitdelay;

			for(pnode = pnodeLine; next = pnode->NextFromHead(); pnode = next) {
				if (pnode != pnodeLine && pnode->xsplittimelen>=0)
					break;

				pixelength += pnode->r.right - pnode->r.left;
			}

			if (pixelength) {
				for(pnode = pnodeLine; next = pnode->NextFromHead(); pnode = next) {
					if (pnode != pnodeLine && pnode->xsplittimelen>=0)
						break;

					pnode->xsplitstart = totaltime + (accum*karaoketime*10)/pixelength;
					accum += pnode->r.right - pnode->r.left;
					pnode->xsplittimelen = totaltime + (accum*karaoketime*10)/pixelength - pnode->xsplitstart;
				}
			} else {
				pnodeLine->xsplitstart = totaltime;
				pnodeLine->xsplittimelen = 1;
			}

			pnode_next = pnode;
		}
	}

	// Determine placement.

	bool fMoveDown = true;

	// cheeseball: force scrollers to come from the border

	if (pdi->nYScrollRate>0)
		y = pdi->nYWindowPos-total_height;
	else if (pdi->nYScrollRate<0)
		y = pdi->nYWindowPos+pdi->nYWindowSize;
	else {
		switch(daPos & 48) {
		case DALIGN_TOP:
			y = rBounds.top;
			break;
		case DALIGN_BOTTOM:
			y = rBounds.bottom - total_height;
			fMoveDown = false;
			break;
		case DALIGN_MIDDLE:
			y = (rBounds.bottom + rBounds.top - total_height)/2;
			break;
		}
	}

	// For center and right justification, we need to offset each line.
	//
	// While we're doing this, also compute line bounding boxes.

	int nMaxBorder = 0;

	for(pnode = pnodeLine = list.AtHead(); pnode_next = pnode->NextFromHead(); pnode = pnode_next) {

		if (!pnode_next->NextFromHead() || !pnode_next->pLineStart) {
			RenderNode *pnodeLine2 = pnodeLine;
			int w = pnode->r.right;
			int h = 0;
			int xo = 0;
			int nMaxAscent = 0;
			int nMaxDescent = 0;
			int y2 = pnodeLine->r.top;

			// Compute line bounding box.

			do {
				if (pnodeLine2->nAscent > nMaxAscent)
					nMaxAscent = pnodeLine2->nAscent;

				if (pnodeLine2->nDescent > nMaxDescent)
					nMaxDescent = pnodeLine2->nDescent;

				if (pnodeLine2->nBorder > nMaxBorder)
					nMaxBorder = pnodeLine2->nBorder;

				pnodeLine2 = pnodeLine2->NextFromHead();
			} while(pnodeLine2 != pnode_next);

			// Write line bounding box to line start.

			if (fAntialias)
				h = scaleia(nMaxAscent) + scaleia(nMaxDescent);
			else
				h = nMaxAscent + nMaxDescent;

			pnodeLine->nLineWidth = w;
			pnodeLine->nLineHeight = h;

			_RPT3(0, "Finished line: [%d, %d] [%ls...\n", w, h, pnodeLine->pszText);

			// For right and center, recenter each line.

			if (!pdi->nXScrollRate) {
				if ((daPos&7) == DALIGN_CENTER)
					xo = (total_width - w)/2;
				else if ((daPos&7) == DALIGN_RIGHT)
					xo = total_width - w;
			}

			do {
				_RPT3(0, "\tOffsetting: (%d, %d) [%ls...\n", pnodeLine->r.left, pnodeLine->r.right, pnodeLine->pszText);

				pnodeLine->r.left += xo;
				pnodeLine->r.right += xo;
				pnodeLine->r.top = y2;
				pnodeLine->r.bottom = y2 + h;
				pnodeLine->nBaseline = nMaxAscent;

				pnodeLine = pnodeLine->NextFromHead();
			} while(pnodeLine != pnode_next);
		}
	}

	// Compute box enclosing all text.

	RECT rLargest;

	rLargest.top	= y;
	rLargest.bottom	= y+total_height;

	if (pdi->nXScrollRate)
		rLargest.left = videoWidth;//vbm->w;
	else
		switch(daPos & 7) {
		case DALIGN_LEFT:	rLargest.left = rBounds.left; break;
		case DALIGN_CENTER:	rLargest.left = (rBounds.left + rBounds.right - total_width)/2; break;
		case DALIGN_RIGHT:	rLargest.left = rBounds.right - total_width; break;
		}

	rLargest.right = rLargest.left + total_width;

	rLargest.left -= nMaxBorder;
	rLargest.top -= nMaxBorder;
	rLargest.right += nMaxBorder;
	rLargest.bottom += nMaxBorder;

	// Check against rectangle list for a collision and try to move
	// the text out of the way vertically.

	bool fCollision = false;

	// Check for collision, but only for non-scrolling text.

	if (!(pdi->nXScrollRate | pdi->nYScrollRate)) {
		do {
			fCollision = false;

			_RPT4(0,"Checking for collision (%d,%d) [%dx%d]\n", rLargest.left, rLargest.top, rLargest.right-rLargest.left, rLargest.bottom-rLargest.top);

			for(pnode = listActive.AtHead(); pnode_next = pnode->NextFromHead(); pnode = pnode_next) {
				RECT rt = pnode->r;

				if (!pnode->nChars || pnode->nXScrollRate || pnode->nYScrollRate)
					continue;

				rt.top -= pnode->nBorder;
				rt.bottom += pnode->nBorder;
				rt.left -= pnode->nBorder;
				rt.right += pnode->nBorder;

				_RPT4(0,"\tTesting against (%d,%d) [%dx%d]\n", rt.left, rt.top, rt.right-rt.left, rt.bottom-rt.top);

				RECT rTemp;

				if (IntersectRect(&rTemp, &rLargest, &rt)) {
					int dy = rTemp.bottom - rTemp.top;

					_RPT1(0,"Collision: moving by %d\n", dy);

					if (fMoveDown) {
						y += dy;
						rLargest.top += dy;
						rLargest.bottom += dy;
					} else {
						y -= dy;
						rLargest.top -= dy;
						rLargest.bottom -= dy;
					}

					fCollision = true;
					break;
				}

			}
		} while(fCollision);

		if (y<0)
			y = 0;
	}

	// Add fragments to list.

	x = total_width;

	while(pnode = list.RemoveTail()) {
		pnode->nStartTime		= pdi->start_ms;
		pnode->r.left += rLargest.left + nMaxBorder;
		pnode->r.right += rLargest.left + nMaxBorder;
		pnode->r.top += y;
		pnode->r.bottom += y;

		w = (pnode->r.right - pnode->r.left);

		x -= w;

		listActive.AddHead(pnode);
	}
}

////////////////////////////////////

void RenderList::RenderNormal(long ms, VFBitmap *vbm, FontCache *pFontCache) {
	RenderNode *pnode = listActive.AtHead(), *pnode_next;

	while(pnode_next = pnode->NextFromHead()) {
		const int x = pnode->r.left;
		const int y = pnode->r.top + pnode->nBaseline - pnode->nAscent;
		const wchar_t *s = pnode->pszText;
		const int len = pnode->nChars;

		pnode->nFontId = pFontCache->Select(pnode->nFontId, pnode->pszFontName, scaley(pnode->nFontHeight), pnode->fItalics, pnode->fUnderline, pnode->fBold, pnode->dwCharSet);
		SetTextColor(vbm->hdc, pnode->rgbBack);

		TextOutW(vbm->hdc, x-1, y-1, s, len);
		TextOutW(vbm->hdc, x  , y-1, s, len);
		TextOutW(vbm->hdc, x+1, y-1, s, len);
		TextOutW(vbm->hdc, x-1, y  , s, len);
		TextOutW(vbm->hdc, x+1, y  , s, len);
		TextOutW(vbm->hdc, x-1, y+1, s, len);
		TextOutW(vbm->hdc, x  , y+1, s, len);
		TextOutW(vbm->hdc, x+1, y+1, s, len);

		SetTextColor(vbm->hdc, pnode->rgbFore);

		TextOutW(vbm->hdc, x, y, s, len);

		pFontCache->Deselect();

		pnode = pnode_next;
	}
}

Pixel *Address32i(unsigned long* data, int pitch, PixCoord x, PixCoord y) 
{
	return (unsigned long *)((char *)data + y*pitch + x*sizeof(Pixel));
}

Pixel *Address32(unsigned long* data, int h, int pitch, PixCoord x, PixCoord y)
{
	return Address32i(data, pitch, x, h-y-1);
}

void RenderList::RenderAntialiased(long ms, /*VFBitmap *vbm,*/int videoWidth, int videoHeight, int videoPitch, unsigned long* videoBuffer, FontCache *pFontCache, HDC hdcAntialias) {
	RenderNode *pnode, *pnode_next;

	pnode = listActive.AtHead();
	while(pnode_next = pnode->NextFromHead()) {
		if (pnode->xsplittimelen >= 0) {
			if (ms < pnode->xsplitstart)
				pnode->xsplit = 0;
			else if (ms >= pnode->xsplitstart + pnode->xsplittimelen)
				pnode->xsplit = -1;
			else
				pnode->xsplit = MulDiv(ms - pnode->xsplitstart, pnode->r.right-pnode->r.left, pnode->xsplittimelen);
		}

		pnode = pnode_next;
	}

	// Make antialiased outlines; render shadows first.
	
	pnode = listActive.AtHead();
	while(pnode_next = pnode->NextFromHead()) {
		if (!pnode->pLineStart && pnode->nLineWidth) {
			RenderNode *const pnodeLine = pnode;
			int w = pnode->nLineWidth;
			int h = pnode->nLineHeight;
			int border = pnode->nBorder;

			if (!pnode->pImageCache) {
				bool bFirstPath = true;

				// Iterate and render fragments on line

				Rasterizer *pRast = pnodeLine->pImageCache = new Rasterizer();

				if (!mbAssumeGDISucks)
					pRast->BeginPath(hdcAntialias);

				do {
					int x = pnode->r.left - pnodeLine->r.left;
					const wchar_t *s = pnode->pszText;
					const int len = pnode->nChars;
					int suboffset = pnode->nBaseline - pnode->nAscent;

					pnode->nFontId = pFontCache->Select(pnode->nFontId, pnode->pszFontName, scaleya(pnode->nFontHeight), pnode->fItalics, pnode->fUnderline, pnode->fBold, pnode->dwCharSet);

//					_RPT3(0,"Rendering: [%d, %d] [%s...\n", x*8, r.top+suboffset, s);

					if (mbAssumeGDISucks) {
						int xpos = scalea(x);

						for(int i=0; i<len; ++i) {
							SIZE size;

							::GetTextExtentPoint32W(hdcAntialias, s+i, 1, &size);

							pRast->PartialBeginPath(hdcAntialias, bFirstPath);
							bFirstPath = false;

							::TextOutW(hdcAntialias, 0, 0, s+i, 1);

							pRast->PartialEndPath(hdcAntialias, xpos, suboffset);
							xpos += size.cx;
						}
					} else
						TextOutW(hdcAntialias, scalea(x), suboffset, s, len);

					pFontCache->Deselect();

					pnode = pnode->NextFromHead();
				} while(pnode->NextFromHead() && pnode->pLineStart);

				if (!mbAssumeGDISucks)
					pRast->EndPath(hdcAntialias);

				pRast->ScanConvert();

				if (border && !pnodeLine->fOpaqueBox)
					pRast->CreateWidenedRegion(8*border);

				if (!(pnodeLine->nXScrollRate | pnodeLine->nYScrollRate)) {
					pRast->Rasterize(0, 0, border!=0);
					pRast->DeleteOutlines();
				}

				pnode_next = pnode;
			}

			if (pnodeLine->nXScrollRate | pnodeLine->nYScrollRate) {
				pnodeLine->pImageCache->Rasterize(
						(long)((__int64)pnodeLine->nXScrollRate * (ms-pnodeLine->nStartTime)/64000),
						(long)((__int64)pnodeLine->nYScrollRate * (ms-pnodeLine->nStartTime)/64000),
						border != 0);
			}

			if (pnodeLine->fOpaqueBox) {
				int x1 = pnodeLine->r.left - pnodeLine->nBorder;
				int y1 = pnodeLine->r.top - pnodeLine->nBorder;
				int x2 = pnodeLine->r.right + pnodeLine->nBorder;
				int y2 = pnodeLine->r.bottom + pnodeLine->nBorder;

				if (pnodeLine->nXScrollRate) {
					x1 = 0;
					x2 = videoWidth;//vbm->w;
				}

				if (pnodeLine->nYScrollRate) {
					y1 = pnodeLine->nYWindowPos;
					y2 = y1 + pnodeLine->nYWindowSize;
				}

				if (x1 < 0)
					x1 = 0;
				if (y1 < 0)
					y1 = 0;
// 				if (x2 > vbm->w)
// 					x2 = vbm->w;
// 				if (y2 > vbm->h)
// 					y2 = vbm->h;

				if (x2 > videoWidth)
					x2 = videoWidth;
				if (y2 > videoHeight)
					y2 = videoHeight;

				if (x1 < x2 && y1 < y2) {
					const int w = x2 - x1;
					int h = y2 - y1;
					const Pixel32 c = revcolor(pnodeLine->rgbBack);
					//Pixel32 *dst = (Pixel32 *)((char *)vbm->data + vbm->pitch * (vbm->h - y2)) + x2;
					Pixel32 *dst = (Pixel32 *)((char *)videoBuffer + videoPitch * (videoHeight - y2)) + x2;

					do {
						int w2 = -w;

						do {
							dst[w2] = c;
						} while(++w2);

						//dst = (Pixel32 *)((char *)dst + vbm->pitch);
						dst = (Pixel32 *)((char *)dst + videoPitch);
					} while(--h);
				}
			} else if (pnodeLine->nShadow) {
				if (pnodeLine->nYScrollRate) {
					//pnodeLine->pImageCache->DrawShadow(vbm->Address32(0, pnodeLine->nYWindowPos + pnodeLine->nYWindowSize),
					pnodeLine->pImageCache->DrawShadow(Address32(videoBuffer, videoHeight, videoPitch, 0, pnodeLine->nYWindowPos + pnodeLine->nYWindowSize),
							(pnodeLine->r.left+pnodeLine->nShadow)*8 + (long)((__int64)pnodeLine->nXScrollRate * (ms-pnodeLine->nStartTime) / 64000),
							(pnodeLine->r.top+pnodeLine->nShadow)*8 + (long)((__int64)pnodeLine->nYScrollRate * (ms-pnodeLine->nStartTime) / 64000),
							//vbm->w, pnodeLine->nYWindowSize, vbm->pitch, revcolor(pnodeLine->rgbBack));
							videoWidth, pnodeLine->nYWindowSize, videoPitch, revcolor(pnodeLine->rgbBack));
					
				} else {
					pnodeLine->pImageCache->DrawShadow(
							//vbm->data,
							videoBuffer,
							(pnodeLine->r.left+pnodeLine->nShadow)*8 + (long)((__int64)pnodeLine->nXScrollRate * (ms-pnodeLine->nStartTime) / 64000),
							(pnodeLine->r.top+pnodeLine->nShadow)*8 + (long)((__int64)pnodeLine->nYScrollRate * (ms-pnodeLine->nStartTime) / 64000),
							//vbm->w, vbm->h, vbm->pitch, 
							videoWidth, videoHeight, videoPitch, 
							revcolor(pnodeLine->rgbBack));
				}
			}
		}

		pnode = pnode_next;
	}

	// Render text.

	{
		long switchbuffer[512];

		pnode = listActive.AtHead();
		while(pnode_next = pnode->NextFromHead()) {
			if (!pnode->pLineStart && pnode->nLineWidth) {
				RenderNode *pnodeLine = pnode;
				int border = pnode->fOpaqueBox ? 0 : pnode->nBorder;
				int w = pnode->nLineWidth;
				int h = pnode->nLineHeight;
				long *pswitch = switchbuffer;
				int cx = border;

				do {
					if (pnode->xsplittimelen >= 0) {
						if (pnode->xsplit < 0) {
							*pswitch++ = revcolor(pnode->rgbFore);
							*pswitch++ = pnode->r.left - pnodeLine->r.left + border;
						} else {
							*pswitch++ = revcolor(pnode->rgbFore);
							*pswitch++ = pnode->r.left - pnodeLine->r.left + border;
							*pswitch++ = revcolor(pnode->rgbFore2);
							*pswitch++ = pnode->r.left - pnodeLine->r.left + border + pnode->xsplit;
						}
					} else {
						*pswitch++ = revcolor(pnode->rgbFore);
						*pswitch++ = pnode->r.left - pnodeLine->r.left + border;
					}
				} while((pnode_next = pnode->NextFromHead()) && (pnode = pnode_next)->pLineStart);

				pswitch[0] = pswitch[-2];
				pswitch[1] = 0x00FFFFFF;

				_RPT3(0,"Rendering(%d,%d): %ls\n", pnodeLine->r.left-border, pnodeLine->r.top-border, pnodeLine->pszText);

				if (pnodeLine->nYScrollRate)
//					pnodeLine->pImageCache->Draw(vbm->Address32(0, pnodeLine->nYWindowPos + pnodeLine->nYWindowSize),
					pnodeLine->pImageCache->Draw(
						Address32(videoBuffer, videoHeight, videoPitch, 0, pnodeLine->nYWindowPos + pnodeLine->nYWindowSize),
						pnodeLine->r.left*8 + (long)(((__int64)pnodeLine->nXScrollRate * (ms-pnodeLine->nStartTime)) / 64000),
						pnodeLine->r.top*8 + (long)(((__int64)pnodeLine->nYScrollRate * (ms-pnodeLine->nStartTime)) / 64000),
						//vbm->w, pnodeLine->nYWindowSize, vbm->pitch, 
						videoWidth, pnodeLine->nYWindowSize, videoPitch, 
						revcolor(pnodeLine->rgbBack), border!=0 && !pnodeLine->fOpaqueBox, switchbuffer);
				else
					pnodeLine->pImageCache->Draw(
						//vbm->data,
						videoBuffer,
						pnodeLine->r.left*8 + (long)(((__int64)pnodeLine->nXScrollRate * (ms-pnodeLine->nStartTime)) / 64000),
						pnodeLine->r.top*8,
						//vbm->w, vbm->h, vbm->pitch, 
						videoWidth, videoHeight, videoPitch, 
						revcolor(pnodeLine->rgbBack), border!=0 && !pnodeLine->fOpaqueBox, switchbuffer);
			}

//			vbm->RectFill(pnode->r.left, pnode->r.top, pnode->r.right - pnode->r.left, pnode->r.bottom - pnode->r.top, 0xFF0000);

			pnode = pnode_next;
		}
	}
}
