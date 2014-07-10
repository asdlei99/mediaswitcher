/*	Subtitler - Sub Station Alpha v2.x/4.x subtitling plugin for VirtualDub
	Copyright (C) 2000-2003 Avery Lee, All Rights Reserved.

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

	Rasterizer.cpp: Converts font outlines to overlay graphics

    WARNING: /QIfist must be specified on the command line!
*/

// shut up you stupid compiler
#pragma warning(disable: 4799)		// warning C4799: function 'x' has no EMMS instruction

#include <crtdbg.h>

#include <string.h>
#include <math.h>

#include <vector>
#include <algorithm>

#include <windows.h>

#include "Rasterizer.h"

Rasterizer::Rasterizer()
: mpPathTypes(NULL)
, mpPathPoints(NULL)
, mpOverlayBuffer(NULL)
{
}

Rasterizer::~Rasterizer() {
	_TrashPath();
	_TrashOverlay();
}

void Rasterizer::_TrashPath() {
	delete[] mpPathTypes;
	delete[] mpPathPoints;
	mpPathTypes = NULL;
	mpPathPoints = NULL;
	mPathPoints = 0;
}

void Rasterizer::_TrashOverlay() {
	delete[] mpOverlayBuffer;
	mpOverlayBuffer = NULL;
}

void Rasterizer::_ReallocEdgeBuffer(int edges) {
	Edge *newheap = (Edge *)realloc(mpEdgeBuffer, sizeof(Edge)*edges);

	mEdgeHeapSize = edges;
	mpEdgeBuffer = newheap;
}

void Rasterizer::_EvaluateBezier(int ptbase) {
	const POINT *pt0 = mpPathPoints + ptbase;
	const POINT *pt1 = mpPathPoints + ptbase + 1;
	const POINT *pt2 = mpPathPoints + ptbase + 2;
	const POINT *pt3 = mpPathPoints + ptbase + 3;

	double x0 = pt0->x;
	double x1 = pt1->x;
	double x2 = pt2->x;
	double x3 = pt3->x;
	double y0 = pt0->y;
	double y1 = pt1->y;
	double y2 = pt2->y;
	double y3 = pt3->y;

	double cx3 = -  x0+3*x1-3*x2+x3;
	double cx2 =  3*x0-6*x1+3*x2;
	double cx1 = -3*x0+3*x1;
	double cx0 =    x0;

	double cy3 = -  y0+3*y1-3*y2+y3;
	double cy2 =  3*y0-6*y1+3*y2;
	double cy1 = -3*y0+3*y1;
	double cy0 =    y0;

	// This equation is from Graphics Gems I.
	//
	// The idea is that since we're approximating a cubic curve with lines,
	// any error we incur is due to the curvature of the line, which we can
	// estimate by calculating the maximum acceleration of the curve.  For
	// a cubic, the acceleration (second derivative) is a line, meaning that
	// the absolute maximum acceleration must occur at either the beginning
	// (|c2|) or the end (|c2+c3|).  Our bounds here are a little more
	// conservative than that, but that's okay.
	//
	// If the acceleration of the parametric formula is zero (c2 = c3 = 0),
	// that component of the curve is linear and does not incur any error.
	// If a=0 for both X and Y, the curve is a line segment and we can
	// use a step size of 1.

	double maxaccel1 = fabs(2*cy2) + fabs(6*cy3);
	double maxaccel2 = fabs(2*cx2) + fabs(6*cx3);

	double maxaccel = maxaccel1 > maxaccel2 ? maxaccel1 : maxaccel2;
	double h = 1.0;

	if (maxaccel > 8.0)
		h = sqrt(8.0 / maxaccel);

	int lastx = x0;
	int lasty = y0;

	// round, not truncate
	cx0 += 0.5;
	cy0 += 0.5;

	for(double t=h; t<1.0; t += h) {
		double x = cx0 + t*(cx1 + t*(cx2 + t*cx3));
		double y = cy0 + t*(cy1 + t*(cy2 + t*cy3));

		// WARNING: This code assumes that the /QIfist compiler flag is
		//			set and that the rounding mode is round down!
		int xi = (int)x;
		int yi = (int)y;

		_EvaluateLine(lastx, lasty, x, y);
		lastx = x;
		lasty = y;
	}

	_EvaluateLine(lastx, lasty, pt3->x, pt3->y);
}

void Rasterizer::_EvaluateLine(int pt1idx, int pt2idx) {
	const POINT *pt1 = mpPathPoints + pt1idx;
	const POINT *pt2 = mpPathPoints + pt2idx;

	_EvaluateLine(pt1->x, pt1->y, pt2->x, pt2->y);
}

void Rasterizer::_EvaluateLine(int x0, int y0, int x1, int y1) {
	int prestep;

	if (y1 > y0) {	// down
		int dy = y1-y0;
		int xacc = x0<<13;

		// prestep y0 down

		int y = ((y0+3)&~7) + 4;

		y1 = (y1-5)>>3;

		int iy = y>>3;

		if (iy <= y1) {
			int invslope = (x1-x0)*65536/dy;

			while(mEdgeNext + y1 + 1 - iy > mEdgeHeapSize)
				_ReallocEdgeBuffer(mEdgeHeapSize*2);

			prestep = y - y0;
			xacc += (invslope * prestep)>>3;

			while(iy <= y1) {
				int ix = (xacc+32768)>>16;

				mpEdgeBuffer[mEdgeNext].next = mpScanBuffer[iy]&0xfffff;
				mpEdgeBuffer[mEdgeNext].posandflag = ix*2+1;

				mpScanBuffer[iy] = (mpScanBuffer[iy]&0xfff00000) + 0x00100000 + mEdgeNext++;

				++iy;
				xacc += invslope;
			}
		}
	} else if (y1 < y0) {		// up
		int dy = y0-y1;
		int xacc = x1<<13;

		// prestep y1 down

		int y = ((y1+3)&~7) + 4;

		y0 = (y0-5)>>3;

		int iy = y>>3;

		if (iy <= y0) {
			int invslope = (x0-x1)*65536/dy;

			while(mEdgeNext + y0 + 1 - iy > mEdgeHeapSize)
				_ReallocEdgeBuffer(mEdgeHeapSize*2);

			prestep = y - y1;
			xacc += (invslope * prestep)>>3;

			while(iy <= y0) {
				int ix = (xacc+32768)>>16;

				mpEdgeBuffer[mEdgeNext].next = mpScanBuffer[iy]&0xfffff;
				mpEdgeBuffer[mEdgeNext].posandflag = ix*2;

				mpScanBuffer[iy] = (mpScanBuffer[iy]&0xfff00000) + 0x00100000 + mEdgeNext++;

				++iy;
				xacc += invslope;
			}
		}
	}
}

bool Rasterizer::BeginPath(HDC hdc) {
	_TrashPath();

	return !!::BeginPath(hdc);
}

void Rasterizer::RenderPath(HDC hdc, int x, int y, const char *pszText) {
	TextOut(hdc, x, y, pszText, strlen(pszText));
}

bool Rasterizer::EndPath(HDC hdc) {
	::CloseFigure(hdc);

	if (::EndPath(hdc)) {
		mPathPoints = GetPath(hdc, NULL, NULL, 0);

		if (!mPathPoints)
			return true;

		mpPathTypes = (BYTE *)malloc(sizeof(BYTE) * mPathPoints);
		mpPathPoints = (POINT *)malloc(sizeof(POINT) * mPathPoints);

		if (mPathPoints == GetPath(hdc, mpPathPoints, mpPathTypes, mPathPoints))
			return true;
	}

	::AbortPath(hdc);

	return false;
}

bool Rasterizer::PartialBeginPath(HDC hdc, bool bClearPath) {
	if (bClearPath)
		_TrashPath();

	return !!::BeginPath(hdc);
}

bool Rasterizer::PartialEndPath(HDC hdc, long dx, long dy) {
	::CloseFigure(hdc);

	if (::EndPath(hdc)) {
		int nPoints;
		BYTE *pNewTypes;
		POINT *pNewPoints;

		nPoints = GetPath(hdc, NULL, NULL, 0);

		if (!nPoints)
			return true;

		pNewTypes = (BYTE *)realloc(mpPathTypes, (mPathPoints + nPoints) * sizeof(BYTE));
		pNewPoints = (POINT *)realloc(mpPathPoints, (mPathPoints + nPoints) * sizeof(POINT));

		if (pNewTypes)
			mpPathTypes = pNewTypes;

		if (pNewPoints)
			mpPathPoints = pNewPoints;

		BYTE *pTypes = new BYTE[nPoints];
		POINT *pPoints = new POINT[nPoints];

		if (pNewTypes && pNewPoints && nPoints == GetPath(hdc, pPoints, pTypes, nPoints)) {
			for(int i=0; i<nPoints; ++i) {
				mpPathPoints[mPathPoints + i].x = pPoints[i].x + dx;
				mpPathPoints[mPathPoints + i].y = pPoints[i].y + dy;
				mpPathTypes[mPathPoints + i] = pTypes[i];
			}

			mPathPoints += nPoints;

			delete[] pTypes;
			delete[] pPoints;
			return true;
		} else
			DebugBreak();

		delete[] pTypes;
		delete[] pPoints;
	}

	::AbortPath(hdc);

	return false;
}

bool Rasterizer::ScanConvert() {
	int lastmoveto = -1;
	int i;

	// Drop any outlines we may have.

	mOutline.clear();
	mWideOutline.clear();

	// Determine bounding box

	if (!mPathPoints) {
		mPathOffsetX = mPathOffsetY = 0;
		mWidth = mHeight = 0;
		return 0;
	}

	int minx =  0x7F000000;
	int miny =  0x7F000000;
	int maxx = -0x7F000000;
	int maxy = -0x7F000000;

	for(i=0; i<mPathPoints; ++i) {
		double x = mpPathPoints[i].x;
		double y = mpPathPoints[i].y;

		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
	}

	minx>>=3;
	miny>>=3;
	maxx = (maxx+7)>>3;
	maxy = (maxy+7)>>3;

	for(i=0; i<mPathPoints; ++i) {
		mpPathPoints[i].x -= minx*8;
		mpPathPoints[i].y -= miny*8;
	}

	if (minx > maxx || miny > maxy) {
		mWidth = mHeight = 0;
		mPathOffsetX = mPathOffsetY = 0;
		_TrashPath();
		return true;
	}

	mWidth = maxx + 1 - minx;
	mHeight = maxy + 1 - miny;

	mWideBorder = 0;

	mPathOffsetX = minx;
	mPathOffsetY = miny;

	// Initialize edge buffer.  We use edge 0 as a sentinel.

	mEdgeNext = 1;
	mEdgeHeapSize = 2048;
	mpEdgeBuffer = (Edge *)malloc(sizeof(Edge)*mEdgeHeapSize);

	// Initialize scanline list.

	mpScanBuffer = new unsigned int[mHeight];

	// Scan convert the outline.  Yuck, Bezier curves....

	for(i=0; i<mHeight; ++i)
		mpScanBuffer[i] = 0;

	// Kick the FPU into round down mode.

	unsigned short fpucw;

	__asm {
		fstcw fpucw
		or fpucw, 400h
		fldcw fpucw
	}

	// Unfortunately, Windows 95/98 GDI has a bad habit of giving us text
	// paths with all but the first figure left open, so we can't rely
	// on the PT_CLOSEFIGURE flag being used appropriately.

	for(i=0; i<mPathPoints; ++i) {
		BYTE t = mpPathTypes[i] & ~PT_CLOSEFIGURE;

		switch(t) {
		case PT_MOVETO:
			if (lastmoveto >= 0)
				_EvaluateLine(i-1, lastmoveto);

			lastmoveto = i;
			break;
		case PT_LINETO:
			_EvaluateLine(i-1, i);
			break;
		case PT_BEZIERTO:
//			EvaluateLine(i-1, i+2);
			_EvaluateBezier(i-1);
			i += 2;
			break;
		}
	}

	if (lastmoveto >= 0)
		_EvaluateLine(i-1, lastmoveto);

	// Restore the normal rounding mode.

	__asm {
		fstcw fpucw
		and fpucw, 0f3ffh
		fldcw fpucw
	}

	// Free the path since we don't need it anymore.

	_TrashPath();

	// Convert the edges to spans.  We couldn't do this before because some of
	// the regions may have winding numbers >+1 and it would have been a pain
	// to try to adjust the spans on the fly.  We use one heap to detangle
	// a scanline's worth of edges from the singly-linked lists, and another
	// to collect the actual scans.

	std::vector<int> heap;

	mOutline.reserve(mEdgeNext / 2);

	int y = 0;

	for(y=0; y<mHeight; ++y) {
		int count = 0;
		int flipcount = mpScanBuffer[y]>>20;

		// Keep the edge heap from doing lots of stupid little reallocates.

		if (heap.capacity() < flipcount)
			heap.reserve((flipcount + 63)&~63);

		// Detangle scanline into edge heap.

		int i=0;

		heap.clear();

		for(unsigned ptr = mpScanBuffer[y]&0xfffff; ptr; ptr = mpEdgeBuffer[ptr].next) {
			//heap[i++] = mpEdgeBuffer[ptr].posandflag;
			heap.push_back(mpEdgeBuffer[ptr].posandflag);
			i++;
		}

		// Sort edge heap.  Note that we conveniently made the opening edges
		// one more than closing edges at the same spot, so we won't have any
		// problems with abutting spans.

		std::sort(heap.begin(), heap.begin()+i);

		// Process edges and add spans.  Since we only check for a non-zero
		// winding number, it doesn't matter which way the outlines go!

		std::vector<int>::iterator itX1 = heap.begin();
		std::vector<int>::iterator itX2 = heap.begin() + flipcount;

		int x1, x2;

		for(; itX1 != itX2; ++itX1) {
			int x = *itX1;

			if (!count) {
				x1 = (x>>1);
			}

			if (x&1)
				++count;
			else
				--count;

			if (!count) {
				x2 = (x>>1);

				if (x2>x1)
					mOutline.push_back(std::pair<int,int>((y<<20)+x1+0x40040000, (y<<20)+x2+0x40040000));
			}
		}
	}

	// Dump the edge and scan buffers, since we no longer need them.

	free(mpEdgeBuffer);
	free(mpScanBuffer);

	// All done!

	return true;
}

void Rasterizer::_OverlapRegion(tSpanBuffer& dst, tSpanBuffer& src, int dx, int dy) {
	tSpanBuffer temp;

	temp.reserve(dst.size()+src.size());

	dst.swap(temp);

	tSpanBuffer::iterator itA = temp.begin();
	tSpanBuffer::iterator itAE = temp.end();
	tSpanBuffer::iterator itB = src.begin();
	tSpanBuffer::iterator itBE = src.end();

	// Don't worry -- even if dx<0 this will still work!

	unsigned int offset1 = (dy<<20) - dx;
	unsigned int offset2 = (dy<<20) + dx;

	while(itA != itAE && itB != itBE) {
		if ((*itB).first+offset1 < (*itA).first) {

			// B span is earlier.  Use it.

			unsigned x1 = (*itB).first+offset1;
			unsigned x2 = (*itB).second+offset2;
			++itB;

			// B spans don't overlap, so begin merge loop with A first.

			for(;;) {

				// If we run out of A spans or the A span doesn't overlap,
				// then the next B span can't either (because B spans don't
				// overlap) and we exit.

				if (itA != itAE && (*itA).first <= x2) {
					do {
						//x2 = std::_MAX(x2, (*itA++).second);
						x2 = max(x2, (*itA).second);
						itA++;
					} while(itA != itAE && (*itA).first <= x2);
				} else
					break;

				// If we run out of B spans or the B span doesn't overlap,
				// then the next A span can't either (because A spans don't
				// overlap) and we exit.

				if (itB != itBE && (*itB).first+offset1 <= x2) {
					do {
						//x2 = std::_MAX(x2, (*itB++).second + offset2);
						x2 = max(x2, (*itB).second + offset2);
						itB++;
					} while(itB != itBE && (*itB).first+offset1 <= x2);
				} else
					break;
			}

			// Flush span.

			dst.push_back(std::pair<unsigned,unsigned>(x1,x2));	

		} else {
			// A span is earlier.  Use it.

			unsigned x1 = (*itA).first;
			unsigned x2 = (*itA).second;

			++itA;

			// A spans don't overlap, so begin merge loop with B first.

			for(;;) {
				// If we run out of B spans or the B span doesn't overlap,
				// then the next A span can't either (because A spans don't
				// overlap) and we exit.

				if (itB != itBE && (*itB).first+offset1 <= x2) {
					do {
						//x2 = std::_MAX(x2, (*itB++).second + offset2);
						x2 = max(x2, (*itB).second + offset2);
						itB++;
					} while(itB != itBE && (*itB).first+offset1 <= x2);
				} else
					break;

				// If we run out of A spans or the A span doesn't overlap,
				// then the next B span can't either (because B spans don't
				// overlap) and we exit.

				if (itA != itAE && (*itA).first <= x2) {
					do {
						//x2 = std::_MAX(x2, (*itA++).second);
						x2 = max(x2, (*itA).second);
						itA++;
					} while(itA != itAE && (*itA).first <= x2);
				} else
					break;
			}

			// Flush span.

			dst.push_back(std::pair<unsigned,unsigned>(x1,x2));	
		}
	}

	// Copy over leftover spans.

	while(itA != itAE)
		dst.push_back(*itA++);

	while(itB != itBE) {
		dst.push_back(std::pair<unsigned,unsigned>((*itB).first+offset1,(*itB).second+offset2));	
		++itB;
	}
}

bool Rasterizer::CreateWidenedRegion(int r) {
	std::vector<std::pair<unsigned, unsigned> > heap3;

	for(int y = -r; y <= r; ++y) {
		int x = (int)(0.5 + sqrt((double)(r*r - y*y)));

		_OverlapRegion(mWideOutline, mOutline, x, y);
	}

	mWideBorder = r;

	return true;
}

void Rasterizer::DeleteOutlines() {
	mWideOutline.clear();
	mOutline.clear();
}

bool Rasterizer::Rasterize(int xsub, int ysub, bool bBorder) {
	_TrashOverlay();

	if (!mWidth || !mHeight) {
		mOverlayWidth = mOverlayHeight = 0;
		return true;
	}

	xsub &= 7;
	ysub &= 7;

	int width = mWidth + xsub;
	int height = mHeight + ysub;
	int border = 0;

	mOffsetX = mPathOffsetX - xsub;
	mOffsetY = mPathOffsetY - ysub;

	if (bBorder) {
		width += 2*mWideBorder;
		height += 2*mWideBorder;
		border = mWideBorder;

		mOffsetX -= mWideBorder;
		mOffsetY -= mWideBorder;
	}

	mOverlayWidth = (width+7)>>3;
	mOverlayHeight = (height+7)>>3;

	mpOverlayBuffer = new byte[2 * mOverlayWidth * mOverlayHeight];

	memset(mpOverlayBuffer, 0, 2 * mOverlayWidth * mOverlayHeight);

	// Are we doing a border?

	xsub += border;
	ysub += border;

	if (bBorder) {
		tSpanBuffer::iterator it = mWideOutline.begin();
		tSpanBuffer::iterator itEnd = mWideOutline.end();

		for(; it!=itEnd; ++it) {
			int y = ((*it).first >> 20)-0x400+ysub;
			int x1 = ((*it).first & 0xfffff)-0x40000+xsub;
			int x2 = ((*it).second & 0xfffff)-0x40000+xsub;

			if (x2 > x1) {
				int first = x1>>3;
				int last = (x2-1)>>3;
				byte *dst = mpOverlayBuffer + 2*(mOverlayWidth*(y>>3) + first) + 1;

				if (first == last)
					*dst += x2-x1;
				else {
					*dst += ((first+1)<<3) - x1;
					dst += 2;

					while(++first < last) {
						*dst += 0x08;
						dst += 2;
					}

					*dst += x2 - (last<<3);
				}
			}
		}

	}

	tSpanBuffer::iterator it = mOutline.begin();
	tSpanBuffer::iterator itEnd = mOutline.end();

	for(; it!=itEnd; ++it) {
		int y = ((*it).first >> 20)-0x400 + ysub;
		int x1 = ((*it).first & 0xfffff)-0x40000 + xsub;
		int x2 = ((*it).second & 0xfffff)-0x40000 + xsub;

		if (x2 > x1) {
			int first = x1>>3;
			int last = (x2-1)>>3;
			byte *dst = mpOverlayBuffer + 2*(mOverlayWidth*(y>>3) + first);

			if (first == last)
				*dst += x2-x1;
			else {
				*dst += ((first+1)<<3) - x1;
				dst += 2;

				while(++first < last) {
					*dst += 0x08;
					dst += 2;
				}

				*dst += x2 - (last<<3);
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////

extern bool g_bUseMMX;

static const __int64 sixtyfour = 0x0040004000400040i64;
static const __int64 onetwentyeight = 0x0080008000800080i64;

static const __int64 gammatable[65]={
        0x0000000000000000,
        0x0003000300030003,
        0x0005000500050005,
        0x0007000700070007,
        0x0009000900090009,
        0x000a000a000a000a,
        0x000c000c000c000c,
        0x000d000d000d000d,
        0x000e000e000e000e,
        0x0010001000100010,
        0x0011001100110011,
        0x0012001200120012,
        0x0013001300130013,
        0x0014001400140014,
        0x0016001600160016,
        0x0017001700170017,
        0x0018001800180018,
        0x0019001900190019,
        0x001a001a001a001a,
        0x001b001b001b001b,
        0x001c001c001c001c,
        0x001d001d001d001d,
        0x001e001e001e001e,
        0x001f001f001f001f,
        0x0020002000200020,
        0x0021002100210021,
        0x0022002200220022,
        0x0023002300230023,
        0x0023002300230023,
        0x0024002400240024,
        0x0025002500250025,
        0x0026002600260026,
        0x0027002700270027,
        0x0028002800280028,
        0x0029002900290029,
        0x002a002a002a002a,
        0x002a002a002a002a,
        0x002b002b002b002b,
        0x002c002c002c002c,
        0x002d002d002d002d,
        0x002e002e002e002e,
        0x002f002f002f002f,
        0x002f002f002f002f,
        0x0030003000300030,
        0x0031003100310031,
        0x0032003200320032,
        0x0033003300330033,
        0x0033003300330033,
        0x0034003400340034,
        0x0035003500350035,
        0x0036003600360036,
        0x0036003600360036,
        0x0037003700370037,
        0x0038003800380038,
        0x0039003900390039,
        0x0039003900390039,
        0x003a003a003a003a,
        0x003b003b003b003b,
        0x003c003c003c003c,
        0x003c003c003c003c,
        0x003d003d003d003d,
        0x003e003e003e003e,
        0x003f003f003f003f,
        0x003f003f003f003f,
        0x0040004000400040,
};

static void __declspec(naked) ApplyWithBorder(unsigned long *dst, const unsigned char *src, int w, int xo, unsigned long seccolor, const long *switchpts) {
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edx,[esp+12+16]			;edx = w
		mov		ecx,[esp+16+16]			;ecx = xo
		add		edx,ecx					;edx = w+xo
		mov		ebx,[esp+24+16]			;ebx = switchpoints
		mov		esi,[esp+8+16]
		mov		edi,[esp+4+16]

		movd	mm5,[esp+20+16]
		movq	mm6,sixtyfour
		pxor	mm7,mm7
		punpcklbw	mm5,mm7
xloop:
		cmp		ecx,[ebx+4]
		jb		noswitch
switchloop:
		add		ebx,8
		cmp		ecx,[ebx+4]
		jae		switchloop
		movd	mm4,[ebx-8]				;fetch new color
		punpcklbw mm4,mm7
noswitch:
		movzx		eax,byte ptr [esi]
		movzx		ebp,byte ptr [esi+1]
		add			esi,2
		movd		mm3,[edi]
		movq		mm0,[gammatable + eax*8]
		punpcklbw	mm3,mm7
		movq		mm1,[gammatable + ebp*8]
		movq		mm2,mm6
		psubw		mm2,mm1
		psubw		mm1,mm0
		pmullw		mm3,mm2				;mm3 = dst * (1-ba)
		pmullw		mm1,mm5				;mm1 = back * ba
		pmullw		mm0,mm4				;mm0 = fore * fa
		paddw		mm3,mm1
		paddw		mm3,mm0
		psrlw		mm3,6
		packuswb	mm3,mm3
		inc		ecx
		movd		[edi],mm3
		cmp		ecx,edx
		lea			edi,[edi+4]
		jne		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	}
}

static void __declspec(naked) ApplyShadow(unsigned long *dst, const unsigned char *src, int w, unsigned long seccolor) {
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ecx,[esp+12+16]			;ecx = w
		mov		esi,[esp+8+16]
		mov		edi,[esp+4+16]

		movd	mm5,[esp+16+16]
		movq	mm6,onetwentyeight
		pxor	mm7,mm7
		punpcklbw	mm5,mm7
xloop:
		movzx		eax,byte ptr [esi+1]
		add			esi,2
		movd		mm3,[edi]
		movq		mm0,[gammatable+eax*8]
		punpcklbw	mm3,mm7
		movq		mm2,mm6
		psubw		mm2,mm0
		pmullw		mm3,mm2				;mm3 = dst * (1-ba)
		pmullw		mm0,mm5				;mm1 = back * ba
		paddw		mm3,mm0
		psrlw		mm3,7
		packuswb	mm3,mm3
		dec			ecx
		movd		[edi],mm3
		lea			edi,[edi+4]
		jne		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	}
}

///////////////////////////////////////////////////////////////////////////

void Rasterizer::Draw(unsigned long *dst, int xsub, int ysub, int bw, int bh, int pitch, unsigned long seccolor, bool bBorder, const long *switchpts) {

	// Clip.

	int x = (xsub + mOffsetX + 4)>>3;
	int y = (ysub + mOffsetY + 4)>>3;
	int w = mOverlayWidth;
	int h = mOverlayHeight;
	int xo=0, yo=0;

	if (x < 0) {
		xo = -x;
		w += x;
		x=0;
	}

	if (y < 0) {
		yo = -y;
		h += y;
		y=0;
	}

	if (x+w > bw)
		w = bw-x;

	if (y+h > bh)
		h = bh-y;

	if (w<=0 || h<=0)
		return;

	// Draw.

	const byte *src = mpOverlayBuffer + 2*(mOverlayWidth * yo + xo);

	dst = (unsigned long *)((char *)dst + pitch * (bh-1-y)) + x;

	do {
		unsigned long color;
		const long *sw = switchpts;

		if (bBorder) {
			if (g_bUseMMX)
				ApplyWithBorder(dst, src, w, xo, seccolor, switchpts);
			else
				for(int wt=0; wt<w; ++wt) {
					int a = src[wt*2];
					int a2 = src[wt*2+1] - a;
					int ia = 64-src[wt*2+1];

					if (wt+xo >= sw[1]) {
						while(wt+xo >= sw[1])
							sw += 2;

						color = sw[-2];
					}

					dst[wt]	= ((((dst[wt]&0x00ff00ff)*ia + (color&0x00ff00ff)*a + (seccolor&0x00ff00ff)*a2)&0x3fc03fc0)>>6)
							+ ((((dst[wt]&0x0000ff00)*ia + (color&0x0000ff00)*a + (seccolor&0x0000ff00)*a2)&0x003fc000)>>6);
				}
		} else {
			for(int wt=0; wt<w; ++wt) {
				int a = src[wt*2];
				int ia = 64-a;

				if (wt+xo >= sw[1]) {
					while(wt+xo >= sw[1])
						sw += 2;

					color = sw[-2];
				}

				dst[wt]	= ((((dst[wt]&0x00ff00ff)*ia + (color&0x00ff00ff)*a)&0x3fc03fc0)>>6)
						+ ((((dst[wt]&0x0000ff00)*ia + (color&0x0000ff00)*a)&0x003fc000)>>6);
			}
		}

		src += 2*mOverlayWidth;
		dst = (unsigned long *)((char *)dst - pitch);
	} while(--h);

	if (g_bUseMMX)
		__asm emms
}

void Rasterizer::DrawShadow(unsigned long *dst, int xsub, int ysub, int bw, int bh, int pitch, unsigned long seccolor) {
	// Clip.

	int x = (xsub + mOffsetX + 4)>>3;
	int y = (ysub + mOffsetY + 4)>>3;
	int w = mOverlayWidth;
	int h = mOverlayHeight;
	int xo=0, yo=0;

	if (x < 0) {
		xo = -x;
		w += x;
		x=0;
	}

	if (y < 0) {
		yo = -y;
		h += y;
		y=0;
	}

	if (x+w > bw)
		w = bw-x;

	if (y+h > bh)
		h = bh-y;

	if (w<=0 || h<=0)
		return;

	// Draw.

	const byte *src = mpOverlayBuffer + 2*(mOverlayWidth * yo + xo);

	dst = (unsigned long *)((char *)dst + pitch * (bh-y-1)) + x;

	do {
		if (g_bUseMMX)
			ApplyShadow(dst, src, w, seccolor);
		else
			for(int wt=0; wt<w; ++wt) {
				int a = src[wt*2+1];
				int ia = 128-a;

				dst[wt]	= ((((dst[wt]&0x00ff00ff)*ia + (seccolor&0x00ff00ff)*a)&0x7f807f80)>>7)
						+ ((((dst[wt]&0x0000ff00)*ia + (seccolor&0x0000ff00)*a)&0x007f8000)>>7);
			}

		src += 2*mOverlayWidth;
		dst = (unsigned long *)((char *)dst - pitch);
	} while(--h);

	if (g_bUseMMX)
		__asm emms
}
